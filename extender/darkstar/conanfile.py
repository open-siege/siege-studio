from conans import ConanFile, CMake, tools
import glob
import os.path

# conan install . -s arch=x86

class LocalConanFile(ConanFile):
    name = "darkstar-core"
    version = "0.5.1"
    build_requires = "cmake/3.22.0"
    settings = "os", "compiler", "build_type", "arch"
    requires = "detours/4.0.1@microsoft/stable", "nlohmann_json/3.10.5", "catch2/2.13.4"
    generators = "cmake_find_package"

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()
        cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.install()

    def imports(self):
        tools.rmdir("cmake")
        tools.mkdir("cmake")
        [tools.rename(file, f"cmake/{file}") for file in glob.glob("*.cmake")]

