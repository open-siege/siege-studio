from conans import ConanFile, tools
import os.path
import json

class DarkstarHookConan(ConanFile):
    settings = {"os": None, "compiler": None, "build_type": None, "arch": "x86"}
    requires = "nlohmann_json/3.8.0", "pybind11/2.5.0"
    generators = "json"

    def imports(self):
        self.copy("*", src="@includedirs", dst="packages/include")

        # There is an internal issue with the library implementation for
        # C++ Builder, which prevents detail::all_of<is_valid_class_option<options>...>::value
        # from evaluating correctly even though the template arguments are correct.
        # We just bypass that static_assert currently on line 1059 and 1060.
        with open("packages/include/pybind11/pybind11.h", "r") as pybind11:
            lines = pybind11.readlines()

        with open("packages/include/pybind11/pybind11.h", "w") as pybind11:
            lines[1059 - 1] = f"//{lines[1059 - 1]}"
            lines[1060 - 1] = f"//{lines[1060 - 1]}"
            pybind11.writelines(lines)

        # Download the source package for Python then copy the headers.
        with open("local-config.json", "r") as localConfigFile:
            localConfig = json.loads(localConfigFile.read())

        pythonSrcFile = os.path.split(localConfig["pythonSrcPackage"])[-1]
        pythonFolderName = ".".join(pythonSrcFile.split(".")[:-1])
        pythonSrcFile = f"tmp/{pythonSrcFile}"

        if not os.path.exists(pythonSrcFile):
            tools.download(localConfig["pythonSrcPackage"], pythonSrcFile)

        tools.unzip(pythonSrcFile, "tmp", pattern=f"{pythonFolderName}/Include/*")
        tools.unzip(pythonSrcFile, "tmp", pattern=f"{pythonFolderName}/PC/*")

        self.copy("*.h", src=os.path.abspath(f"tmp/{pythonFolderName}/Include"), dst="packages/include")
        self.copy("*.h", src=os.path.abspath(f"tmp/{pythonFolderName}/PC"), dst="packages/include")
        tools.rmdir(f"tmp/{pythonFolderName}")

