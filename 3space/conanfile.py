from conans import ConanFile, CMake, tools
import glob
import os.path


class LocalConanFile(ConanFile):
    name = "3space"
    version = "0.6.1"
    url = "https://github.com/open-siege/open-siege"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    build_requires = "cmake/3.22.0"
    settings = "os", "compiler", "build_type", "arch"
    # openssl is here to force package resolution issue with cmake on linux
    requires = "nlohmann_json/3.10.5", "boost/1.78.0", "glm/0.9.9.8", "span-lite/0.10.3", "taocpp-pegtl/3.2.1", "libzip/1.8.0", "openssl/1.1.1o", "catch2/2.13.8"
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


