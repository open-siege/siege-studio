# Darkstar DTS Converter

WARNING - Still being actively developed - It can convert models to OBJ, but still needs additional work.

This is a Python 2.7 script designed to convert DTS files from the Darkstar Engine, used in Starsige and Starsiege: Tribes, into a more modern format. Python 3 support is coming.

Currently the script can export a DTS (from a default keyframe) to an OBJ file.

The ultimate goal will be to convert the files into a format such as 3DS or DAE, in addition to the goals below.

This is a learning project, with the intention being multifold:
* Learning to programme with Python
* Understanding 3D model formats better
* Providing new opportunities to mod or port the respective games' assets

Features still to be added:
* Support for v8 DTS files used in Starsiege Tribes
* Support for UV texture coordinates so that a material file can be generated for proper texture mapping
* Support for exporting animations to a separate file that can be used by custom code (or a general purpose animation format)
* Support for more output formats, such as Torque DTS (the successor format) and whatever other format I can learn
* Support for converting other formats into Darkstar DTS files, for example Earthsiege DTS to Darkstar DTS, or MD2 to DTS
* Support for up or down converting of DTS files to specific versions


Current Usage
  `python convertDts.py someFile.dts`

Writes out someFile.obj

Inspired by a DTS converter for Earthsiege 2 (https://github.com/booto/convert_dts)

The inner workings of the file format gleaned from (https://github.com/jamesu/TribesViewer) and (https://github.com/AltimorTASDK/TribesRebirth)
