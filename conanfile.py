from conans import ConanFile, CMake, tools
import glob
import os.path
from PIL import Image


class LocalConanFile(ConanFile):
    name = "3space-studio"
    version = "0.5.1"
    url = "https://github.com/3space-studio/3space-studio"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    system_requires = "opengl/system"
    build_requires = "cmake/3.17.3"
    settings = "os", "compiler", "build_type", "arch"
    requires = "toml11/3.4.0", "nlohmann_json/3.9.0", "boost/1.76.0", "imgui-sfml/2.1@bincrafters/stable", "wxwidgets/3.1.3@bincrafters/stable", "glm/0.9.9.8", "span-lite/0.9.0", "taocpp-pegtl/3.1.0", "catch2/2.13.4"
    generators = "cmake_find_package", "virtualenv"
    exports_sources = "CMakeLists.txt", "LICENSE", "README.md", "game-support.md", "src/*"

    def configure(self):
        self.options["boost"].shared = False
        self.options["boost"].header_only = True
        self.options["boost"].bzip2 = False
        self.options["boost"].zlib = False
        self.options["boost"].numa = False

    def _configure_cmake(self):
        cmake = CMake(self)

        cmake.configure(source_folder=os.path.abspath("."), build_folder=os.path.abspath("build"))
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()
        cmake.test()

    def package(self):
        git = tools.Git(folder="wiki")
        git.clone("https://github.com/StarsiegePlayers/3space-studio.wiki.git", shallow=True)
        tools.rmdir("wiki/.git")
        cmake = self._configure_cmake()
        self.copy(pattern="LICENSE", src=self.source_folder, dst="licenses")
        self.copy(pattern="README.md", src=self.source_folder, dst="res")
        self.copy(pattern="wiki/*.md", src=self.source_folder, dst="res")
        self.copy(pattern="game-support.md", src=self.source_folder, dst="res")
        tools.rmdir("wiki")
        cmake.install()

    def package_info(self):
        self.cpp_info.libs.append("3space")

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


