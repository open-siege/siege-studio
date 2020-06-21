# Darkstar Hook Progress & Roadmap

## Introduction
Darkstar Hook intends to extend the functionality of games using the Darkstar engine, to be able to enhance the quality of life for those games and expand the mod capabilities of them.

The heart of the project is a C++ Builder dll module which interfaces with a game in-process the uses new code to extend existing code based on the overall engine structure of the game.

This documentation is considered to be work in progress and should not be considered definitive or exhaustive.

## Engine Overview
### Simplified Structure
What makes the hook possible is the structure of the Darkstar engine, which has a powerful scripting engine used for configuration and game logic.

Internally, the scripting engine is backed by native code and further new code can be added dynamically with scripts.

The language is referred to as CScript with the file extension cs, not to be confused with cs files used for C# code.

The following pseudo code represents a simplistic structure of the internal interpreter (known as the console):

```
interface ConsoleCallback
    string execute(int32 callbackId, string[] arguments)

interface Console
    void addCommand(string name, int32 callbackId, ConsoleCallback callback)
    void removeCommand(string name)
    string execute(string name, string[] arguments)
```

Native code can then extend the console by conforming to the required interface, and the name of a command callback will be exactly the name of a function available for use from a CScript file.

Scripting functions like **schedule**, **echo**, **eval** and many others are internally known as commands, and have associated callbacks for each.

One callback instance can support many commands, which was likely done for performance reasons, or for certain commands to share the same state.

An extension of this idea is a **plugin**.

```
interface GamePlugin extends ConsoleCallback
    void init()
    void startFrame()
    void endFrame()
```

Plugins are used to extend the game as well as the console, and will be notified every time of a render cycle via startFrame and endFrame.

Starsiege, for example, contains 30 plugins which all extend the interpreter and enable script code to: configure vehicles, use IRC, edit levels, etc.


### Relevant Games
The above engine structure applies to the following games:
* Front Page Sports: Ski Racing
* Starsiege
* Starsiege: Tribes

Each game was compiled with Borland C++. A possible exception might be GarageGame's version of Tribes, but this needs to be verified.

## Hook Overview
### General Structure
The core of the hook is **darkstar.dll** which contains all of the hook logic. It is implemented with C++ Builder 10.3, using C++ 17 language and library features.

In addition, Python is integrated into the hook, using _pybind11_, to allow for extending the game without having to compile new dlls, to make use of other related Darkstar projects, and (eventually) become a first class language for implementing mods in each of the target games.

Another dependency ls _nlohmann/json_ which adds support for JSON parsing.

In order to hook specific games, thin proxy dlls are created which use _module forward exports_ to integrate with **darkstar.dll**.

Currently, the first proxy dll is **mem.dll**, however proxies will be made for **smackw32.dll**, **rBdgfx.dll** or other dlls specific to games which do not link with mem.dll, as well as to avoid collisions if an alternative mem.dll is used.
#### Further Material
* https://pybind11.readthedocs.io/en/stable/
* https://github.com/nlohmann/json

### Development Tools
C++ Builder is the primary development tool for the project. However Visual C++ is still needed to create the proxy dlls as the C++ Builder linker does not support module forwarding.

As described in README.md, Visual C++ cannot be used because it does not support Borland __fastcall, which is only available in C++ Builder (but also in Delphi and Free Pascal, because __fastcall is also known as the Delphi register calling convention). 

Visual Studio can still be used to edit the code, and eventually there will be a build system that runs both the C++ Builder and Visual C++ build processes to reduce manual creation of certain files.

A 32-bit installation of Python 3 is required for the development headers and the Python embedded package is needed when testing the hook with a game.

#### Further reading
* https://en.wikipedia.org/wiki/X86_calling_conventions#Borland_register

### Implementation
#### Module Forward Exports
To create a custom **mem.dll**, Visual C++ is used. 

There isn't any code which goes into the dll, because all of the work is handled by the linker.

Forwarding of exports is a native feature of the Windows dll loader, and the Visual C++ linker enables this interaction.

The exports are defined in _mem.def_, which simply uses darkstar.dll as the true implementation.

This separation is required for easy creation of alternative proxy dlls, especially for Ski Racing and Tribes 2009.

In addition, a future consideration is to have plugin dlls, and they will need to refer back to a constant module, which would be darkstar.dll in this case.

##### Tools
* C++ Builder impdef: http://docwiki.embarcadero.com/RADStudio/Rio/en/IMPDEF.EXE,_the_Module_Definition_Manager
* C++ Builder implib: http://docs.embarcadero.com/products/rad_studio/radstudio2007/RS2007_helpupdates/HUpdate4/EN/html/devwin32/implib_xml.html
* Visual C++ lib: https://docs.microsoft.com/en-us/cpp/build/reference/lib-reference?view=vs-2019
##### Further Material
* https://devblogs.microsoft.com/oldnewthing/20121116-00/?p=6073
* https://devblogs.microsoft.com/oldnewthing/20060719-24/?p=30473
* https://kevinalmansa.github.io/application%20security/DLL-Proxying/
* https://www.synack.com/blog/dll-hijacking-in-2014/

#### Targeting a Game Address Space
In general, most executables do not export functions to be loaded by other modules. However, even though the functions are not exported, they still have to have an address in memory.

Because most Windows applications, and the games in question, have code in the .text area of an executable, and that it all starts from 0x000000 in terms of addressing, it is possible to use a disassembler or decompiler to identify known code and then the addresses for a particular executable.

Since each game uses the same engine and architecture, the functions are the same, and generally have the same calling convention for the same compiler, but exist in different locations in the executable.

To manage this, all the function addresses are kept in a json file on a per executable basis, and if all the required functions have their addresses added, any game can be supported.

The hook uses the function address to create function pointers into the memory of the executable, because it is in the same memory address space.

Currently only Starsiege 1.003 is supported, but support will be extended to more games when features are added and considered to be stable.

The json file in question is **functions.json** but this name could change later on, or the structure could change to accommodate new features. 
##### Further Material
* https://www.youtube.com/watch?v=1S0aBV-Waeo

#### TODO - finish remaining sections 
#### Extending the Game
TODO discuss the different ways of adding features to the game.

#### Python Support
TODO discuss what is possible with Python in the context of the game.

### Further Future Work
#### Typescript Support
TODO discuss the differences in approach between this and Python, the motivations and so on
##### Transpilation to CScript
TODO discuss syntax similarities and differences, while referring to overall improvements and the CScript backend which will be worked on, on GitHub.
##### Extending CScript
TODO discuss how the game interpreter will be extended with data types and functions needed to make the runtime compliant with JavaScript/TypeScript
#### Reshade Integration
TODO discuss how Python or TypeScript code can dynamically control shaders based on events in the game.
Also to discuss would be changes needed to the Reshade32.dll API to support dynamic control from darkstar.dll

#### Darkstar.dll API
TODO discuss how data types and functions can be exported from the dll to enable plugins for/in other languages to be created, to make it easier for developers from different programming backgrounds to contribute.
