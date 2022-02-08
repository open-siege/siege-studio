import os
import glob
import shutil
from conans import ConanFile, CMake, MSBuild, tools

# To add this to the local cache:
# conan export . detours/4.0.1@microsoft/stable

class DetoursConanfile(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    build_requires = "cmake/3.17.3"
    exports_sources = "CMakeLists.txt", "src/*"

    def source(self):
        tools.rmdir("code")
        tools.rmdir("src")
        tools.rmdir("include")
        git = tools.Git(folder="code")
        git.clone("https://github.com/microsoft/Detours.git", shallow=True)
        tools.rmdir("code/.git")
        tools.mkdir("src")
        for child in self.exports_sources:
            file_name = os.path.join("code", child)
            files = glob.glob(file_name)
            for file in files:
                shutil.move(file, os.path.join(*(file.split(os.path.sep)[1:])))
        tools.rmdir("code")

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.install()

    def package_info(self):
        self.cpp_info.libs.append("detours")
