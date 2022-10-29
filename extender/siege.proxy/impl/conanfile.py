import os
from conans import ConanFile, CMake, MSBuild, tools

# conan install . --profile ./local-profile.ini

gcc = "https://github.com/brechtsanders/winlibs_mingw/releases/download/10.3.0-12.0.0-9.0.0-r2/winlibs-i686-posix-dwarf-gcc-10.3.0-llvm-12.0.0-mingw-w64-9.0.0-r2.7z"
filename = "mingw.7z"

class DarkstarHookConan(ConanFile):
    settings = {"os": None, "compiler": None, "build_type": None, "arch": "x86"}

    def requirements(self):
        if not os.path.exists(filename):
            tools.download(gcc, filename)
        if not os.path.exists("mingw32"):
            print(f"Extracting {filename}")
            self.run(f"cmake -E tar x {filename}")

    def _configure_cmake(self):
        with open("lib.bat", "w") as file:
            file.write(f"{tools.vcvars_command(self)}\n")
            file.write("lib %*\n")
        cmake = CMake(self, make_program="/".join(os.path.abspath("mingw32/bin/mingw32-make.exe").split("\\")))
        cmake.definitions["CMAKE_C_COMPILER"] = "/".join(os.path.abspath("mingw32/bin/gcc.exe").split("\\"))
        cmake.definitions["CMAKE_CXX_COMPILER"] = "/".join(os.path.abspath("mingw32/bin/g++.exe").split("\\"))
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        return cmake

    def build(self):
        self._configure_cmake().build()

    def package(self):
        self._configure_cmake().install()
