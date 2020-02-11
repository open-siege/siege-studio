# Darkstar VOL Extractor

This is a Python 3 script designed to extract VOL files by the Darkstar Engine, used in the following games:
* Front Page Sports: Ski Racing
* Driver's Education '98 & Driver's Education '99
* Starsiege: Tribes
* Starsiege
* Field & Stream: Trophy Bass 3D

Currently, the script can extract all files out of a VOL without any decompression, and fallback to extract.exe for anything with compression.

The original focus was to obtain DTS files without having to use vtList.exe and extract.exe.

Eventually, decompression will be supported, especially to decompress the cs script files for the game.

Currently it just extracts all files into a folder with the same name as the vol file. This will be expanded up over time.

A script to generate new vol files also exists.

### Extracting files

Current Usage to extract files:

  `python unvol.py someFile.vol [someFile2.vol] [someFile3.vol]`

Globs are supported too:

  `python unvol.py *.vol [**/*.vol] [**/**/*.vol]`
   
The same works for unvol.exe

A folder is make per vol file and the contents extracted of each into them.

A JSON file is also generated with the original order of the files for repacking.

### Packing Files 

To make new volume files, just use nuvol.py.

Usage is:

`python nuvol.py someFolder1 [someFolder2] [someFolder3]`

The script will take all the files inside of someFolder1 and make someFolder1.vol

If a **vol.json** file is present, then only the files listed in it will be used.

To generate multiple vol files from folders with vol.json files, the following usage applies:

`python nuvol.py **/*.vol.json [**/**/*.vol.json] [**/**/**/*.vol.json]`

Which will convert multiple folders to vol files. For just one file, try:

`python nuvol.py someFolder1/someFolder1.vol.json`

### Notes

The format was reverse engineered initially by using already extracted files to determine embedded file sizes and various offsets. Later revisions (especially to get how the compression is determined) came from reading the original source code (https://github.com/AltimorTASDK/TribesRebirth).

This project is related to https://github.com/matthew-rindel/darkstar-dts-converter