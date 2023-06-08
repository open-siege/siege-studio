from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
import glob
import os.path


class LocalConanFile(ConanFile):
    name = "json-to-dts"
    version = "0.6.3"
    url = "https://github.com/open-siege/open-siege"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    build_requires = "cmake/3.26.4"
    settings = "os", "compiler", "build_type", "arch"
    requires = "3space/0.6.3"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        if self.settings.os == "Linux":
            self.requires("tbb/2020.3")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.install()

    def imports(self):
        tools.rmdir("cmake")
        tools.mkdir("cmake")
        [tools.rename(file, f"cmake/{file}") for file in glob.glob("*.cmake")]


