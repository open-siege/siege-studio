# Siege Extender Tester

Test app for verifying Siege Extender functionality.

## Setup Instructions
The example requires the Conan package manager.

Python is required by Conan, and if it is installed on your system you can do something the effect of:

``pip install conan``

For this project, you also need to add the Bincrafters Conan remote, with this command:

``conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan``

As a setup command, run ```conan install cmake/3.17.3@/ -g virtualenv```

Then run ```activate``` or ```./activate```

Once that is done, you can run the following command inside of the project directory:

```conan install .``` or ```conan install . --build=missing```

To build the project from the command line, use:

``conan build .``

Finally use ``build/bin/hello-wx-widgets`` to run the program.

You can also use your favourite IDE, like Visual Studio or CLion, after doing the ``conan install .`` step.


## Example Overview
The **conanfile.py** defines all of the dependencies and copies the source files to the _packages_ directory.

The **CMakeLists.txt** is the CMake project file for the example. It defines the include directories and build steps for the project.

The _src/main.cpp_ is a program which has all of the code for the UI.

