# Siege Studio

<img height="320" width="320" src="logo/logo.svg" alt="Open Siege logo" />

### Summary

Classic Games, Modern Technology.

Siege Studio is a tool for previewing, converting and (eventually) editing reverse engineered files for games such as Earthsiege, Starsiege and other related games from the 1990s and early 2000s.

In addition, several extension modules are available to fix compatibility issues with selected games, as well as configure controller support and assistance with networking.

The project contains the following components:
* Siege Studio (in this repo)
  * Supports browsing of game files and previewing the contents of supported formats.
  * Allows for conversion to and from supported formats.
  * Supports extraction of game archive files.
* Siege Modules (also in this repo)
  * Various static libraries which contain the main logic for parsing files and/or modifying them:
      * Siege Platform: a connection of static libraries, especially to handle platform specific features and make each platform easier to use.
      * Siege Content: a static library used to read, write and parse file formats such as Phoenix Bitmap (pba), Dynamix 3Space Shape (dts) and many more.
      * Siege Resource: a static library used to read, write and parse compound file formats, such as VOL or ZIP, which can contain many files within.
  * Presentation Modules, which are dynamic libraries describing the UI for interacting with various supported file types, to be loaded by a host program dynamically.
      * Archive Modules are Presentation Modules which also expose an API for interacting with archive files for use in other Presentation Modules.
      * Siege Presentation 2D, Siege Presentation 3D, Siege Presentation Audio, Siege Presentation Resource are some of the available presentation modules.
  * Extension Modules, which can contain compatibility fixes or custom logic to improve networking or controller support.
      * Current publicly available families of extensions are:
        * id Tech (for games built using id Tech or related engines).
        * 3Space (for games built using 3Space or Torque).
        * Other (for games or tools which aren't logically organised).
      * There are also two additonal dynamic libraries meant for supporting advanced features available to every game:
        * Input Filtration - which allows for keyboard and mouse input to be restricted to specific devices.
        * WinSock Over Zero Tier - provides a wsock32/ws2_32 compatible API for playing games over Zero Tier.
      * Orchestration of all of these modules is done by Siege Presentation Executable module.
  * Installation Modules, which contain logic for unpacking game data and installing them onto a system.
      * Current publicly available families of installation modules are:
        * Dynamix (for games made by Dyanmix which aren't available on digital storefronts).
        * Raven (for games made by Raven Software which aren't available on digital storefronts).
        * Other (for games which can't be organised logically yet).
* Siege Launcher (a commercial product, but not in this repo)
  * The commercial version of Siege Studio but with a greater emphasis on playing games versus viewing their data.
  * It has the same architecture as Siege Studio and reuses all the same modules, with the exception that more modules are offered in the commercial release.
    * Examples of private Extension Modules included:
      * Cry Engine (for some Cry Engine 1 and 2 titles).
      * isiMotor (for most isiMotor 1 and 2 titles).
      * Unreal (for most Unreal Engine 1 and 2 titles).
      * And more...
    * Examples of private Installation Modules included:
      * EA (for older Need for Speed and FIFA titles).
      * Microsoft (for the various Madness titles).
      * And more...


### Setup and Build Instructions
If you don't already have Conan on your system, find instructions here: https://conan.io/downloads.html

To configure and build a debug build, use:
```conan build . -s build_type=Debug -s compiler.runtime=static -s arch=x86 --build=missing```

To configure and build a release build, use:
```conan build . -s build_type=Release -s compiler.runtime=static -s arch=x86 --build=missing```

TODO add instructions for 64 bit builds and merging binaries into one package

Generated files will go into the **build/Release/bin** or **build/Debug/bin** folder.

### License Information

See [LICENSE](LICENSE) for license information about the code (which is under an MIT license).

All other contents of the repository, which are not present in code form, such as UI designs, UI themes, images or logos fall under a Creative Commons - Attribution-ShareAlike 4.0 International (CC BY-SA 4.0) license.

This includes **besieged-theme.svg** and **besieged-theme.afdesign**.

<a rel="license" href="http://creativecommons.org/licenses/by-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-sa/4.0/88x31.png" /></a>

The Besieged Theme, by Matthew Rindel, is work licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-sa/4.0/">Creative Commons Attribution-ShareAlike 4.0 International License</a>.
