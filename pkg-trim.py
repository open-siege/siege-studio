import os
import glob
import json
from collections import OrderedDict
import hashlib
import click

def getAllFiles(path):
    globPath = os.path.join(path, "**", "*")
    replacementPath = os.path.join(path, "")
    files = glob.glob(globPath, recursive=True)

    results = {}
    for file in files:
        key = file.replace(replacementPath, "")

        if ".DELETE." in key:
            key = key.split(".DELETE.")[0]

        results[key] = file

        if key != key.lower():
            results[key.lower()] = file

    return results

def getFileHash(path):
    with open(path, "rb") as tempFile:
        return hashlib.sha512(tempFile.read()).hexdigest()

@click.group()
def cli():
    pass


@cli.command("bulk")
@click.argument("packages", nargs=2)
@click.option("--base-dir", default="packages", help="The base directory where all the packages should be present.")
def bulk(packages, base_dir):
    metaPackages = {}
    for package in packages:
        metaPackages[package] = {}
        metaPackageDir = os.path.join(base_dir, package)
        packageGlob = os.path.join(metaPackageDir, "**", "package.json")
        packageJsonFiles = glob.glob(packageGlob)
        for file in packageJsonFiles:
            packageFolder = os.path.split(file)[:-1]
            packageFolder = os.path.join(*packageFolder)
            with open(file, "r") as packageInfo:
                parsedInfo = json.loads(packageInfo.read())
                parsedInfo["folder"] = packageFolder
                parsedInfo["file"] = file
                metaPackages[package][parsedInfo["name"]] = parsedInfo

    firstVersion = packages[0]
    secondVersion = packages[1]

    firstMeta = metaPackages[firstVersion]
    secondMeta = metaPackages[secondVersion]

    for packageName, packageInfo in firstMeta.items():
        if packageName in secondMeta:
            secondInfo = secondMeta[packageName]
            firstFiles = getAllFiles(packageInfo["folder"])
            secondFiles = getAllFiles(secondInfo["folder"])
            processedFiles = set()
            dependentVersions = OrderedDict()

            for file, path in firstFiles.items():
                if file.endswith("package.json"):
                    continue
                if os.path.isdir(path):
                    continue

                if file.lower() in processedFiles:
                    continue

                packageVersion = packageInfo["version"]
                if ".DELETE." in path:
                    packageVersion = path.split('.DELETE.')[-1]

                dependentVersions[packageVersion] = None

                if file in secondFiles or file.lower() in secondFiles:
                    firstHash = getFileHash(path)

                    if file in secondFiles:
                        secondFilePath = secondFiles[file]
                    else:
                        secondFilePath = secondFiles[file.lower()]

                    if os.path.isfile(f"{secondFilePath}.DELETE.{packageVersion}"):
                        continue

                    if ".DELETE." in secondFilePath:
                        continue

                    secondHash = getFileHash(secondFilePath)
                    if firstHash == secondHash:
                        latestVersion = [*dependentVersions.keys()][-1]
                        if packageName not in secondInfo["dependencies"] or \
                                secondInfo["dependencies"] != latestVersion:
                            secondInfo["dependencies"][packageName] = latestVersion
                            packageFile = secondInfo["file"]
                            packageFolder = secondInfo["folder"]
                            del secondInfo["file"]
                            del secondInfo["folder"]
                            with open(packageFile, "w") as fileHandle:
                                fileHandle.write(json.dumps(secondInfo, indent="\t"))
                            secondInfo["file"] = packageFile
                            secondInfo["folder"] = packageFolder
                        newPath = f"{secondFilePath}.DELETE.{packageVersion}"
                        os.rename(secondFilePath, newPath)
                        processedFiles.add(file.lower())
                    else:
                        print(f"{file} does not match second version")


if __name__ == "__main__":
    cli()