from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import copy
import glob
import os.path

class SiegeLauncherConanFile(ConanFile):
    name = "siege-frontend"
    version = "0.6.3"
    url = "https://github.com/open-siege/open-siege"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    build_requires = "cmake/3.26.4"
    requires = "3space/0.6.3", "sdl/2.28.5", "catch2/3.5.2", "cpr/1.10.5"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("xz_utils/5.4.4", override=True)

    def layout(self):
        cmake_layout(self)

    def configure(self):
        self.options["cpr"].shared = False
        self.options["libcurl"].shared = False
        self.options["sdl"].shared = False
        self.options["sdl"].opengl = False
        self.options["sdl"].opengles = False
        self.options["sdl"].vulkan = False
        self.options["sdl"].sdl2main = False

        if self.settings.os == "Linux":
            self.options["sdl"].wayland = False

        if self.settings.os == "Windows":
            self.options["libcurl"].with_ssl = "schannel"
            self.options["cpr"].with_ssl = "winssl"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.install()
