from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
import glob
import os.path


class LocalConanFile(ConanFile):
    name = "starsiege-launcher"
    version = "0.0.1"
    url = "https://github.com/open-siege/open-siege"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    build_requires = "cmake/3.26.4"
    settings = "os", "compiler", "build_type", "arch"
    requires = "catch2/3.3.2", "cpr/1.10.1", "3space/0.6.3", "zlib/1.2.13"
    generators = "CMakeToolchain", "CMakeDeps", "virtualenv"
    exports_sources = "CMakeLists.txt", "LICENSE", "README.md", "game-support.md", "src/*"

    def layout(self):
        cmake_layout(self)

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


