from conans import ConanFile, CMake, tools
import os

class LocalConanFile(ConanFile):
    build_requires = "cmake/3.17.3", "cppcheck_installer/2.0@bincrafters/stable"
    settings = "os", "compiler", "build_type", "arch"
    requires = "boost_endian/1.69.0@bincrafters/stable", "nlohmann_json/3.9.0"
    generators = "cmake", "virtualenv"
    build_folder = "build"

    def build(self):
        self.build_folder = "build"
        cmake = CMake(self)
        cmake.configure()
        self.run("cppcheck src --error-exitcode=1")
        cmake.build()

