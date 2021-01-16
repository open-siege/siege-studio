import os
from conans import ConanFile, CMake, MSBuild, tools

#conan install . -s arch=x86 -s build_type=Debug

class DarkstarHookConan(ConanFile):
    settings = {"os": None, "compiler": None, "build_type": None, "arch": "x86"}

    def build(self):
        if not os.path.exists(f"{self.source_folder}/libs"):
            os.mkdir(f"{self.source_folder}/libs")

        msbuild = MSBuild(self)
        msbuild.build_env.lib_paths.append(f"{self.source_folder}/libs")
        msbuild.build("mem.sln", upgrade_project=False, toolset="v142")

