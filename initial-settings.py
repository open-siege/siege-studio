from conans import ConanFile, CMake, tools
import os.path
import sys

class LocalConanFile(ConanFile):
    settings = "arch_build"
    generator = []

    def requirements(self):
        profile = "default"

        if "--profile" in sys.argv:
            profile = sys.argv[sys.argv.index("--profile") + 1]
            profile = os.path.abspath(profile) if profile != "default" else profile

        print(f"Configuring CMake arch to {self.settings.arch_build} for {profile} profile. Helps cross-compiling.")
        self.run(f"conan profile update settings.cmake:arch={self.settings.arch_build} {profile}")

        print(f"Adding the bincrafters remote @ https://bincrafters.jfrog.io/artifactory/api/conan/public-conan.")
        self.run(f"conan remote add bincrafters https://bincrafters.jfrog.io/artifactory/api/conan/public-conan --force")

        print("Configuring settings to work correctly with bincrafters.")
        self.run(f"conan config set general.revisions_enabled=1")
