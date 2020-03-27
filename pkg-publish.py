import os
import json
import glob
import click

@click.group()
def cli():
    pass


@cli.command("bulk")
@click.option("--base-dir", default="packages", help="The base directory where all the packages should be present.")
def bulk(base_dir):
    with open("pkg-publish.config.json", "r") as configFile:
        config = json.loads(configFile.read())

    registryUrl = None

    if "registryUrl" in config:
        registryUrl = config["registryUrl"]

    packageFiles = glob.glob(os.path.join(base_dir, "**", "package.json"), recursive=True)
    for packageFile in packageFiles:
        packageFolder = packageFile.replace("package.json", "")
        filesToDelete = glob.glob(os.path.join(packageFolder, "**", "*.DELETE.*"), recursive=True)
        for file in filesToDelete:
            os.remove(file)

        npmCommand = f"{config['npmPath']} publish --cwd {packageFolder}"

        if registryUrl is not None:
            npmCommand = f"{npmCommand} --registry {registryUrl}"

        os.system(npmCommand)


if __name__ == "__main__":
    cli()