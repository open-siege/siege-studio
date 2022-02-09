from conans import ConanFile, CMake, tools
import glob
import os.path


class LocalConanFile(ConanFile):
    name = "3space"
    version = "0.5.1"
    url = "https://github.com/3space-studio/3space-studio"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    build_requires = "cmake/3.22.0"
    settings = "os", "compiler", "build_type", "arch"
    requires = "nlohmann_json/3.9.0", "boost/1.76.0", "glm/0.9.9.8", "span-lite/0.9.0", "taocpp-pegtl/3.1.0", "catch2/2.13.4"
    generators = "cmake_find_package"
    exports_sources = "CMakeLists.txt", "include/*", "src/*"

    def configure(self):
        self.options["boost"].shared = False
        self.options["boost"].header_only = True
        self.options["boost"].bzip2 = False
        self.options["boost"].zlib = False
        self.options["boost"].numa = False

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()
        cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.install()

    def package_info(self):
        self.cpp_info.libs.append("3space")

    def imports(self):
        tools.rmdir("cmake")
        tools.mkdir("cmake")
        [tools.rename(file, f"cmake/{file}") for file in glob.glob("*.cmake")]


