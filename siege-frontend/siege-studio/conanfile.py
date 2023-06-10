from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
import glob
import os.path
import sys
from PIL import Image

class LocalConanFile(ConanFile):
    name = "siege-studio"
    version = "0.6.3"
    url = "https://github.com/open-siege/open-siege"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    build_requires = "cmake/3.26.4"
    settings = "os", "compiler", "build_type", "arch"
    requires = "3space/0.6.3", "libpng/1.6.37", "imgui/cci.20230105+1.89.2.docking", "sdl/2.26.1"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        if self.settings.os == "Linux":
            self.requires("tbb/2020.3")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure()
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


