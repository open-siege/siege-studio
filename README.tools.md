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


### Usage Instructions

TODO: add information for all of the other working build targets

#### siege-studio

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
