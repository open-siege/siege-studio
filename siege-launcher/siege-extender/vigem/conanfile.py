import os
import shutil
from conans import ConanFile, CMake, MSBuild, tools

# To add this to the local cache:
# conan export . detours/4.0.1@microsoft/stable

def remove(path):
    if os.path.exists(path):
        os.remove(path)

class DetoursConanfile(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    build_requires = "cmake/3.22.0"

    def source(self):
        tools.rmdir("code")
        tools.rmdir("include")
        tools.rmdir("src")
        files_to_remove = [".gitignore", "LICENSE", "README.md", "CMakeLists.txt",
                           "ViGEmClient.sln", "ViGEmClient.sln.DotSettings", "appveyor.yml",
                           "src/ViGEmClient.vcxproj", "src/ViGEmClient.vcxproj.filters"]

        for file in files_to_remove:
            remove(file)
        git = tools.Git(folder="code")
        git.clone("https://github.com/ViGEm/ViGEmClient.git", shallow=True)
        tools.rmdir("code/.git")
        tools.rmdir("code/.github")
        file_names = os.listdir("code")
        for file_name in file_names:
            shutil.move(os.path.join("code", file_name), ".")

        files_to_remove.remove("CMakeLists.txt")
        for file in files_to_remove:
            remove(file)

        with open("CMakeLists.txt", "a") as cmake_file:
            cmake_file.write('\ninstall(DIRECTORY include DESTINATION . COMPONENT devel FILES_MATCHING PATTERN "*.hpp")\n')
            cmake_file.write('install(TARGETS ViGEmClient CONFIGURATIONS Debug Release RUNTIME DESTINATION lib)\n')

        with open('CMakeLists.txt', 'r+') as cmake_file:
            content = cmake_file.read()
            cmake_file.seek(0)
            cmake_file.truncate()
            cmake_file.write(content.replace('ViGEmClient EXCLUDE_FROM_ALL', 'ViGEmClient STATIC'))


    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.install()

    def package_info(self):
        self.cpp_info.libs.append("ViGEmClient")
