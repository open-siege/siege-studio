# Open Siege

<img height="320" width="320" src="logo/logo.svg" alt="Open Siege logo" />

### Summary

Classic Games, Modern Technology.

Open Siege is an open-source reimplementation of the 3Space game engine, used by several games such as the Earthsiege/Starsiege series, the Red Baron/Great War Planes series and several others.

Open Siege exists to breathe new life into games made by Dynamix, which use the 3Space engine and its descendants, while also reverse engineering their formats for fun, modding and preservation.

For a list of features currently being worked on, see the release notes for the upcoming changes here: vhttps://github.com/open-siege/open-siege/wiki/Release-Notes

Primary goals of the project at this time are to reverse engineer all of the file formats of files used by each game, while also enabling each of the games to work correctly on modern platforms, with additional enhances provided by hooking the games as needed.

Some of the key areas of focus are:
* Support the viewing of 3D assets from various games (specifically DTS - Dynamix Three Space files).
* Support the extraction of archive files for specified games (typically VOL - a game volume archive file).
* Support viewing of texture data for the 3D assets and the games in question.

Secondary goals include:
* The ability to save new 3D assets using the supported format for that game.
* The ability to do some basic manipulation of the 3D assets.
* The ability to convert the 3D assets to a more common format for use in other 3D software.
* Game specific file format support, for example VEH for Starsiege, which would allow for editing of vehicle load-outs and saving them back to the file system.
* Support of other game formats which serve a similar function (like MDL or PAK from Quake).
* Become a tool of choice for modding games using 3Space or Torque technology and potentially other game engines.

Tertiary goals:
* Create a new game engine that can host all of the files and content from each game, including:
  * A scripting engine supporting scripts from Starsiege, Starsiege Tribes and Tribes 2.
  * Support for original game files with minimal to no modifications.
  * The abillity to play the games on more platforms beyond DOS and Windows.

### Game & Format Support

For a full list of games which are intended to be supported, see: [game-support.md](docs/game-support.md)

### Setup and Build Instructions

New to C++, CMake or Conan? Checkout this set of examples with instructions to get started: https://github.com/matthew-rindel/hello-cpp-durban

If you don't already have Conan on your system, find instructions here: https://conan.io/downloads.html

On first clone and setup, run:

```conan install initial-settings.py```

This will configure bincrafters and relevant settings to make the project easier to work with.

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
