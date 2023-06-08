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
    requires = "nlohmann_json/3.11.2", "imgui/cci.20230105+1.89.2.docking", "sdl/2.26.1"
    settings = "os", "compiler", "build_type", "arch", "arch_build"
    generators = "CMakeToolchain", "CMakeDeps"
    exports_sources = "CMakeLists.txt", "include/*", "src/*"

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

    def package_info(self):
        self.cpp_info.libs.append("siege-input")

    def imports(self):
        tools.rmdir("cmake")
        tools.mkdir("cmake")
        [tools.rename(file, f"cmake/{file}") for file in glob.glob("*.cmake")]