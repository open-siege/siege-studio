# Colony Wars BND Converter

WARNING - Still being actively developed - It can convert models to OBJ, but still needs major additional work.

This is a Python script designed to convert BND files from the Colony Wars games into a more compatible format.

Currently only files from Colony Wars: Vengeance have been tested and the parser is still off when it comes to getting polygon and UV information.

To figure out the format, the CUBE.TMD file in CW: Vengeance has been parsed and converted into an OBJ, to see the values and find them inside of the CUBE.BND file also present.

While the cube does export in both cases, there are missing faces in the final OBJ and needs more work to determine the correct values and their order.

TMD embed a variation of the TMD format and also the TIM files for the model. These will be converted too eventually.

This project is related to the RSC extractor needed to get the BND (and TMD) files for the game https://github.com/matthew-rindel/colony-wars-rsc-extractor

Eventually, readTmd.py will be put into it's own project and have full support for all variations of the file format and not just the CUBE.TMD file used.
