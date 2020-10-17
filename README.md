# 3Space Studio

![](https://openclipart.org/image/400px/svg_to_png/97921/rubik-3D-colored.png)

### Summary

Classic Games, Modern Technology.

3Space Studio exists to breathe new life into games made by Dynamix, which use the 3Space engine and its descendants, while also reverse engineering their formats for fun, modding and preservation. 

For a list of current development tasks, check out this ClickUp board here: https://share.clickup.com/b/h/5-15151441-2/aeb3a2cc99994ef

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

### Game Support

Because the 3Space engine has a long history, and has morphed into engines with new names, here is a matrix of the games, most of which were made by Dynamix, (focusing specifically on DOS or Windows) which are intended to be supported or are supported:

| Game                                                         | Engine                              | Release Date   | Formats: | Model                                                        | Texture                      | Archive            | Other                                      |
| ------------------------------------------------------------ | ----------------------------------- | -------------- | -------- | ------------------------------------------------------------ | ---------------------------- | ------------------ | ------------------------------------------ |
| Arcticfox                                                    | 3Space 1                            | 1987 (for DOS) |          | ?                                                            | ?                            | ?                  |                                            |
| A-10 Tank Killer                                             | 3Space 1                            | 1989 (for DOS) |          |                                                              | ?                            | ?                  |                                            |
| Abrams Battle Tank                                           | 3Space 1                            | 1989 (for DOS) |          | ?                                                            | ?                            | ?                  |                                            |
| David Wolf Secret Agent                                      | 3Space 1                            | 1989           |          | ?                                                            | ?                            | ?                  |                                            |
| DeathTrack                                                   | 3Space 1                            | 1989           |          | ?                                                            | ?                            | ?                  |                                            |
| Die Hard                                                     | 3Space 1                            | 1989           |          | ?                                                            | ?                            | ?                  |                                            |
| MechWarrior                                                  | 3Space 1                            | 1989 (for DOS) |          | ?                                                            | ?                            | ?                  |                                            |
| F-14 Tomcat                                                  | 3Space 1                            | 1990 (for DOS) |          | ?                                                            | ?                            | ?                  |                                            |
| Red Baron                                                    | 3Space 1.5                          | 1990 (for DOS) |          | ?                                                            | ?                            | RMF❌               |                                            |
| Stellar 7 (re-release)                                       | 3Space 1.5                          | 1990 (for DOS) |          | ?                                                            | ?                            | ?                  |                                            |
| A-10 Tank Killer 1.5                                         | 3Space 1.5                          | 1991           |          | ?                                                            | ?                            | RMF❌               |                                            |
| Nova 9: The Return of Gir Draxon                             | 3Space 1.5                          | 1991 (for DOS) |          | ?                                                            | ?                            | RMF❌               |                                            |
| Aces of the Pacific                                          | 3Space 1.5                          | 1992           |          | ?                                                            | ?                            | DYN❌               |                                            |
| Aces Over Europe                                             | 3Space 1.5                          | 1993           |          | ?                                                            | ?                            | DYN❌               |                                            |
| Aces of the Deep                                             | 3Space 2.0                          | 1994           |          | DTS❌                                                         | ?                            | * DYN❌<br />* VOL❌ |                                            |
| Metaltech: Battledrome                                       | 3Space 2.0                          | 1994           |          | * DCS?❌<br />* DTS❌                                          | ?                            | ?                  |                                            |
| Metaltech: Earthsiege                                        | 3Space 2.0                          | 1994           |          | DTS❌                                                         | ?                            | VOL❌               |                                            |
| Command: Aces of the Deep                                    | 3Space 2.0                          | 1995           |          | DTS❌                                                         | ?                            | * DYN❌<br />* VOL❌ |                                            |
| Silent Thunder: A-10 Tank Killer 2                           | 3Space 2.0                          | 1996           |          | DTS❌                                                         | BMP❌                         | VOL❌               |                                            |
| Red Baron 2                                                  | 3Space 2.0                          | 1997           |          | DTS❌                                                         | BMP❌                         | VOL❌               |                                            |
| Pro Pilot '98                                                | 3Space 2.0                          | 1997           |          | DTS❌                                                         | BMP❌                         | VOL❌               |                                            |
| Front Page Sports: Ski Racing                                | 3Space 3.0 aka Darkstar             | 1997           |          | DTS✅                                                         | BMP❌                         | VOL❌               |                                            |
| Red Baron 3D                                                 | 3Space 2.0                          | 1998           |          | DTS❌                                                         | BMP❌                         | VOL❌               |                                            |
| Pro Pilot '99                                                | 3Space 2.0                          | 1998           |          | DTS❌                                                         | BMP❌                         | VOL❌               |                                            |
| Driver's Education '98                                       | 3Space 3.0 aka Darkstar (partially) | 1998           |          | DTS❌(has special version of Darkstar DTS which is not yet supported) | BMP❌                         | VOL❌               |                                            |
| Starsiege                                                    | 3Space 3.0 aka Darkstar             | 1999           |          | DTS✅                                                         | * BMP❌<br />* PBA❌           | VOL❌               | * VEH❌<br />* MIS❌<br />* GUI❌<br />* DLG❌ |
| Starsiege: Tribes                                            | 3Space 3.0 aka Darkstar             | 1999           |          | DTS✅                                                         | * BMP❌<br />* PBA❌           | VOL❌               |                                            |
| Driver's Education '99                                       | 3Space 3.0 aka Darkstar (partially) | 1998           |          | DTS❌(has special version of Darkstar DTS which is not yet supported) | BMP❌                         | VOL❌               |                                            |
| Field & Stream: Trophy Bass 3D                               | 3Space 3.0 aka Darkstar (partially) | 1999           |          | DTS✅                                                         | BMP❌                         | VOL❌<br />         |                                            |
| Tribes 2                                                     | Torque                              | 2001           |          | DTS❌                                                         | ?                            | VL2 (ZIP)❌         | DSO❌                                       |
| Notable post-Dynamix games:                                  |                                     |                |          |                                                              |                              |                    |                                            |
| Marble Blast Gold                                            | Torque                              | 2002           |          | DTS❌                                                         | ?                            | ?                  | DSO❌                                       |
| Marble Blast Ultra                                           | Torque                              | 2006           |          | DTS❌                                                         | ?                            | ?                  | DSO❌                                       |
| Dark Horizons - Lore                                         | Torque                              | 2006           |          | DTS❌                                                         | ?                            | ?                  | DSO❌                                       |
| Legions: Overdrive (re-release)                              | Torque                              | 2010           |          | DTS❌                                                         | ?                            | ?                  | DSO❌                                       |
| Blockland (Steam)                                            | Torque                              | 2013           |          | DTS❌                                                         | ?                            | ?                  | DSO❌                                       |
| BeamNG.drive                                                 | Torque                              | 2015           |          | DTS❌                                                         | ?                            | ?                  | DSO❌                                       |
| Wacky Wheels HD                                              | Torque                              | 2016           |          | DTS❌                                                         | ?                            | ?                  | DSO❌                                       |
| More Torque games: https://github.com/John3/awesome-torque3d#games | Torque                              | -              |          | DTS❌                                                         | * PNG<br />* DDS<br />* More | VL2 (ZIP)❌         | DSO❌                                       |

Supported format matrix:

| Format                          | Reading/Unpacking                                            | Writing/Packing                                              | Rendering/Viewing | Editing | Converting                                                   | Affects Games                                                |
| ------------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ | ----------------- | ------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| 3Space 3.0 DTS aka Darkstar DTS | ✅                                                            | ✅                                                            | ✅                 | ❌       | To: <br />* OBJ✅<br />* JSON✅<br />* Torque DTS❌<br />From:<br />* OBJ❌<br />* JSON✅<br />* Torque DTS❌ | * Front Page Sports: Ski Racing✅<br />* Driver's Education '98❌<br />* Starsiege✅<br />* Starsiege: Tribes✅<br />* Driver's Education '99❌<br />* Field & Stream: Trophy Bass 3D✅ |
| 3Space 3.0 VOL aka Darkstar VOL | Only via https://github.com/matthew-rindel/darkstar-vol-extractor<br />Rewrite and merging of codebase on backlog. | Only via https://github.com/matthew-rindel/darkstar-vol-extractor<br />Rewrite and merging of codebase on backlog. | ❌                 | ❌       | ❌                                                            | * Front Page Sports: Ski Racing❌<br />* Driver's Education '98❌<br />* Starsiege❌<br />* Starsiege: Tribes❌<br />* Driver's Education '99❌<br />* Field & Stream: Trophy Bass 3D❌ |
| 3Space 2.0 RMF                  | Only via https://github.com/matthew-rindel/3space-vol-extractor<br />Rewrite and merging of codebase on backlog. | ❌                                                            | ❌                 | ❌       | ❌                                                            | * Red Baron❌<br />* A-10 Tank-Killer 1.5❌<br />* Nova 9❌     |
| 3Space 2.0 DYN                  | Only via https://github.com/matthew-rindel/3space-vol-extractor<br />Rewrite and merging of codebase on backlog. | ❌                                                            | ❌                 | ❌       | ❌                                                            | * Aces of the Pacific❌<br />* Aces over Europe❌<br />* Aces of the Deep❌ |
| 3Space 2.0 VOL                  | Only via https://github.com/matthew-rindel/3space-vol-extractor<br />Rewrite and merging of codebase on backlog. | ❌                                                            | ❌                 | ❌       | ❌                                                            | * Metaltech: Earthsiege❌<br />* Metaltech: Battledrome❌<br />* Earthsiege 2❌<br />* Aces of the Deep❌ |
| Starsiege VEH                   | Only via https://github.com/matthew-rindel/starsiege-veh-editor<br />Rewrite and merging of codebase on backlog | ❌                                                            | ❌                 | ❌       | ❌                                                            | Starsiege❌                                                   |
| Starsiege MIS                   | Only via https://github.com/matthew-rindel/darkstar-mis-extractor<br />Rewrite and merging of codebase on backlog | ❌                                                            | ❌                 | ❌       | ❌                                                            | Starsiege❌                                                   |



### Format Background

Despite each version of the engine having some of the same extensions for some of their formats, each iteration of the engine has completely different structures and binary layouts for said formats.

In other words, 3Space 2.0 DTS files are fundamentally different to 3Space 3.0 DTS files, which are in turn completely different to Torque DTS files.

They do share a similar high level structure and some features, but the overall format changes over time and even between games there is a big difference between the format used.

For example, while Earthsiege and Red Baron 2 might share a 3Space 2.0 core (of sorts), the DTS files themselves have different version tags for each entity and need different code to handle them. Depending on how different they are, they may need completely separate implementations for parsing and viewing.

### Setup and Build Instructions

If you don't already have Conan on your system, find instructions here: https://conan.io/downloads.html

As a setup command, without any pre-built packages, run ```conan install cmake/3.17.3@/ -g virtualenv```

Then run ```activate``` or ```./activate```

To install project dependencies, use:

```conan install .``` or ```conan install . --build=missing``` if you system is missing precompiled packages for the dependencies.

For debug builds use:
```conan install . -s build_type=Debug``` or ```conan install . -s build_type=Debug --build=missing```

To build the project, use:

```conan build .```

Generated files will go into the **build/bin** folder.

### Usage Instructions

#### 3space-studio

TODO: write a decent description of how to use the 3Space Studio GUI.

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
