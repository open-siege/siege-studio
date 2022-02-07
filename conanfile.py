from conans import ConanFile, CMake, tools
from shutil import copyfile
import os.path


class LocalConanFile(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    build_requires = "cmake/3.22.0"

    def requirements(self):
        self.run("conan install cmake/3.22.0@/ -g virtualenv")
        activate = "activate.bat" if self.settings.os == "Windows" else "./activate.sh"
        install = "install.bat" if self.settings.os == "Windows" else "./install.sh"
        copyfile(activate, install)
        with open(install, "a") as file:
            targets = ["dts-to-json", "json-to-dts", "dts-to-obj", "unvol", "siege-shell", "3space-studio"]
            commands = [
                "cd 3space",
                f"conan install . -s build_type={self.settings.build_type} --build=missing",
                "conan export .",
                "cd .."
            ]

            for target in targets:
                commands.append(f"cd {target}")
                commands.append(f"conan install . -s build_type={self.settings.build_type} --build=missing")
                commands.append(f"cd ..")

            file.write("\n" + " && ".join(commands))

        if self.settings.os != "Windows":
            self.run(f"chmod +x {install}")
        self.run(install)


    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()

    def package(self):
        git = tools.Git(folder="wiki")
        git.clone("https://github.com/StarsiegePlayers/3space-studio.wiki.git", shallow=True)
        tools.rmdir("wiki/.git")
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        self.copy(pattern="LICENSE", src=self.source_folder, dst="licenses")
        self.copy(pattern="README.md", src=self.source_folder, dst="res")
        self.copy(pattern="wiki/*.md", src=self.source_folder, dst="res")
        self.copy(pattern="game-support.md", src=self.source_folder, dst="res")
        tools.rmdir("wiki")
        cmake.install()
