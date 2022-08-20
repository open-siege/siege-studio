from conans import ConanFile, CMake, tools
import os.path

class HelloWxWidgetsConanFile(ConanFile):
    build_requires = "cmake/3.22.0"
    requires = "wxwidgets/3.1.5@bincrafters/stable"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package"

    def configure(self):
        self.options["wxwidgets"].jpeg = "off"
        self.options["wxwidgets"].tiff = "off"
        self.options["wxwidgets"].expat = "off"
        self.options["wxwidgets"].aui = True
        self.options["wxwidgets"].opengl = True
        self.options["wxwidgets"].mediactrl = False
        self.options["wxwidgets"].secretstore = False
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

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.install()

