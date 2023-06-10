# Siege Studio

![](https://openclipart.org/image/400px/svg_to_png/97921/rubik-3D-colored.png)

### Summary

Classic Games, Modern Technology.

Siege Studio exists to breathe new life into games made by Dynamix, which use the 3Space engine and its descendants, while also reverse engineering their formats for fun, modding and preservation.

For a list of current development tasks, check out this ClickUp board here: https://share.clickup.com/b/h/5-15151441-2/aeb3a2cc99994ef

If you are reading this on your computer, checkout the latest README here: https://github.com/open-siege/open-siege

A first goal for each game is to do the following:

* Support the viewing of 3D assets from various games (specifically DTS - Dynamix Three Space files).
* Support the extraction of archive files for specified games (typically VOL - a game volume archive file).
* Support viewing of texture data for the 3D assets and the games in question.

Secondary goals include:

* The ability to save new 3D assets using the supported format for that game.
* The ability to do some basic manipulation of the 3D assets.
* The ability to convert the 3D asset to a more common format for use in other 3D software.

Tertiary goals would be:

* Game specific file format support, for example VEH for Starsiege, which would allow for editing of vehicle load-outs and saving them back to the file system.
* Support of other game formats which serve a similar function (like MDL or PAK from Quake).
* Become a tool of choice for modding games using 3Space or Torque technology and potentially other game engines.

### Game & Format Support

For a full list of games which are intended to be supported, see: [game-support.md](docs/game-support.md)

### Format Background

Despite each version of the engine having some of the same extensions for some of their formats, each iteration of the engine has completely different structures and binary layouts for said formats.

In other words, 3Space 2.0 DTS files are fundamentally different to 3Space 3.0 DTS files, which are in turn completely different to Torque DTS files.

They do share a similar high level structure and some features, but the overall format changes over time and even between games there is a big difference between the format used.

For example, while Earthsiege and Red Baron 2 might share a 3Space 2.0 core (of sorts), the DTS files themselves have different version tags for each entity and need different code to handle them. Depending on how different they are, they may need completely separate implementations for parsing and viewing.

### Setup and Build Instructions

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

### TGUI Theme Generation

Once the Besieged Theme and the related script is release-ready, it will be integrated into the build pipeline via Conan.

**besieged-theme.afdesign** is the source of truth, and **besieged-theme.svg** exists only as a convenience for build automation (there are plans to do cloud builds eventually).

With that being said, contributions to the theme may become tricky because of the arrangement, and the advice is either for a person to use Affinity Designer (if they have it), for changes to the svg to be backported during a merge request, by someone with an Affinity Designer license.

Hopefully it doesn't come down to that, because the release-ready version should be mostly final.

### Usage Instructions

TODO: add information for all of the other working build targets

#### 3space-studio

TODO: write a decent description of how to use the Siege Studio GUI.

#### dts-to-json
With dts-to-json, you can convert either individual or multiple DTS or DML files to JSON.

You can do ```dts-to-json *``` to convert all files in a folder.

Or ```dts-to-json some.dts``` to convert an individual file.

Or you can simply drag one or more files and drop it onto the executable, and it will convert all items that it can.

The result will be a plain text JSON file which should be editable in any text editor, and which has the same name as the DTS/DML file except with **.json** added onto it.

This file can then be fed back into **json-to-dts** to create a new DTS/DML file.

#### json-to-dts
With json-to-dts, you can convert either individual or multiple JSON files to DTS or DML.

You can do ```json-to-dts *``` to convert all files in a folder.

Or ```json-to-dts some.dts``` to convert an individual file.

Or you can simply drag one or more files and drop it onto the executable, and it will convert all items that it can.

The result will be a new DTS or DML file that can be put back into the game of choice.

If an existing DTS or DML file is present, then it will be renamed with the extension **.old** appended to it.

Any existing **.old** files will not be overwritten for backup purposes of the original file being modified.

### License Information

See [LICENSE](LICENSE) for license information about the code (which is under an MIT license).

All other contents of the repository, which are not present in code form, such as UI designs, UI themes, images or logos fall under a Creative Commons - Attribution-ShareAlike 4.0 International (CC BY-SA 4.0) license.

This includes **besieged-theme.svg** and **besieged-theme.afdesign**.

<a rel="license" href="http://creativecommons.org/licenses/by-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-sa/4.0/88x31.png" /></a>

The Besieged Theme, by Matthew Rindel, is work licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-sa/4.0/">Creative Commons Attribution-ShareAlike 4.0 International License</a>.
