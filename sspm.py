from clint.textui import progress
from functools import partial
from multiprocessing.pool import ThreadPool
from threading import Lock
from time import time as timer
import json
import click
import core
import plugins as extPlugins
from types import SimpleNamespace

@click.group()
def cli():
    pass

def getLocalConfig(dest_dir, with_cache, localPrint):
    with open("sspm.config.json", "r") as configFile:
        config = json.loads(configFile.read())

    if localPrint is None:
        localPrint = print

    config["localPrint"] = localPrint
    config["destDir"] = dest_dir
    config["withCache"] = with_cache

    packageFile = core.getDestinationPackageFile(config)
    config["packageFile"] = packageFile

    return config

def installPackageInternal(packageName, config, plugins, localPrint):
    packages = {}
    with_cache = config["withCache"]
    dest_dir = config["destDir"]

    localPrint(f"downloading package info for {packageName}")
    packageName = packageName.split("@")
    packageVersion = None

    if len(packageName) == 2:
        packageVersion = packageName[-1]
        if packageVersion == "latest":
            packageVersion = None

    packageName = packageName[0]

    (rootPackageInfo, rootVersionInfo) = core.downloadPackageInformation(config, packageName, packageVersion, packages, not with_cache)

    pool = ThreadPool(config["numberOfConcurrentDownloads"])
    results = pool.imap_unordered(partial(core.downloadPackageTarForThread, config), packages.values())

    start = timer()
    localPrint(f"downloading {len(packages)} files for {packageName}")
    if localPrint is print:
        with progress.Bar(label=packageName, expected_size=len(packages)) as bar:
            for index, (packageInfo, versionInfo) in enumerate(results):
                bar.label = f"{packageInfo['name']}@{versionInfo['version']}\t"
                bar.show(index + 1)
                core.extractTar(config, packageInfo, versionInfo)
    else:
        for index, (packageInfo, versionInfo) in enumerate(results):
            localPrint(f"downloaded {packageInfo['name']}@{versionInfo['version']}\t")
            core.extractTar(config, packageInfo, versionInfo)

    localPrint(f"Elapsed Time: {timer() - start}")
    localPrint(f"copying files to {dest_dir}")

    for (packageInfo, versionInfo) in reversed(packages.values()):
        core.copyFilesToFinalFolder(config, dest_dir, packageInfo, versionInfo)

    for (packageInfo, versionInfo) in reversed(packages.values()):
        if "scripts" in versionInfo and "postinstall" in versionInfo["scripts"]:
            postInstallScript = versionInfo["scripts"]["postinstall"]
            for plugin in plugins.postInstall:
                if plugin.canExecute(postInstallScript):
                    postInstallScriptPath = [filePath for filePath in versionInfo["extractedFiles"]
                                             if filePath.endswith(postInstallScript)][0]

                    plugin.execute(postInstallScriptPath, dest_dir)


    finalPackageVersion = f"^{rootVersionInfo['version']}" if packageVersion is None  else rootVersionInfo["version"]

    if rootPackageInfo["name"] not in config["packageFile"]["dependencies"] or\
            config["packageFile"]["dependencies"][rootPackageInfo["name"]] != finalPackageVersion:
        config["packageFile"]["dependencies"][rootPackageInfo["name"]] = finalPackageVersion
        core.updateDestinationPackageFile(config, config["packageFile"])

def installPackageCore(packageName, dest_dir, with_cache, localPrint):
    config = getLocalConfig(dest_dir, with_cache, localPrint)
    core.createTempDirectory(config)
    plugins = SimpleNamespace()
    plugins.postInstall = extPlugins.loadPostInstallPlugins(config)
    return installPackageInternal(packageName, config, plugins, localPrint)

def installPackagesCore(installpackages, dest_dir, with_cache, localPrint=None):
    for packageName in installpackages:
        installPackageCore(packageName, dest_dir, with_cache, localPrint)


@cli.command("install")
@click.argument("installPackages", nargs=-1)
@click.option("--dest-dir", default=".", help="The directory where the game will be placed")
@click.option("--with-cache", default=True, type=click.BOOL, help="Whether or not package info should be cached in a local database.")
def install(installpackages, dest_dir, with_cache):
    installPackagesCore(installpackages, dest_dir, with_cache)

if __name__ == "__main__":
    cli()
