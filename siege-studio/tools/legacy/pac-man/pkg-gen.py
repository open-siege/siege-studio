import json
import os
import shutil
import glob
import click


def getChildren(package: dict, allPackages: list):
    if package not in allPackages:
        allPackages.append(package)
    if "children" in package:
        for child in package["children"]:
            child["parent"] = package
            allPackages.append(child)
            getChildren(child, allPackages)


@click.group()
def cli():
    pass


@cli.command("new")
@click.argument("specname")
@click.option("--dest-dir", default="packages", help="The directory where the packages will be placed")
@click.option("--base-dir", default=".", help="The directory where the packages will be placed")
@click.option("--version", help="The version that the packages should be.")
def new(specname, dest_dir, base_dir, version):

    allPackages = []

    with open("package-prototype.json", "r") as prototypeFile:
        prototype = json.loads(prototypeFile.read())

    with open(specname, "r") as specFile:
        mainPackage = json.loads(specFile.read())

    getChildren(mainPackage, allPackages)

    for package in allPackages:
        destDir = os.path.join(dest_dir, package["name"])
        filesToInclude = []
        filesToExclude = []
        if "include" in package:
            for fileGlob in package["include"]:
                fileGlob = os.path.join(base_dir, fileGlob)
                filesToInclude.extend(glob.glob(fileGlob))
        if "exclude" in package:
            for fileGlob in package["exclude"]:
                fileGlob = os.path.join(base_dir, fileGlob)
                filesToExclude.extend(glob.glob(fileGlob))

        # if there are meant to be files but none were found,
        # then we don't have a package to make
        if len(filesToInclude) == 0 and ("files" in package and len(package["files"] > 0)):
            continue

        if not os.path.exists(destDir):
            os.makedirs(destDir)

        packageFile = os.path.join(destDir, "package.json")

        with open(packageFile, "w") as newPackageFile:
            prototype["name"] = package["name"]
            prototype["version"] = version
            prototype["dependencies"] = {}
            prototype["optionalDependencies"] = {}

            if "children" in package:
                for child in package["children"]:
                    prototype["dependencies"][child["name"]] = version
            newPackageFile.write(json.dumps(prototype, indent="\t"))

        for file in filesToInclude:
            newDir = file.replace(os.path.join(base_dir, ""), "")
            newDir = os.path.split(newDir)[:-1]
            newDir = os.path.join(destDir, *newDir)
            if not os.path.exists(newDir):
                os.makedirs(newDir)

            if file not in filesToExclude:
                print(f"Copying {file}")
                shutil.copy(file, newDir)

if __name__ == "__main__":
    cli()