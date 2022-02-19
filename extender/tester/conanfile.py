from conans import ConanFile, CMake, tools
import os.path

class HelloWxWidgetsConanFile(ConanFile):
    build_requires = "cmake/3.22.0"
    requires = "wxwidgets/3.1.5@bincrafters/stable"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package"

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.install()

