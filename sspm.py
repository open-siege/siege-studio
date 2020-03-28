from clint.textui import progress
from functools import partial
from multiprocessing.pool import ThreadPool
from threading import Lock
from time import time as timer
import json
import click
import core
import plugins
import os

s_print_lock = Lock()

def s_print(*a, **b):
    """Thread safe print function"""
    with s_print_lock:
        print(*a, **b)

@click.group()
def cli():
    pass


@cli.command("install")
@click.argument("installPackages", nargs=-1)
@click.option("--dest-dir", default=".", help="The directory where the game will be placed")
def install(installpackages, dest_dir):
    with open("sspm.config.json", "r") as configFile:
        config = json.loads(configFile.read())

    core.createTempDirectory(config)
    postInstallPlugins = plugins.loadPostInstallPlugins(config)

    for packageName in installpackages:
        packages = {}
        s_print(f"downloading package info for {packageName}")
        packageName = packageName.split("@")
        packageVersion = None

        if len(packageName) == 2:
            packageVersion = packageName[-1]

        packageName = packageName[0]

        core.downloadPackageInformation(config, packageName, packageVersion, packages)

        pool = ThreadPool(config["numberOfConcurrentDownloads"])
        results = pool.imap_unordered(partial(core.downloadPackageTarForThread, config), packages.values())

        start = timer()
        s_print(f"downloading {len(packages)} files for {packageName}")
        with progress.Bar(label=packageName, expected_size=len(packages)) as bar:
            for index, (packageInfo, versionInfo) in enumerate(results):
                bar.label = f"{packageInfo['name']}@{versionInfo['version']}\t"
                bar.show(index + 1)
                core.extractTar(config, packageInfo, versionInfo)


        s_print(f"Elapsed Time: {timer() - start}")
        s_print(f"copying files to {dest_dir}")

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
                print(f"{packageInfo['name']} has {postInstallScript}")

if __name__ == "__main__":
    cli()
