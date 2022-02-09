from conans import ConanFile, CMake, tools
import glob
import os.path


class LocalConanFile(ConanFile):
    name = "siege-shell"
    version = "0.5.1"
    url = "https://github.com/3space-studio/3space-studio"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    build_requires = "cmake/3.22.0"
    settings = "os", "compiler", "build_type", "arch"
    requires = "3space/0.5.1", "wxwidgets/3.1.5@bincrafters/stable"
    generators = "cmake_find_package"

    def build(self):
        cmake = CMake(self)
        print(f"Command line args: {cmake.command_line}")
        print(f"Build args: {cmake.build_config}")
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.install()

    def imports(self):
        #TODO will need some tweaks to make it work cross platform
        # Could also just export a function and call it here
        self.run("svg2png")
        tools.rmdir("cmake")
        tools.mkdir("cmake")
        [tools.rename(file, f"cmake/{file}") for file in glob.glob("*.cmake")]


