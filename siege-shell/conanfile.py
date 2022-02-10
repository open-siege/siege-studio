from conans import ConanFile, CMake, tools
from conans.tools import os_info, SystemPackageTool
import glob
import os.path
import sys

urls = {
    "x86": "https://download.imagemagick.org/ImageMagick/download/binaries/ImageMagick-7.1.0-23-Q16-HDRI-x86-dll.exe",
    "x86_64": "https://download.imagemagick.org/ImageMagick/download/binaries/ImageMagick-7.1.0-23-Q16-HDRI-x64-dll.exe"
}

class LocalConanFile(ConanFile):
    name = "siege-shell"
    version = "0.5.1"
    url = "https://github.com/3space-studio/3space-studio"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    build_requires = "cmake/3.22.0"
    settings = "os", "compiler", "build_type", "arch"
    requires = "3space/0.5.1", "wxwidgets/3.1.5@bincrafters/stable"
    generators = "cmake_find_package"

    def system_requirements(self):
        arch = str(self.settings.arch)
        if os_info.is_windows and arch in urls:
            url = urls[arch]
            if not os.path.exists(f"installer-{arch}.exe"):
                tools.download(url, f"installer-{arch}.exe")

            if not os.path.exists(f"C:/ImageMagick-{arch}"):
                self.run(f"installer-{arch}.exe /DIR=\"C:\\ImageMagick-{arch}\" /VERYSILENT /NORESTART /MERGETASKS=install_devel")



    def build(self):
        cmake = CMake(self)
        cmake.definitions["PYTHON_EXECUTABLE"] = sys.executable
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.definitions["PYTHON_EXECUTABLE"] = sys.executable
        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        cmake.install()

    def imports(self):
        tools.rmdir("cmake")
        tools.mkdir("cmake")
        [tools.rename(file, f"cmake/{file}") for file in glob.glob("*.cmake")]


