from conans import ConanFile, CMake, tools
import os.path
import sys


class LocalConanFile(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    build_requires = "cmake/3.22.0"

    def requirements(self):
        profile = "default"

        if "--profile" in sys.argv:
            profile = sys.argv[sys.argv.index("--profile") + 1]
            profile = os.path.abspath(profile) if profile != "default" else profile
        targets = ["dts-to-json", "json-to-dts", "dts-to-obj", "unvol"]

        for target in targets:
            commands = [f"cd {target}", f"conan install . --profile {profile} -s build_type={self.settings.build_type} --build=missing"]
            self.run(" && ".join(commands), run_environment=True)


    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.install()
