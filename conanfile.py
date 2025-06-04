from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import copy
import glob
import os.path

include_path = os.path.abspath("libzt/include")

class SiegeLauncherConanFile(ConanFile):
    name = "open-siege"
    version = "0.6.3"
    url = "https://github.com/open-siege/open-siege"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    build_requires = "cmake/3.31.6"
    requires = "glm/cci.20230113", "taocpp-pegtl/3.2.7", "libzip/1.9.2", "catch2/3.8.1", "nlohmann_json/3.9.1", "zlib/1.3.1"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        if self.settings.os == "Windows":
            if not os.path.isdir("libzt"):
                self.run("git clone https://github.com/open-siege/libzt.git --branch 1.8.10 --recurse-submodules --depth=1")
            self.run(f"conan install detours-conanfile.py -s build_type=Release -s compiler.runtime=static -s arch={self.settings.arch} --build=missing -of siege-modules/siege-extension/detours")
        
        self.requires("xz_utils/5.4.4", override=True)

    def layout(self):
        cmake_layout(self)

    def generate(self):
        info = self.dependencies["nlohmann_json"].cpp_info
        copy(self, "*.h", info.includedirs[0], include_path)
        copy(self, "*.hpp", info.includedirs[0], include_path)

    def configure(self):
        self.options["catch2"].shared = False
        self.options["libzip"].shared = False

        if self.settings.os == "Windows":
            self.options["libzip"].crypto = False

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.install()
