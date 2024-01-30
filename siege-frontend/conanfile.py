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
    requires = "3space/0.6.3", "imgui/cci.20230105+1.89.2.docking", "sdl/2.28.5", "catch2/3.5.2", "cpr/1.10.5"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("xz_utils/5.4.4", override=True)

    def layout(self):
        cmake_layout(self)

    def configure(self):
        self.options["sdl"].shared = False
        self.options["sdl"].opengl = False
        self.options["sdl"].opengles = False
        self.options["sdl"].vulkan = False
        self.options["sdl"].sdl2main = False

        if self.settings.os == "Linux":
            self.options["sdl"].wayland = False

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.install()

    def generate(self):
        dep = self.dependencies["imgui"]
        (base, other) = os.path.split(dep.cpp_info.libdirs[0])
        fullDir = os.path.join(base, "res", "bindings")
        dstDirs = [os.path.join(self.source_folder, "siege-interface", "bindings"), os.path.join(self.source_folder, "siege-launcher-win32", "bindings")]
        print(fullDir)
        print(dstDirs)

        for dstDir in dstDirs:
            if not os.path.exists(dstDir):
                os.makedirs(dstDir)
            copy(self, "*.h", fullDir, dstDir)
            copy(self, "*.cpp", fullDir, dstDir)       
            