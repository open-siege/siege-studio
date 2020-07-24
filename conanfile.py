from conans import ConanFile, MSBuild, tools


class DarkstarHookConan(ConanFile):
    settings = {"os": None, "compiler": None, "build_type": None, "arch": "x86"}
    requires = "nlohmann_json/3.8.0"
    generators = "json"

    def build(self):
        # TODO make this a bit better
        path = '"%PROGRAMFILES(X86)%\\Embarcadero\\Studio\\20.0\\bin\\rsvars.bat"'
        self.run(f"{path} && msbuild {self.source_folder}\\src\\darkstar\\darkstar.cbproj")

        msbuild = MSBuild(self)
        msbuild.build_env.lib_paths.append(f"{self.source_folder}/src/mem/libs")
        msbuild.build("src/mem/mem.sln", upgrade_project=False)

    def imports(self):
        self.copy("*", src="@includedirs", dst="packages/include")
