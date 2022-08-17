from conans import ConanFile, CMake, tools
import glob
import os.path


class LocalConanFile(ConanFile):
    name = "starsiege-launcher"
    version = "0.0.1"
    url = "https://github.com/3space-studio/3space-studio"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    system_requires = "opengl/system"
    build_requires = "cmake/3.22.0"
    settings = "os", "compiler", "build_type", "arch"
    requires = "wxwidgets/3.1.5@bincrafters/stable", "catch2/2.13.8", "cpr/1.7.2"
    generators = "cmake_find_package", "virtualenv"
    exports_sources = "CMakeLists.txt", "LICENSE", "README.md", "game-support.md", "src/*"

    def configure(self):
        self.options["wxwidgets"].mediactrl = False
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

    def _configure_cmake(self):
        self.source_folder = os.path.abspath(".")
        self.build_folder = os.path.abspath("build")
        cmake = CMake(self)

        cmake.configure()
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()
        cmake.test()

    def package(self):
        cmake = self._configure_cmake()
        self.copy(pattern="LICENSE", src=self.source_folder, dst="licenses")
        self.copy(pattern="README.md", src=self.source_folder, dst="res")
        cmake.install()

    def imports(self):
        tools.rmdir("cmake")
        tools.mkdir("cmake")
        [tools.rename(file, f"cmake/{file}") for file in glob.glob("*.cmake")]


