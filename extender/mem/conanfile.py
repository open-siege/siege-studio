from conans import ConanFile, CMake, tools
import os.path


class LocalConanFile(ConanFile):
    name = "darkstar-mem"
    version = "0.6.2"
    url = "https://github.com/open-siege/open-siege"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    build_requires = "cmake/3.22.0"
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.run("cd impl && conan install . -s arch=x86 -s build_type=Debug")

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.install()

    def package_info(self):
        self.cpp_info.libs.append("darkstar.proxy")


