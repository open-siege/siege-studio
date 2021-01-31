from conans import ConanFile, CMake, tools
import glob
import os.path
from PIL import Image


class LocalConanFile(ConanFile):
    system_requires = "opengl/system"
    build_requires = "cmake/3.17.3"
    settings = "os", "compiler", "build_type", "arch"
    requires = "toml11/3.4.0", "nlohmann_json/3.9.0", "boost_endian/1.69.0@bincrafters/stable", "imgui-sfml/2.1@bincrafters/stable", "wxwidgets/3.1.3@bincrafters/stable", "glm/0.9.9.8", "span-lite/0.9.0", "catch2/2.13.4"
    generators = "cmake_find_package", "virtualenv"
    build_folder = "build"

    def build(self):
        self.build_folder = "build"
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.test()

    def imports(self):
        if not os.path.exists("src/3space-studio/logo.ico"):
            tools.download(f"https://openclipart.org/image/400px/svg_to_png/97921/rubik-3D-colored.png", "src/3space-studio/logo.png")
            filename = "src/3space-studio/logo.png"
            img = Image.open(filename)
            img.save("src/3space-studio/logo.ico")

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


