from conans import ConanFile, CMake, tools
import os.path
import glob
# conan install . -s arch=x86

class LocalConanFile(ConanFile):
    name = "starsiege-launcher"
    version = "0.6.2"
    build_requires = "cmake/3.22.0"
    settings = "os", "compiler", "build_type", "arch"
    requires = "detours/4.0.1@microsoft/stable"
    generators = "cmake_find_package"

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.install()

    def imports(self):
        tools.rmdir("cmake")
        tools.mkdir("cmake")
        [tools.rename(file, f"cmake/{file}") for file in glob.glob("*.cmake")]
