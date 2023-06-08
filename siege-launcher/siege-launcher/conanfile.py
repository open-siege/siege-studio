from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
import glob
import os.path

class HelloImguiSfmlConanFile(ConanFile):
    name = "siege-input"
    version = "0.6.3"
    url = "https://github.com/open-siege/open-siege"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    build_requires = "cmake/3.26.4"
    requires = "imgui/cci.20230105+1.89.2.docking"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

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

    def imports(self):
        self.copy("*.h", src="res/bindings/", dst="bindings")
        self.copy("*.cpp", src="res/bindings/", dst="bindings")
        tools.rmdir("cmake")
        tools.mkdir("cmake")
        [tools.rename(file, f"cmake/{file}") for file in glob.glob("*.cmake")]