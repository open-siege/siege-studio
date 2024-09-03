### VOL File Format

Across most Dynamix games, files with the VOL extension are highly prevalent, with certain games also having unique extensions which are also VOL files by nature.

VOL is short for "volume", and volume files are intended to store resources for the game they belong to.

Each generation of the 3Space engine has a different binary format for VOL files, though the function of these files remains the same.

#### Variations

Here is a list of variations of the VOL file format:

* RMF + 001, 002, etc...
  * For older games, which were usually stored on floppy disks or similar media.
  * Each numbered file represented a portion of the volume that could then be put onto multiple disks, as needed.
* DYN
  * Certain games had DYN files instead, which have the same binary layout as RMF, but with all of the segments all merged into one file.
  * See [DYN](DYN) for more information.
* VOL (Earthsiege branch)
* VOL (Red Baron 2 branch)
  * The VOL files present in games such as Red Baron 2 and Pro Pilot are not the same as VOL files in games such as Earthsiege, however they have some elements which would appear in later iterations of the format.
* VOL (Darkstar branch)
  * This is the final iteration of the custom VOL format, used in the Darkstar/3Space 3 variants of the engine. 
* VL2 (Tribes 2)
  * This is actually just a ZIP file with the VL2 extension. Most future Torque games make use of their resource files with the ZIP extension.

#### Source Code
* RMF, DYN, VOL (Earthsiege)
    * [include/siege/resource/three_space_resource.hpp](/siege-modules/siege-resource/include/siege/resource/three_space_resource.hpp)
    * [siege-modules/siege-resource/src/three_space_resource.cpp](/siege-modules/siege-resource/src/three_space_resource.cpp)
* VOL - Red Baron 2 and Darkstar 
    * [siege-modules/siege-resource/include/siege/resource/darkstar_resource.hpp](/siege-modules/siege-resource/include/siege/resource/darkstar_resource.hpp)
    * [siege-modules/siege-resource/src/darkstar_resource.cpp](/siege-modules/siege-resource/src/darkstar_resource.cpp)
