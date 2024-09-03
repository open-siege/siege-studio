### PAL File Format

PAL files are typically palettes for use with other image files present in whichever games use them.

Depending on the version of the engine, the format of the PAL file can vary greatly.

#### Variations

Here are some of the most common palette formats present across Dynamix's games:
* The PAL: tagged format.
    * The variant of the format may contain colours for EGA, CGA and VGA (sometimes altogether).
    * This variant can be found in the following games:
        * A-10 Tank Killer 1.0
        * David Wolf Secret Agent
        * Die Hard
        * MechWarrior
        * F-14 Tomcat
        * Red Baron
        * Stellar 7
        * Heart of China
        * A-10 Tank Killer 1.5
        * Nova 9: The Return of Gir Draxon
        * Aces of the Pacific
        * Aces Over Europe
        * Betrayal at Krondor
        * Aces of the Deep
* The DPL file format, with the { 0x0f, 0x00, 0x28, 0x00 } tag.
    * This variant can be found in the following games:
        * MetalTech: Earthsiege
        * MetalTech: Battledrome
        * Earthsiege 2
* The RIFF Microsoft PAL format.
    * The file extension could either be PAL or PPL, depending on the game.
    * This variant can be found in the following games:
        * A-10 Tank Killer 2: Silent Thunder
        * Trophy Bass 3D
        * Trophy Bass 4
        * Driver's Education '98
        * Driver's Education '99
        * Starsiege
        * Starsiege: Tribes
* The PPAL tagged format.
    * This is known as the Phoenix Palette format.
    * This variant can be found in the following games:
        * Front Page Sports: Ski Racing
        * A-10 Tank Killer 2: Silent Thunder
* The PL98 tagged format.
    * The file extension will either be PPL or IPL.
    * This variant can be found in the following games:
        * Starsiege
        * Starsiege: Tribes

#### Source Code
* PAL/IPL/PPL (Windows and Phoenix; Partial DPL support)
    * [siege-modules/siege-content/include/siege/content/pal/palette.hpp](/siege-modules/siege-content/include/siege/content/pal/palette.hpp)
    * [siege-modules/siege-content/src/pal/palette.cpp](/siege-modules/siege-content/src/pal/palette.cpp)