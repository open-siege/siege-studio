# Darkstar VOL Extractor

** Work will resume in May, with the other game extractors ** 

This is a Python script designed to extract VOL files by the Darkstar Engine, used in Starsige and Starsiege: Tribes.

Currently, the script can extract all files out of a VOL without any decompression.

The primary focus has been to obtain DTS files without having to use vtList.exe and extract.exe.

Eventually, decompression will be supported, especially to decompress the cs script files for the game.

Currently it just extracts all files into a folder with the same name as the vol file. This will be expanded up over time.

This project is related to https://github.com/matthew-rindel/darkstar-dts-converter


Current Usage
  `python extractVol.py someFile.vol [someFile2.vol] [someFile3.vol]`

Makes a folder per vol file and extracts the contents of each into them.

The format was reverse engineered initially by using already extracted files to determine embedded file sizes and various offsets. Later revisions (especially to get how the compression is determined) came from reading the original source code (https://github.com/AltimorTASDK/TribesRebirth).
