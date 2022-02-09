from conans import ConanFile, CMake, MSBuild, tools
from shutil import copyfile
import os.path

class DarkstarHookConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    build_requires = "cmake/3.22.0"

    def requirements(self):
        if self.settings.arch != "x86":
            return

        self.run("conan install cmake/3.22.0@/ -g virtualenv")
        activate = "activate.bat" if self.settings.os == "Windows" else "./activate.sh"
        install = "install.bat" if self.settings.os == "Windows" else "./install.sh"
        copyfile(activate, install)

        # Release builds cause the dll not to work properly.
        # Tried with various optimisation levels and still has issues.
        # The debug build isn't too bad since all the main logic has been moved to darkstar.dll
        with open(install, "a") as file:
            commands = [
                "cd detours",
                f"conan source .",
                "conan export . detours/4.0.1@microsoft/stable",
                "cd ..",
                "cd darkstar",
                "conan install . -s arch=x86 --build=missing",
                "cd ..",
                "cd darkstar.proxy",
                "conan install . -s arch=x86",
                "cd ..",
                "cd mem",
                "conan install . -s arch=x86",
                "cd ..",
                "cd launcher",
                "conan install . -s arch=x86 --build=missing",
                "cd .."
            ]

            file.write("\n" + " && ".join(commands))

        if self.settings.os != "Windows":
            self.run(f"chmod +x {install}")
        self.run(install)

    def build(self):
        cmake = CMake(self)
        cmake.definitions["CONAN_CMAKE_SYSTEM_PROCESSOR"] = self.settings.arch
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.definitions["CONAN_CMAKE_SYSTEM_PROCESSOR"] = self.settings.arch
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.install()
