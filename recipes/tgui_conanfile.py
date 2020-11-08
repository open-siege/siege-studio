from conans import ConanFile, CMake, tools
import os.path


class TguiConanFile(ConanFile):
    build_requires = "cmake/3.17.3"
    requires = "sfml/2.5.1@bincrafters/stable"
    settings = "arch", "build_type", "compiler"
    generators = "cmake_find_package"
    #TODO add some code to build sfml using the shared options
    options = {
        "shared": [True, False]
    }

    default_options = {
        'shared': False
    }

    def source(self):
        if not os.path.exists("v0.8.8.zip"):
            tools.download("https://github.com/texus/TGUI/archive/v0.8.8.zip", "v0.8.8.zip")
        tools.unzip("v0.8.8.zip")
        os.rename("TGUI-0.8.8", "source")

    def imports(self):
        with open("Findsfml.cmake", "r") as find_sfml:
            contents = find_sfml.readlines()

        with open("Findsfml.cmake", "w") as find_sfml:
            sfml_root = self.deps_cpp_info['sfml'].rootpath.replace("\\", "/")
            find_sfml.write(f"include({sfml_root}/lib/cmake/SFML/SFMLConfig.cmake)\n")
            find_sfml.writelines(contents)
            for key, value in self.deps_cpp_info.dependencies:
                if key == "sfml":
                    continue
                lib_path = value.rootpath.replace("\\", "/") + "/lib"
                if os.path.exists(lib_path):
                    find_sfml.write(f"link_directories({lib_path})\n")

    def _configure_cmake(self):
        self.source_folder = os.path.abspath("source")
        self.build_folder = os.path.abspath("build")
        cmake = CMake(self)


        cmake.definitions["TGUI_SHARED_LIBS"] = self.options.shared
        sfml_root = self.deps_cpp_info['sfml'].rootpath

        cmake.definitions["SFML_STATIC_LIBRARIES"] = not self.options.shared
        cmake.definitions["SFML_ROOT"] = sfml_root
        cmake.configure()
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        suffix = "-s" if not self.options.shared else ""
        suffix += "-d" if self.settings.build_type == "Debug" else ""

        self.cpp_info.libs.append("tgui" + suffix)






