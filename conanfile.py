from conans import ConanFile


class DarkstarHookConan(ConanFile):
    settings = {"os": None, "compiler": None, "build_type": None, "arch": "x86"}
    requires = "nlohmann_json/3.8.0", "pybind11/2.5.0"
    generators = "json"

    def imports(self):
        self.copy("*", dst="packages/include", src="@includedirs")

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

