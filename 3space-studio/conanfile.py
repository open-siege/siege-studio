from conans import ConanFile, CMake, tools
import glob
import os.path
from PIL import Image

class LocalConanFile(ConanFile):
    name = "3space-studio"
    version = "0.6.1"
    url = "https://github.com/open-siege/open-siege"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    build_requires = "cmake/3.22.0"
    settings = "os", "compiler", "build_type", "arch"
    requires = "3space/0.6.1", "wxwidgets/3.1.5@bincrafters/stable", "imgui-sfml/2.5@bincrafters/stable", "zlib/1.2.12"
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

    def imports(self):
        if not os.path.exists("src/logo.ico"):
            tools.download(f"https://openclipart.org/image/400px/svg_to_png/97921/rubik-3D-colored.png", "src/logo.png", verify=False)
            filename = "src/logo.png"
            img = Image.open(filename)
            img.save("src/logo.ico")

        registry_url = "https://www.khronos.org/registry"

        if not os.path.exists("packages/include/KHR/khrplatform.h"):
            tools.download(f"{registry_url}/EGL/api/KHR/khrplatform.h", "packages/include/KHR/khrplatform.h")
            tools.download(f"{registry_url}/OpenGL/api/GL/glext.h", "packages/include/GL/glext.h")
            tools.download(f"{registry_url}/OpenGL/api/GL/glxext.h", "packages/include/GL/glxext.h")
            tools.download(f"{registry_url}/OpenGL/api/GL/wgl.h", "packages/include/GL/wgl.h")
            tools.download(f"{registry_url}/OpenGL/api/GL/wglext.h", "packages/include/GL/wglext.h")
            tools.download(f"{registry_url}/OpenGL/api/GL/glcorearb.h", "packages/include/GL/glcorearb.h")

        tools.rmdir("cmake")
        tools.mkdir("cmake")
        [tools.rename(file, f"cmake/{file}") for file in glob.glob("*.cmake")]


