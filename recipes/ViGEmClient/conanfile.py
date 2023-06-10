import os
from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import get, patch



class LocalConanFile(ConanFile):
    name = "vigemclient"
    version = "1.21.222.0"
    build_requires = "cmake/3.26.4"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    exports_sources = "*.patch"

    def source(self):
        get(self, "https://codeload.github.com/ViGEm/ViGEmClient/zip/refs/tags/v1.21.222.0", strip_root=True)
        patch_file = os.path.join(self.export_sources_folder, "cmake_update.patch")
        patch(self, patch_file=patch_file)


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

    def package_info(self):
        self.cpp_info.libs.append("ViGEmClient")


