# 3Space VOL Extractor

This is a set of Python scripts designed to extract VOL files by the 3Space Engine, used in multiple Dynamix simulation games.

Currently, there are two scripts:
* extractVol.py
    * It extracts VOL files from the following games:
        * Metaltech: Earthsiege + Earthsiege Expansion
        * Metaltech: Battledrome
        * Earthsiege 2
        * Aces of the Deep
* extractVol2.py
    * It does not extract VOL files yet, but can read file names, of the following games:
        * Red Baron 3D (probably Red Baron 2 as well)
        * Silent Thunder - A10 Tank Killer 2

Here is a list of games which use the 3Space engine: http://sierrachest.com/index.php?a=engines&id=30

However, the file formats vary between all of them.

I will document each game over time, however here are additional "VOL" file formats I have discovered and their related games:
* RMF + 001 + 002 + etc
    * Red Baron
    * A-10 Tank Killer 1.5
    * Nova 9
* DYN
    * Aces of the Pacific
    * Aces over Europe

Soon there will be extractBmf.py and extractDyn.py.


All of the DTS files from the Metaltech games can be converted with the convert_dts script I found, but the DTS files in the shapes.VOL file in Aces of the Deep do not convert with the script (https://github.com/booto/convert_dts).

In addition, most of the older games, like Mechwarrior and Abrams Battletank, don't seem to use similar volume formats, and the files appear to be unarchived (for the most part).

All of the scripts will be made to extract all files into a folder with the same name as the vol file. This will be expanded up over time.

Current Usage
  `python extractVol.py someFile.vol [someFile2.vol] [someFile3.vol]`

Makes a folder per vol file and extracts the contents of each into them.

The formats are currently being reverse engineering through the use of a hex editor, and in the case of Earthsiege/Earthsiege 2, comparing already extracted files to values in hex.
