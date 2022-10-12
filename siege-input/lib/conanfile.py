from conans import ConanFile, CMake, tools
import glob
import os.path

class HelloImguiSfmlConanFile(ConanFile):
    name = "siege-input"
    version = "0.6.3"
    url = "https://github.com/open-siege/open-siege"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    build_requires = "cmake/3.22.0"
    # openssl is here to force package resolution issue with cmake on linux
    requires = "nlohmann_json/3.10.5", "imgui/cci.20220621+1.88.docking", "sdl/2.24.0", "openssl/1.1.1o"
    settings = "os", "compiler", "build_type", "arch", "arch_build"
    generators = "cmake_find_package"
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
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.install()

    def package_info(self):
        self.cpp_info.libs.append("siege-input")

    def imports(self):
        tools.rmdir("cmake")
        tools.mkdir("cmake")
        [tools.rename(file, f"cmake/{file}") for file in glob.glob("*.cmake")]