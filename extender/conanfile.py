from conans import ConanFile, CMake, MSBuild, tools
from shutil import copyfile
import os.path

class DarkstarHookConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    build_requires = "cmake/3.22.0"

    def requirements(self):
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
                "conan install . --profile ./local-profile.ini -s build_type=Debug",
                "cd ..",
                "cd mem",
                "conan install . -s arch=x86 -s build_type=Debug",
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
        self.run("cd darkstar && conan build .")
        self.run("cd darkstar.proxy && conan build .")
        self.run("cd mem && conan build .")
        self.run("cd launcher && conan build .")

    def package(self):
        self.build()
        tools.rmdir("package/bin")
        tools.mkdir("package/bin")
        tools.rename("darkstar.proxy/build/darkstar.proxy.dll", "package/bin/darkstar.proxy.dll")
        tools.rename("darkstar/build/config.json", "package/bin/config.json")

        if (os.path.exists("darkstar/build/Release")):
            tools.rename("darkstar/build/Release/darkstar.dll", "package/bin/darkstar.dll")
            tools.rename("launcher/build/Release/Starsiege.ext.exe", "package/bin/Starsiege.ext.exe")
        else:
            tools.rename("darkstar/build/Debug/darkstar.dll", "package/bin/darkstar.dll")
            tools.rename("launcher/build/Debug/Starsiege.ext.exe", "package/bin/Starsiege.ext.exe")
        tools.rename("mem/build/mem.dll", "package/bin/mem.dll")
