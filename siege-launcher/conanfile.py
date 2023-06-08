from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
import sys
import os.path

class HelloImguiSfmlConanFile(ConanFile):
    name = "siege-input"
    version = "0.6.3"
    url = "https://github.com/open-siege/open-siege"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    build_requires = "cmake/3.26.4"
    settings = "os", "compiler", "build_type", "arch", "arch_build"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        args = sys.argv[3:]
        settings = ' '.join(args)

        self.run(" && ".join([
            "cd siege-input",
            f"conan install . {settings}",
            "conan export . "]), run_environment=True)

        targets = ["siege-launcher"]

        for target in targets:
            commands = [
                f"cd {target}",
                f"conan install . {settings}"
            ]
            self.run(" && ".join(commands), run_environment=True)


    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.install()