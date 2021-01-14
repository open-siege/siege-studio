import os
from conans import ConanFile, CMake, MSBuild, tools

if not os.path.exists("local-env.ini"):
    possible_paths = [
        "BCC103/bin/bcc32c.exe",
        "../BCC103/bin/bcc32c.exe",
        "bin/bcc32c.exe",
        "../bin/bcc32c.exe",
        os.path.expandvars("%PROGRAMFILES(X86)%/Embarcadero/Studio/20.0/bin/bcc32c.exe"),
        os.path.expandvars("%PROGRAMFILES%/Embarcadero/Studio/20.0/bin/bcc32c.exe")
    ]

    for path in possible_paths:
        if os.path.exists(path):
            path = os.path.abspath(path)
            print(f"Found C++ Builder at: {path}. Writing local-env.ini")
            with open("local-env.ini", "w") as env:
                env.write("[env]\n")
                env.write(f'CC="{path}"\n')
                env.write(f'CXX="{path}"\n')
            break
    if not os.path.exists("local-env.ini"):
        print("Could not find C++ Builder. Install C++ Builder or configure local-env.ini manually.")

class DarkstarHookConan(ConanFile):
    settings = {"os": None, "compiler": None, "build_type": None, "arch": "x86"}
    requires = "nlohmann_json/3.8.0"
    generators = "json"

    def build(self):
        # TODO make this a bit better
        #path = '"%PROGRAMFILES(X86)%\\Embarcadero\\Studio\\20.0\\bin\\rsvars.bat"'
        #self.run(f"{path} && msbuild {self.source_folder}\\src\\darkstar\\darkstar.cbproj")

        self.build_folder = "build"
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

        if not os.path.exists(f"{self.source_folder}/src/mem/libs"):
            os.mkdir(f"{self.source_folder}/src/mem/libs")

        msbuild = MSBuild(self)
        msbuild.build_env.lib_paths.append(f"{self.source_folder}/src/mem/libs")
        msbuild.build("src/mem/mem.sln", upgrade_project=False)

    def imports(self):
            self.copy("*", src="@includedirs", dst="packages/include")
