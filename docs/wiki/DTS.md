### DTS File Format

DTS is short for "Dynamix 3Space Shape" and is the most commonly used 3D model format across most of Dynamix's games, and the primary format for the Torque game engine.

For the previous shape format, see [TBL](TBL.md).

#### Variations

Each iteration of the 3Space engine also led to variations of the DTS file format, each with their own binary layout.

Here is a list of the variations of the DTS format:

* 3Space 2.0
    * DTS files with the { 0x08, 0x00, 0x14, 0x00 } tag are present in:
        * Aces of the Deep.
    * DTS files with the { 0x03, 0x00, 0x1e, 0x00 } tag are present in:
        * Earthsiege
        * Earthsiege 2.
    * DGS files contain DTS data, starting with the tag { 0x01, 0x00, 0xbc, 0x02 }, present in:
        * Earthsiege
        * Earthsiege 2.
    * DCS files contain DTS data, starting with the tag { 0xf8, 0x01, 0xbc, 0x02 }, present in:
        * Battledrome
* 3Space 2.5
    * DT2 files with the { 0x64, 0x00, 0x14, 0x00 } tag are present in:
        * A10 Tank Killer 2 - Silent Thunder
    * DTS files with the { 0x65, 0x00, 0x14, 0x00 } tag are present in:
        * Red Baron 2
        * Red Baron 3D
        * Curse You! Red Baron
        * Kid Pilot
        * Pro Pilot '98
        * Pro Pilot USA
        * Pro Pilot '99
* Darkstar
    * DTS files with the PERS tag, with the subsequent TS::Shape string, are present in:
        * Front Page Sports: Ski Racing
        * Trophy Bass 3D
        * Trophy Bass 4
        * Driver's Education '98
        * Driver's Education '99
        * Starsiege
        * Starsiege: Tribes
* Torque
    * DTS files for Torque games start with a byte representing the version number. Usually it is 20+
        * Version 16 files are present in:
            * Field and Stream: Trophy Hunting 4
        * Version 22 files are present in:
            * Tribes 2
            * Field and Stream: Trophy Hunting 5

Common with these formats is a node graph which represents the skeleton of the model, and a series of objects which connect meshes to those nodes.

For Torque DTS files, animations can be embedded within the DTS file or stored externally inside of a [DSQ](DSQ.md) file.