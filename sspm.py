import json
import tarfile
import requests
import os.path
from io import BytesIO


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
    packageInfoResponse = requests.get(versionInfo['dist']['tarball'])
    return packageInfoResponse

def getTarballName(versionInfo):
    return os.path.split(versionInfo['dist']['tarball'])[-1]

def downloadPackageWithDependencies(packageName, version = None):
    print(f"downloading {packageName}")
    destinationDirectory = os.path.join(tempDirectory, packageName)
    if not os.path.exists(destinationDirectory):
        os.makedirs(destinationDirectory)
    packageInfo = getPackageInfo(packageName)
    if version is None:
        version = packageInfo["dist-tags"]["latest"]

    versionInfo = packageInfo["versions"][version]

    tarballResponse = downloadTarball(versionInfo)

    with open(getTarballName(versionInfo), "wb") as tempFile:
        tempFile.write(tarballResponse.content)

    with tarfile.open(getTarballName(versionInfo)) as tf:
        tf.extractall(path=destinationDirectory)

    for key in versionInfo["dependencies"]:
        downloadPackageWithDependencies(key)


downloadPackageWithDependencies(packageName)

