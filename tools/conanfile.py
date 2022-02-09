from conans import ConanFile, CMake, tools
from shutil import copyfile
import os.path


class LocalConanFile(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    build_requires = "cmake/3.22.0"

    def requirements(self):
        targets = ["dts-to-json", "json-to-dts", "dts-to-obj", "unvol"]

        for target in targets:
            commands = [f"cd {target}", f"conan install . -s build_type={self.settings.build_type} --build=missing"]
            self.run(" && ".join(commands), run_environment=True)


    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.install()
