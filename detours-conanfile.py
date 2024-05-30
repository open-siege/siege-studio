from conan import ConanFile
from conan.tools.files import copy

class SiegeLauncherConanFile(ConanFile):
    name = "open-siege"
    version = "0.6.3"
    url = "https://github.com/open-siege/open-siege"
    license = "MIT"
    author = "Matthew Rindel (matthew@thesiegehub.com)"
    build_requires = "cmake/3.26.4"
    requires = "detours/cci.20220630"
    settings = "os", "compiler", "build_type", "arch"

    def generate(self):
        info = self.dependencies["detours"].cpp_info
        copy(self, "*.h", info.includedirs[0], self.build_folder)
        copy(self, "*.lib", info.libdirs[0], self.build_folder, keep_path=False)
