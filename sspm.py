import json
import tarfile
import requests
from clint.textui import progress
import os.path

packageName = "starsiege-sounds"
with open("sspm.config.json", "r") as configFile:
    config = json.loads(configFile.read())

tempDirectory = "temp/packages"

if not os.path.exists(tempDirectory):
    os.makedirs(tempDirectory)

def getPackageInfo(packageName):
    packageInfoResponse = requests.get(f"{config['packageRegistry']}/{packageName}")
    return packageInfoResponse.json()

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

def downloadPackageWithDependencies(packageName, version = None):
    destinationDirectory = os.path.join(tempDirectory, packageName)
    if not os.path.exists(destinationDirectory):
        os.makedirs(destinationDirectory)
    packageInfo = getPackageInfo(packageName)

    if version is None:
        versionToUse = packageInfo["dist-tags"]["latest"]
    else:
        versionToUse = findMatchingVersion(packageInfo, version)

    if versionToUse is None:
        raise RuntimeError(f"Could not find matching version for {packageName} {version}")

    print(f"downloading {packageName} {versionToUse}")
    versionInfo = packageInfo["versions"][versionToUse]

    with open(getTarballName(versionInfo), "wb") as tempFile:
        tarballResponse = downloadTarball(versionInfo)
        tarLength = int(tarballResponse.headers.get("content-length"))

        if tarLength is None:
            tempFile.write(tarballResponse.content)
        else:
            for chunk in progress.bar(tarballResponse.iter_content(chunk_size=1024), expected_size=(tarLength / 1024) + 1):
                if chunk:
                    tempFile.write(chunk)
                    tempFile.flush()

    with tarfile.open(getTarballName(versionInfo)) as tf:
        tf.extractall(path=destinationDirectory)

    if "dependencies" in versionInfo:
        for key, value in versionInfo["dependencies"].items():
            downloadPackageWithDependencies(key, value)


downloadPackageWithDependencies(packageName)

