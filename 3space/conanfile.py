from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
import glob
import os.path


class LocalConanFile(ConanFile):
    name = "3space"
    version = "0.6.3"
    url = "https://github.com/open-siege/open-siege"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    build_requires = "cmake/3.26.4"
    settings = "os", "compiler", "build_type", "arch"
    # openssl is here to force package resolution issue with cmake on linux
    requires = "glm/cci.20230113", "span-lite/0.10.3", "taocpp-pegtl/3.2.7", "libzip/1.9.2", "catch2/3.3.2"
    generators = "CMakeToolchain", "CMakeDeps"
    exports_sources = "CMakeLists.txt", "include/*", "src/*"

    def requirements(self):
        self.requires("boost/1.82.0", transitive_headers=True)
        self.requires("nlohmann_json/3.11.2", transitive_headers=True)

    def layout(self):
        cmake_layout(self)

    def configure(self):
        self.options["boost"].shared = False
        self.options["boost"].header_only = True
        self.options["boost"].bzip2 = False
        self.options["boost"].zlib = False
        self.options["boost"].numa = False

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs.append("3space")

