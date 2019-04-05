# 3Space VOL Extractor

** Work will resume in May, with the other game extractors ** 

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
* extractDyn.py
    * It extracts DYN files from the following games:
        * Aces of the Pacific
        * Aces over Europe + Expansion
        * Aces of the Deep + Expansion
* extractRmf.py
    * It extracts RMF files with related 00x files from the following games:
        * Red Baron
        * A-10 Tank-Killer 1.5
        * Nova 9

Here is a list of games which use the 3Space engine: http://sierrachest.com/index.php?a=engines&id=30

However, the file formats vary between all of them.

All games which use RMF, and all which use DYN (except for Aces of the Deep) seem to use OVL files, which are executable overlays, which replace parts of executing code during runtime to reduce how much memory is used by the game. Stellar 7, while not having any archive files, does have an OVL file called STELLAR.RES. Further research will be made into that format, as well as all the other game formats used.

DYN files seem to be very much the same in structure as the RMF and 001 + 002 + 00x combinations, except being unified into one file format.

DTS files only really show up in the Metaltech games, Aces of the Deep then all the Windows based games. It appears as if TBL was the previous format, but investigation still needs to be made.

All of the DTS files from the Metaltech games can be converted with the convert_dts script I found, but the DTS files in the shapes.VOL file in Aces of the Deep do not convert with the script (https://github.com/booto/convert_dts).

In addition, most of the older games, like Mechwarrior and Abrams Battletank, don't seem to use similar volume formats, and the files appear to be unarchived (for the most part).

All of the scripts will be made to extract all files into a folder with the same name as the vol file. This will be expanded up over time.

Current Usage
  `python extractVol.py someFile.vol [someFile2.vol] [someFile3.vol]`

Makes a folder per vol file and extracts the contents of each into them.

The formats are currently being reverse engineering through the use of a hex editor, and in the case of Earthsiege/Earthsiege 2, comparing already extracted files to values in hex.
