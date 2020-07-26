from conans import ConanFile, CMake, tools

class LocalConanFile(ConanFile):
    build_requires = "cmake/3.17.3"
    settings = "os", "compiler", "build_type", "arch"
    requires = "boost_endian/1.69.0@bincrafters/stable"
    generators = "cmake"
    build_folder = "build"

    def build(self):
        self.build_folder = "build"
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

