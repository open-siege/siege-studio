# Darkstar DTS Converter

### Summary
A set of C++ tools for converting to and from the DTS and DML formats used in the following games:
* Starsiege
* Starsiege Tribes
* Front Page Sports Ski Racing
* Trophy Bass 3D
* Driver's Education 98/99

Current converting DTS and DML files to JSON is supported, and soon OBJ, which was in the original Python version of this project.

This is the third generation of the DTS format, so versions will be described as 3.x in the documentation, but as simply x in the file format itself.

Darkstar DTS versions 3.2, 3.3, 3.5, 3.6, 3.7 and 3.8 can be processed, which covers files from each of the games above.

### Setup and Build Instructions

If you don't already have Conan on your system, find instructions here: https://conan.io/downloads.html

To install project dependencies, use:

```conan install .``` or ```conan install . --missing``` if you system is missing precompiled packages for the dependencies.

To build the project, use:

```conan build .```

Generated files will go into the **build/bin** folder.

### Usage Instructions

#### dts-to-json
With dts-to-json, you can convert either individual or multiple DTS or DML files to JSON.

You can do ```dts-to-json *``` to convert all files in a folder.

Or ```dts-to-json some.dts``` to convert an individual file.

Or you can simply drag one or more files and drop it onto the executable, and it will convert all items that it can.

Inspired by a DTS converter for Earthsiege 2 (https://github.com/booto/convert_dts)

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

### Known Issues
* A small amount of files from Tribes have _null_ instead of a number in the JSON output. 
* DTS files from Driver's Education have a variant of DTS 3.5, which is not fully supported currently.