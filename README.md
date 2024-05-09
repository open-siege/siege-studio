# Siege Studio

<img height="320" width="320" src="logo/logo.svg" alt="Open Siege logo" />

### Summary

Classic Games, Modern Technology.

Open Siege is a suite of tools to enable the compatibility and playability of classic games, primarily focusing on vehicle simulations and first person shooters from the DOS and early Windows era.

The eventual goal is to become an open-source reimplementation of the 3Space game engine, used by several games such as the Earthsiege/Starsiege series, the Red Baron/Great War Planes series and several others.

Until then, the focus is currently on the following goals:
* Installability: making games easier to install and integrating them with modern game launchers.
* Controllability: configuring modern controllers, both common and niche, with popular control schemes and fixing compatibility issues.
* Modability: being able to explore, preview and edit assets which belong to a game.

The project contains the following components:
* Siege Studio (in this repo)
  * Supports browsing of game files and previewing the contents of supported formats.
  * Allows for conversion to and from certain formats.
  * Supports extraction of game archive files.
* Siege Modules (also in this repo)
  * The core of the file format support in the project. 
  * It contains all the logic to parse game archive and asset files as well as configuration files.
* Siege Launcher (a commercial product soon to be available on Steam)
  * A configuration tool for detecting controllers and games and creating matching configurations for them.
  * It supports editing game configuration files to match the physical layout of the controller, and modern control schemes for the genre.
  * It takes care of the differences between DirectInput and XInput controllers and makes sure the game works with both.
  * It can configure multiple device setups such as joystick + throttle or joystick + joystick, plus a few others.
  * Can run in headless mode or with a UI with system tray support.
  * Allows for installation of a game from a phyiscal disc or compressed/disc archive.
  * Can work with multiple levels of archives (ISO file inside of a zip file).
  * Support for adding the game to a launcher such as GOG or Steam.
  * Support for adding a game to an existing entry in GOG or Steam, especially for sequels or related games (for example adding Fury3 to Terminal Velocity).


### Setup and Build Instructions

_Caution!_ The instructions are currently incorrect because of a move from Conan 1 to Conan 2 and thus are no longer correct.
Updates are coming soon.

New to C++, CMake or Conan? Checkout this set of examples with instructions to get started: https://github.com/matthew-rindel/hello-cpp-durban

If you don't already have Conan on your system, find instructions here: https://conan.io/downloads.html

To configure and build a debug build, use:
```conan install . -s build_type=Debug --build=missing```
```conan build .```

To configure and build a release build, use:
```conan install . --build=missing```
```conan build .```

For x86 builds, which triggers building of additional projects, use:
```conan install . -s arch=x86 --build=missing```
```conan build .```

Generated files will go into the **build/Release/bin** or **build/Debug/bin** folder.

### License Information

See [LICENSE](LICENSE) for license information about the code (which is under an MIT license).

All other contents of the repository, which are not present in code form, such as UI designs, UI themes, images or logos fall under a Creative Commons - Attribution-ShareAlike 4.0 International (CC BY-SA 4.0) license.

This includes **besieged-theme.svg** and **besieged-theme.afdesign**.

<a rel="license" href="http://creativecommons.org/licenses/by-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-sa/4.0/88x31.png" /></a>

The Besieged Theme, by Matthew Rindel, is work licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-sa/4.0/">Creative Commons Attribution-ShareAlike 4.0 International License</a>.
