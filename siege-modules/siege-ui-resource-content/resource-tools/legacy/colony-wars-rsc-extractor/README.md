# Colony Wars RSC extractor

** Work will resume in May, with the other game extractors ** 

This is a Python script designed to extract RSC files used in Colony Wars and Colony Wars: Vengeance. The RSC file in Colony Wars: Red Sun is not yet supported as it uses some form of compression.

Currently, the script can extract all files out of a RSC without any further modification.

The primary focus has been to obtain BND, TMD and TIM files for conversion to other formats. Secondarily, there has been an interest in seeing how the game works through its text based files.

Currently it just extracts all files into a folder called 'extracted'

This project is related to https://github.com/matthew-rindel/colony-wars-bnd-converter


Current Usage
  `python extractRsc.py someFile.rsc [someFile2.rsc] [someFile3.rsc]`

All extracted files will go into the 'extracted' folder. It currently does not create this folder automatically. Updates will be made to use the name of the RSC file and create the folder automatically.

The format was reverse engineered initially by using the already extracted FRONT.OVL file on Colony Wars Disc 1 (and Disc 2) to figure out the file offsets in the file.
