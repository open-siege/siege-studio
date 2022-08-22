from conans import ConanFile, CMake, tools
import glob
import os.path
import sys

# conan install . -s arch=x86

class LocalConanFile(ConanFile):
    name = "darkstar-core"
    version = "0.6.2"
    build_requires = "cmake/3.22.0"
    settings = "os", "compiler", "build_type", "arch"
    requires = "detours/4.0.1@microsoft/stable", "sqlite3/3.37.2", "nlohmann_json/3.10.5", "sfml/2.5.1", "sdl/2.0.20", "catch2/2.13.8"
    generators = "cmake_find_package"

    def configure(self):
        self.options["sqlite3"].enable_json1 = True
        self.options["sdl"].shared = False
        self.options["sdl"].opengl = False
        self.options["sdl"].opengles = False
        self.options["sdl"].vulkan = False
        self.options["sdl"].sdl2main = False


    def requirements(self):
        args = sys.argv[3:]
        for index, value in enumerate(args):
            if value == "--profile":
                profile = args[index + 1]
                args[index + 1] = os.path.abspath(profile) if os.path.exists(profile) else profile

        settings = ' '.join(args)
        commands = [
            f"cd music",
            f"conan install . {settings}"
        ]
        self.run(" && ".join(commands), run_environment=True)

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()
        cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.install()

    def imports(self):
        tools.rmdir("cmake")
        tools.mkdir("cmake")
        [tools.rename(file, f"cmake/{file}") for file in glob.glob("*.cmake")]

