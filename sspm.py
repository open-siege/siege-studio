import json
import tarfile
import requests
import os
import hashlib
import os.path
import click
import base64
from tinydb import TinyDB, where
from clint.textui import progress
from os import walk
from multiprocessing.pool import ThreadPool
from time import time as timer
from threading import Lock

s_print_lock = Lock()

def s_print(*a, **b):
    """Thread safe print function"""
    with s_print_lock:
        print(*a, **b)


with open("sspm.config.json", "r") as configFile:
    config = json.loads(configFile.read())

tempDirectory = os.path.join("temp", "packages")

if not os.path.exists(tempDirectory):
    os.makedirs(tempDirectory)

def getAllFilesFromFolder(folder, files = None):
    if files is None:
        files = []

    for (dirpath, dirnames, filenames) in walk(folder):
        filenames = map(lambda f: os.path.join(dirpath, f), filenames)
        files.extend(filenames)
        for dir in dirnames:
            getAllFilesFromFolder(os.path.join(dirpath, dir), files)

    return files

def getPackageInfo(packageName, withoutCache):
    db = TinyDB(config["cacheDbLocation"])
    packages = db.table("packages")
    results = []
    if withoutCache is False:
        results = packages.search(where("name") == packageName)

    if len(results) == 0:
        packageInfoResponse = requests.get(f"{config['packageRegistry']}/{packageName}")
        result = packageInfoResponse.json()

        if "dist-tags" in result and withoutCache is False:
            packages.insert(result)
        elif withoutCache is True:
            packages.remove(where("name") == packageName)
        return result
    else:
        return results[0]

def downloadTarball(versionInfo):
    packageInfoResponse = requests.get(versionInfo['dist']['tarball'], stream=True)
    return packageInfoResponse

def getTarballName(versionInfo):
    return os.path.split(versionInfo['dist']['tarball'])[-1]

def findMatchingVersion(packageInfo, version):
    version = version.split("^")[-1]
    for someVersion in packageInfo["versions"]:
        if someVersion == version:
            return someVersion
    return None

def copyFilesToFinalFolder(finalDirectory, packageInfo):
    for file in packageInfo["extractedFiles"]:

        fileFolder = file.replace(os.path.join(tempDirectory, packageInfo["name"], "package", ""), "")
        splitData = os.path.split(fileFolder)
        fileFolder = os.path.join(finalDirectory, *splitData[:-1])
        filename = splitData[-1]
        if not os.path.exists(fileFolder):
            os.makedirs(fileFolder)
        if filename != "package.json":
            if not os.path.exists(os.path.join(fileFolder, filename)):
                os.rename(file, os.path.join(fileFolder, filename))
    for child in packageInfo["expandedDependencies"]:
        copyFilesToFinalFolder(finalDirectory, child)


def downloadPackageInformation(packageName, version, packages, withoutCache=False):
    packageInfo = getPackageInfo(packageName, withoutCache)

    if version is None:
        versionToUse = packageInfo["dist-tags"]["latest"]
    else:
        versionToUse = findMatchingVersion(packageInfo, version)

    if versionToUse is None:
        if withoutCache is False:
            return downloadPackageInformation(packageName, version, packages, True)
        raise RuntimeError(f"Could not find matching version for {packageName} {version}")

    versionInfo = packageInfo["versions"][versionToUse]

    packages.append((packageInfo, versionInfo))
    processDependeciesInformation(packageInfo, versionInfo, packages)

    return packageInfo

def processDependeciesInformation(packageInfo, versionInfo, packages):
    packageInfo["expandedDependencies"] = []
    if "dependencies" in versionInfo:
        for key, value in versionInfo["dependencies"].items():
            childPackage = downloadPackageInformation(key, value, packages)
            packageInfo["expandedDependencies"].append(childPackage)

def extractTar(packageInfo, versionInfo):
    destinationDirectory = os.path.join(tempDirectory, packageInfo["name"])
    if not os.path.exists(destinationDirectory):
        os.makedirs(destinationDirectory)
    with tarfile.open(os.path.join(tempDirectory, getTarballName(versionInfo))) as tf:
        tf.extractall(path=destinationDirectory)

    packageInfo["extractedFiles"] = getAllFilesFromFolder(destinationDirectory)


def downloadPackageTar(versionInfo):
    downloadPath = os.path.join(tempDirectory, getTarballName(versionInfo))

    if os.path.exists(downloadPath):
        with open(os.path.join(downloadPath), "rb") as tempFile:
            rawData = tempFile.read()
        fileHash = f"sha512-{base64.encodebytes(hashlib.sha512(rawData).digest()).decode('utf8')}"
        fileHash = "".join(fileHash.split("\n"))
        if fileHash != versionInfo["dist"]["integrity"]:
            os.remove(downloadPath)

    if not os.path.exists(downloadPath):
        with open(os.path.join(downloadPath), "wb") as tempFile:
            with downloadTarball(versionInfo) as tarballResponse:
                tarLength = int(tarballResponse.headers.get("content-length"))

                if tarLength is None:
                    tempFile.write(tarballResponse.content)
                else:
                    for chunk in tarballResponse.iter_content(chunk_size=4096):
                        if chunk:
                            tempFile.write(chunk)
            tempFile.flush()


def downloadPackageTarForThread(rawData):
    (packageInfo, versionInfo) = rawData
    downloadPackageTar(versionInfo)
    return rawData

@click.group()
def cli():
    pass


@cli.command("install")
@click.argument("packagename")
@click.option("--dest-dir", default=".", help="The directory where the game will be placed")
def install(packagename, dest_dir):
    packages = []
    s_print(f"downloading package info for {packagename}")
    packagename = packagename.split("@")
    packageVersion = None

    if len(packagename) == 2:
        packageVersion = packagename[-1]

    packagename = packagename[0]

    finalPackage = downloadPackageInformation(packagename, packageVersion, packages)

    pool = ThreadPool(config["numberOfConcurrentDownloads"])
    results = pool.imap_unordered(downloadPackageTarForThread, packages)

    start = timer()
    s_print(f"downloading {len(packages)} files for {packagename}")
    with progress.Bar(label="nonlinear", expected_size=len(packages)) as bar:
        bar.label = f"{packages[0][0]['name']}\t"
        for index, (packageInfo, versionInfo) in enumerate(results):
            if index + 1 < len(packages):
                bar.label = f"{packages[index + 1][0]['name']}\t"
            bar.show(index + 1)
            extractTar(packageInfo, versionInfo)


    s_print(f"Elapsed Time: {timer() - start}")
    s_print(f"copying files to {dest_dir}")
    copyFilesToFinalFolder(dest_dir, finalPackage)


if __name__ == "__main__":
    cli()
