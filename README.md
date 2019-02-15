# Darkstar DTS Converter

WARNING - Still being actively developed - It can convert models to OBJ, but still needs additional work.

This is a Python script designed to convert DTS files from the Darkstar Engine, used in Starsige and Starsiege: Tribes, into a more modern format.

Currently the script can export a DTS (from a default keyframe) to an OBJ file.

The ultimate goal will be to convert the files into a format such as 3DS or DAE, in addition to the goals below.

This is a learning project, with the intention being multifold:
* Learning to programme with Python
* Understanding 3D model formats better
* Providing new opportunities to mod or port the respective games' assets

Current Usage
`python convertDts someFile.dts`
Writes out someFile.obj

Inspired by a DTS converter for Earthsiege 2 (https://github.com/booto/convert_dts)

The inner workings of the file format gleaned from (https://github.com/jamesu/TribesViewer) and (https://github.com/AltimorTASDK/TribesRebirth)
