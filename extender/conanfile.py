from conans import ConanFile, CMake, MSBuild, tools
import json
import os.path

class DarkstarHookConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch", "arch_build"
    build_requires = "cmake/3.22.0"

    def requirements(self):
        if self.settings.arch != "x86":
            return

        settings = f"-s arch={self.settings.arch} --build=missing"

        self.run("conan install cmake/3.22.0@/ -g json")

        with open("conanbuildinfo.json", "r") as info:
            data = json.load(info)
            deps_env_info = data["deps_env_info"]
            os.environ["PATH"] += os.pathsep + deps_env_info["PATH"][0] + os.sep
            os.environ["CMAKE_ROOT"] = deps_env_info["CMAKE_ROOT"]
            os.environ["CMAKE_MODULE_PATH"] = deps_env_info["CMAKE_MODULE_PATH"]

        self.run(" && ".join([
            "cd detours",
            "conan source .",
            "conan export . detours/4.0.1@microsoft/stable"]), run_environment=True)

        targets = ["darkstar", "darkstar.proxy", "mem", "launcher", "tray-player", "tester"]

        for target in targets:
            self.run(" && ".join([f"cd {target}", f"conan install . {settings}"]), run_environment=True)


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
