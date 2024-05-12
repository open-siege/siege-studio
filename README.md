# Siege Studio

<img height="320" width="320" src="logo/logo.svg" alt="Open Siege logo" />

### Summary

Classic Games, Modern Technology.

Siege Studio is a tool for previewing, converting and (eventually) editing reverse engineered files for games such as Earthsiege, Starsiege and other related games from the 1990s and early 2000s.

The core modules of the project are meant to be used for a future open-source reimplementation of the 3Space game engine, used by several games such as the Earthsiege/Starsiege series, the Red Baron/Great War Planes series and several others.

The project contains the following components:
* Siege Studio (in this repo)
  * Supports browsing of game files and previewing the contents of supported formats.
  * Allows for conversion to and from supported formats.
  * Supports extraction of game archive files.
* Siege Modules (also in this repo)
  * The core of the file format support in the project. 
  * It contains all the logic to parse game archive and asset files as well as configuration files.
  * The modules are grouped as such:
      * Siege Platform: a connection of static libraries, especially to handle platform specific features and make each platform easier to use.
      * Siege Content: a static library used to read, write and parse file formats such as Phoenix Bitmap (pba), Dynamix 3Space Shape (dts) and many more.
      * Siege Resource: a static library used to read, write and parse compound file formats, such as VOL or ZIP, which can contain many files within.
      * Siege 2D Content, Siege 3D Content, Siege Audio Content, Siege Resource Content: dynamic libraries containing UI and other logic to be loaded by a host program dynamically.
  * Each dynamic module will eventually have their own C# wrapper with related packages published to NuGet.    
* Siege Launcher (a commercial product to be available on Steam later this year, but not in this repo)
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
If you don't already have Conan on your system, find instructions here: https://conan.io/downloads.html

To configure and build a debug build, use:
```conan build . -s build_type=Debug --build=missing```

To configure and build a release build, use:
```conan build . -s build_type=Release -s compiler.runtime=static --build=missing```

Generated files will go into the **build/Release/bin** or **build/Debug/bin** folder.

### License Information

See [LICENSE](LICENSE) for license information about the code (which is under an MIT license).

All other contents of the repository, which are not present in code form, such as UI designs, UI themes, images or logos fall under a Creative Commons - Attribution-ShareAlike 4.0 International (CC BY-SA 4.0) license.

This includes **besieged-theme.svg** and **besieged-theme.afdesign**.

<a rel="license" href="http://creativecommons.org/licenses/by-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-sa/4.0/88x31.png" /></a>

The Besieged Theme, by Matthew Rindel, is work licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-sa/4.0/">Creative Commons Attribution-ShareAlike 4.0 International License</a>.
