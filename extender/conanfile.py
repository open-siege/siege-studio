from conans import ConanFile, CMake, MSBuild, tools
from shutil import copyfile
import os.path

class DarkstarHookConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch", "arch_build"
    build_requires = "cmake/3.22.0"

    def requirements(self):
        if self.settings.arch != "x86":
            return

        settings = f"-s cmake:arch={self.settings.arch_build} -s arch={self.settings.arch} --build=missing"

        self.run(" && ".join([
            "cd detours",
            "conan source .",
            "conan export . detours/4.0.1@microsoft/stable"]), run_environment=True)

        self.run(" && ".join([
            "cd darkstar",
            f"conan install . {settings}"]), run_environment=True)

        # Release builds cause the dll not to work properly.
        # Tried with various optimisation levels and still has issues.
        # The debug build isn't too bad since all the main logic has been moved to darkstar.dll
        self.run(" && ".join([
            "cd darkstar.proxy",
            "conan install . -s arch=x86"]), run_environment=True)

        self.run(" && ".join(["cd mem", f"conan install . {settings}"]), run_environment=True)

        self.run(" && ".join(["cd launcher", f"conan install . {settings}"]), run_environment=True)

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
