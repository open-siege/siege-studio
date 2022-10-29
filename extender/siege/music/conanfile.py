from conans import ConanFile, CMake, tools
import glob
import os.path

class HelloWxWidgetsConanFile(ConanFile):
    build_requires = "cmake/3.22.0"
    requires = "sfml/2.5.1"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package"

    def imports(self):
        tools.rmdir("cmake")
        tools.mkdir("cmake")
        [tools.rename(file, f"cmake/{file}") for file in glob.glob("*.cmake")]

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.install()

