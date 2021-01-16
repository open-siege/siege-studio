import os.path
from conans import ConanFile, CMake, MSBuild, tools

class DarkstarHookConan(ConanFile):
    def requirements(self):
        if not os.path.exists("packages/4.0.1.zip"):
            tools.download("https://github.com/matthew-rindel/Detours/archive/4.0.1.zip", "packages/4.0.1.zip")

        tools.unzip("packages/4.0.1.zip", "packages")

        self.run("cd packages/Detours-4.0.1 && conan export . detours/4.0.1@microsoft/stable")

        self.run("cd darkstar && conan install . --profile ./local-profile.ini")
        self.run("cd darkstar.detours && conan install . -s arch=x86 --build=missing")
        self.run("cd mem && conan install . -s arch=x86 -s build_type=Debug")

    def build(self):
        self.run("cd darkstar && conan build .")
        self.run("cd darkstar.detours && conan build .")
        self.run("cd mem && conan build .")

    def package(self):
        tools.rmdir("package/bin")
        tools.mkdir("package/bin")
        tools.rename("darkstar/build/darkstar.dll", "package/bin/darkstar.dll")
        tools.rename("darkstar/build/functions.json", "package/bin/functions.json")

        if (os.path.exists("darkstar.detours/build/Release")):
            tools.rename("darkstar.detours/build/Release/darkstar.detours.dll", "package/bin/darkstar.detours.dll")
        else:
            tools.rename("darkstar.detours/build/Debug/darkstar.detours.dll", "package/bin/darkstar.detours.dll")
        tools.rename("mem/build/mem.dll", "package/bin/mem.dll")