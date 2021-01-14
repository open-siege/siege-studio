import os
from conans import ConanFile, CMake, MSBuild, tools

if not os.path.exists("local-env.ini"):
    with open("local-env.ini", "w") as env:
        env.write("")

class DarkstarHookConan(ConanFile):
    settings = {"os": None, "compiler": None, "build_type": None, "arch": "x86"}
    build_requires = "cmake/3.17.3"
    requires = "nlohmann_json/3.8.0"
    generators = "json"

    def build(self):
        self.build_folder = "build"
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        os.remove("build/darkstar.dll") if os.path.exists("build/darkstar.dll") else None
        os.rename("build/libdarkstar.dll", "build/darkstar.dll")

        if not os.path.exists(f"{self.source_folder}/src/mem/libs"):
            os.mkdir(f"{self.source_folder}/src/mem/libs")

        msbuild = MSBuild(self)
        msbuild.build_env.lib_paths.append(f"{self.source_folder}/src/mem/libs")
        msbuild.build("src/mem/mem.sln", upgrade_project=False, toolset="v142")

    def imports(self):
            self.copy("*", src="@includedirs", dst="packages/include")
