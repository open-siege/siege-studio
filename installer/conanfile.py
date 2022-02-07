from conans import ConanFile, CMake, tools
import glob
import os.path


class LocalConanFile(ConanFile):
    name = "starsiege-launcher"
    version = "0.0.1"
    url = "https://github.com/StarsiegePlayers/3space-studio"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    system_requires = "opengl/system"
    build_requires = "cmake/3.17.3"
    settings = "os", "compiler", "build_type", "arch"
    requires = "wxwidgets/3.1.3@bincrafters/stable","catch2/2.13.4"
    generators = "cmake_find_package", "virtualenv"
    exports_sources = "CMakeLists.txt", "LICENSE", "README.md", "game-support.md", "src/*"

    def _configure_cmake(self):
        self.source_folder = os.path.abspath(".")
        self.build_folder = os.path.abspath("build")
        cmake = CMake(self)

        cmake.configure()
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()
        cmake.test()

    def package(self):
        cmake = self._configure_cmake()
        self.copy(pattern="LICENSE", src=self.source_folder, dst="licenses")
        self.copy(pattern="README.md", src=self.source_folder, dst="res")
        cmake.install()

    def imports(self):
        tools.rmdir("cmake")
        tools.mkdir("cmake")
        [tools.rename(file, f"cmake/{file}") for file in glob.glob("*.cmake")]


