from conans import ConanFile, CMake, tools
import sys
import os.path

class HelloImguiSfmlConanFile(ConanFile):
    name = "siege-input"
    version = "0.6.3"
    url = "https://github.com/open-siege/open-siege"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    build_requires = "cmake/3.22.0"
    settings = "os", "compiler", "build_type", "arch", "arch_build"
    generators = "cmake_find_package"

    def requirements(self):
        args = sys.argv[3:]
        settings = ' '.join(args)
        targets = ["lib", "app"]

        self.run(" && ".join([
            "cd lib",
            "conan export . "]), run_environment=True)

        for target in targets:
            commands = [
                f"cd {target}",
                f"conan install . {settings}"
            ]
            self.run(" && ".join(commands), run_environment=True)


    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.install()