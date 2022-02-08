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

    def build(self):
        cmake = CMake(self)
        cmake.definitions["CMAKE_MAKE_PROGRAM"] = "/".join(os.path.abspath("mingw32/bin/mingw32-make.exe").split("\\"))
        cmake.definitions["CMAKE_C_COMPILER"] = "/".join(os.path.abspath("mingw32/bin/gcc.exe").split("\\"))
        cmake.definitions["CMAKE_CXX_COMPILER"] = "/".join(os.path.abspath("mingw32/bin/g++.exe").split("\\"))
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()
        os.remove("build/darkstar.proxy.dll") if os.path.exists("build/darkstar.proxy.dll") else None
        os.rename("build/libdarkstar.proxy.dll", "build/darkstar.proxy.dll")
