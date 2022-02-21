from conans import ConanFile, CMake, tools
import glob
import os.path

class HelloWxWidgetsConanFile(ConanFile):
    build_requires = "cmake/3.22.0"
    requires = "wxwidgets/3.1.5@bincrafters/stable", "mio/cci.20201220", "sfml/2.5.1"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package"

    def configure(self):
        self.options["sqlite3"].enable_json1 = True
        self.options["wxwidgets"].mediactrl = True
        self.options["wxwidgets"].zlib = "off"
        self.options["wxwidgets"].png = "off"
        self.options["wxwidgets"].jpeg = "off"
        self.options["wxwidgets"].tiff = "off"
        self.options["wxwidgets"].expat = "off"
        self.options["wxwidgets"].secretstore = False
        self.options["wxwidgets"].aui = False
        self.options["wxwidgets"].opengl = False
        self.options["wxwidgets"].propgrid = False
        self.options["wxwidgets"].ribbon = False
        self.options["wxwidgets"].richtext = False
        self.options["wxwidgets"].sockets = False
        self.options["wxwidgets"].stc = False
        self.options["wxwidgets"].richtext = False
        self.options["wxwidgets"].webview = False
        self.options["wxwidgets"].xml = False
        self.options["wxwidgets"].xrc = False
        self.options["wxwidgets"].fs_inet = False
        self.options["wxwidgets"].help = False
        self.options["wxwidgets"].protocol = False
        self.options["wxwidgets"].html_help = False
        self.options["wxwidgets"].custom_disables = "wxUSE_ARTPROVIDER_TANGO,wxUSE_SVG"

    def imports(self):
        tools.rmdir("cmake")
        tools.mkdir("cmake")
        [tools.rename(file, f"cmake/{file}") for file in glob.glob("*.cmake")]

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.install()

