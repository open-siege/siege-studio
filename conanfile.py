import os.path
from conans import ConanFile, CMake, MSBuild, tools

class DarkstarHookConan(ConanFile):
    def requirements(self):
        if not os.path.exists("packages/4.0.1.zip"):
            tools.download("https://github.com/matthew-rindel/Detours/archive/4.0.1.zip", "packages/4.0.1.zip")

        tools.unzip("packages/4.0.1.zip", "packages")

        self.run("cd packages/Detours-4.0.1 && conan export . detours/4.0.1@microsoft/stable")

        self.run("cd darkstar && conan install . -s arch=x86 --build=missing")
        # Release builds cause the dll not to work properly.
        # Tried with various optimisation levels and still has issues. 
        # The debug build isn't too bad since all the main logic has been moved to darkstar.dll
        self.run("cd darkstar.proxy && conan install . --profile ./local-profile.ini -s build_type=Debug --build=missing")
        self.run("cd mem && conan install . -s arch=x86 -s build_type=Debug")

    def build(self):
        self.run("cd darkstar && conan build .")
        self.run("cd darkstar.proxy && conan build .")
        self.run("cd mem && conan build .")

    def package(self):
        tools.rmdir("package/bin")
        tools.mkdir("package/bin")
        tools.rename("darkstar.proxy/build/darkstar.proxy.dll", "package/bin/darkstar.proxy.dll")
        tools.rename("darkstar/build/config.json", "package/bin/config.json")

        if (os.path.exists("darkstar/build/Release")):
            tools.rename("darkstar/build/Release/darkstar.dll", "package/bin/darkstar.dll")
        else:
            tools.rename("darkstar/build/Debug/darkstar.dll", "package/bin/darkstar.dll")
        tools.rename("mem/build/mem.dll", "package/bin/mem.dll")