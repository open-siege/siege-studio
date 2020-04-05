from clint.textui import progress
from functools import partial
from multiprocessing.pool import ThreadPool
from threading import Lock
from time import time as timer
import json
import click
import core
import plugins

@click.group()
def cli():
    pass


def installCore(installpackages, dest_dir, with_cache, localPrint=None):
    with open("sspm.config.json", "r") as configFile:
        config = json.loads(configFile.read())

    if localPrint is None:
        localPrint = print

    config["localPrint"] = localPrint
    core.createTempDirectory(config)
    postInstallPlugins = plugins.loadPostInstallPlugins(config)

    for packageName in installpackages:
        packages = {}
        localPrint(f"downloading package info for {packageName}")
        packageName = packageName.split("@")
        packageVersion = None

        if len(packageName) == 2:
            packageVersion = packageName[-1]
            if packageVersion == "latest":
                packageVersion = None

        packageName = packageName[0]

        core.downloadPackageInformation(config, packageName, packageVersion, packages, not with_cache)

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
                for plugin in postInstallPlugins:
                    if plugin.canExecute(postInstallScript):
                        postInstallScriptPath = [filePath for filePath in versionInfo["extractedFiles"]
                                                 if filePath.endswith(postInstallScript)][0]

                        plugin.execute(postInstallScriptPath, dest_dir)

@cli.command("install")
@click.argument("installPackages", nargs=-1)
@click.option("--dest-dir", default=".", help="The directory where the game will be placed")
@click.option("--with-cache", default=True, type=click.BOOL, help="Whether or not package info should be cached in a local database.")
def install(installpackages, dest_dir, with_cache):
    installCore(installpackages, dest_dir, with_cache)

if __name__ == "__main__":
    cli()
