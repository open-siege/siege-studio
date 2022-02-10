from conans import ConanFile, CMake, tools
import os.path
import sys
import shutil

class LocalConanFile(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    build_requires = "cmake/3.22.0"

    def requirements(self):
        profile = "default"

        if "--profile" in sys.argv:
            profile = sys.argv[sys.argv.index("--profile") + 1]
            profile = os.path.abspath(profile) if profile != "default" else profile

        commands = [
            "cd 3space-studio",
            "pip3 install -r requirements.txt",
            "cd ..",
            "cd siege-shell",
            "pip3 install -r requirements.txt"
        ]

        self.run(" && ".join(commands), run_environment=True)

        targets = ["tools", "siege-shell", "3space-studio", "extender"]
        shutil.rmtree("cmake")
        os.mkdir("cmake")
        commands = [
            "cd 3space",
            f"conan install . --profile {profile} -s build_type={self.settings.build_type} -s arch={self.settings.arch} --build=missing",
            "conan export .",
            "cd ..",
            "cd cmake",
            f"conan install 3space/0.5.1@/ -g cmake_find_package --profile {profile} -s build_type={self.settings.build_type} -s arch={self.settings.arch} --build=missing"
        ]
        self.run(" && ".join(commands), run_environment=True)


        for target in targets:
            commands = [
                f"cd {target}",
                f"conan install . --profile {profile} -s build_type={self.settings.build_type} -s arch={self.settings.arch} --build=missing"
            ]
            self.run(" && ".join(commands), run_environment=True)


    def build(self):
        cmake = CMake(self)
        cmake.definitions["CONAN_CMAKE_SYSTEM_PROCESSOR"] = self.settings.arch
        cmake.definitions["CI"] = os.environ["CI"] if "CI" in os.environ else "False"
        cmake.definitions["PYTHON_EXECUTABLE"] = sys.executable
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()

    def package(self):
        git = tools.Git(folder="wiki")
        git.clone("https://github.com/StarsiegePlayers/3space-studio.wiki.git", shallow=True)
        tools.rmdir("wiki/.git")
        cmake = CMake(self)
        cmake.definitions["CONAN_CMAKE_SYSTEM_PROCESSOR"] = self.settings.arch
        cmake.definitions["CI"] = os.environ["CI"] if "CI" in os.environ else "False"
        cmake.definitions["PYTHON_EXECUTABLE"] = sys.executable
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        self.copy(pattern="LICENSE", src=self.source_folder, dst="licenses")
        self.copy(pattern="README.md", src=self.source_folder, dst="res")
        self.copy(pattern="wiki/*.md", src=self.source_folder, dst="res")
        self.copy(pattern="game-support.md", src=self.source_folder, dst="res")
        tools.rmdir("wiki")
        cmake.install()
