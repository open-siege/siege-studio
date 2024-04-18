from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.system.package_manager import Apt
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
    requires = "glm/cci.20230113", "taocpp-pegtl/3.2.7", "libzip/1.9.2", "catch2/3.5.2"
    generators = "CMakeToolchain", "CMakeDeps"
    exports_sources = "CMakeLists.txt", "include/*", "src/*", "tools/*"

    # TODO make this more robust so that it can work for other distro's
    # or add a proper check for Ubuntu specifically
    def system_requirements(self):
        if self.settings.os == "Linux":
            apt = Apt(self)
            apt.install(["libtbb-dev"], update=True, check=True)


    def requirements(self):
        self.requires("nlohmann_json/3.11.2", transitive_headers=True)

    def layout(self):
        cmake_layout(self)

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

