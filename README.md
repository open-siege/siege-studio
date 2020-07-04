# Darkstar Hook
A C++ Builder dll which allows for extension of Darkstar engine games and embeds Python to make that process fully dynamic.

### Dependencies
#### Tools
* C++ Builder 10.3.3 or newer.
* A disassembler or decompiler for the relevant game. I use Binary Ninja 
#### Libraries
* Python 3.8.3 or newer (32-bit version only)
* Pybind11 2.5.0
* nlohmann_json 3.8.0

### Project Setup
First, make sure you have installed Python 3 on your system. This is easy as running:

``choco install python3 --x86``

The default installation directory is *C:\Python38-32*, and there is an _include_ folder inside there with all of the Python C headers, which are required by this project.
 
If the directory is somewhere else, update the pythonIncludeDir in local-config.json to point to the include folder.

Conan is required to install the project dependencies (except for Python - at the moment)

If you don't already have it, just run:

``pip install conan``

Once everything is installed, run the following in the main checkout directory of the project.

``conan install . --profile ./local-profile.ini``

All installed packages are copied into the _packages_ folder. This includes the Python include files from the local Python installation.

### Setup with Game
TODO

### Notes
* A lib file for Python must be created from the python38.def file in the libs folder
    * To do this, impdef is required. TODO add exact commands
* Small changes to the Pybind11 headers are needed for things to compile
    * TODO add the two lines which need to be commented out.
* Everything is work in progress, including the documentation.
    * More detailed explanations of how this module works and the exposed API will be added over time.
* All of the exe details for each game get stored in functions.json. Right now, only Starsiege 1.0003r has been added.
* Why C++ Builder? Why not Visual C++ or MinGW?
    * Most Darkstar games were compiled with the predecessor to C++ Builder and use a calling convention called __fastcall
    * The meaning of __fastcall in C++ Builder is different to that of Visual C++, thus meaning raw assembly code would be needed to interface with the game code if Visual C++ is to be used
    * So far, object structures (like vtables) are compatible between C++ Builder and Borland C++, which makes it possible extend the game in new ways based on disassembled information  
* What can be done with this?
    * The built-in game interpreter can be extended with new functions either from C++ or Python
    * Existing built-in game functions can also be removed, replaced or decorated
    * Examples of ideas:
        * Updating the screenshot function of the game to convert BMP files to PNG
        * Adding support for remote controlling of dedicated servers through a browser
        * Discord integration with the chat engine in the game
        * Much more

### Related Projects
https://github.com/matthew-rindel/darkstar-projects