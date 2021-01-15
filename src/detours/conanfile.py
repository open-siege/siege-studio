import os
import shutil
from conans import ConanFile, CMake, MSBuild, tools

if not os.path.exists("CMakeLists.txt"):
    with open("CMakeLists.txt", "w") as env:
        env.write("cmake_minimum_required(VERSION 3.16)\n")
        env.write("project(detours)\n")
        env.write("add_library(detours SHARED src/detours.cpp\n")
        env.write("src/modules.cpp\n")
        env.write("src/disasm.cpp\n")
        env.write("src/image.cpp\n")
        env.write("src/creatwth.cpp\n")
        env.write("src/disolx86.cpp\n")
        env.write("src/disolx64.cpp\n")
        env.write("src/disolia64.cpp\n")
        env.write("src/disolarm.cpp\n")
        env.write("src/disolarm64.cpp)\n")
        env.write("target_link_libraries(detours)\n")
        env.write("install(TARGETS detours CONFIGURATIONS Debug RUNTIME DESTINATION bin)\n")
        env.write("install(TARGETS detours CONFIGURATIONS Release RUNTIME DESTINATION bin)\n")


class DetoursConanfile(ConanFile):
    settings = {"os": None, "compiler": None, "build_type": None, "arch": "x86"}
    build_requires = "cmake/3.17.3"

    def _configure_cmake(self):
        self.source_folder = os.path.abspath(".")
        self.build_folder = os.path.abspath("bin")
        cmake = CMake(self)

        cmake.configure()
        return cmake

    def source(self):
        if not os.path.exists("src/detours.cpp"):
            if not os.path.exists("4.0.1.zip"):
                tools.download("https://github.com/microsoft/Detours/archive/4.0.1.zip", "4.0.1.zip")
            tools.unzip("4.0.1.zip", "tmp", pattern="Detours-4.0.1/src/*")
            os.remove("src") if os.path.exists("src") else None
            shutil.move("tmp/Detours-4.0.1/src", ".")
            shutil.rmtree("tmp")

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs.append("detours")