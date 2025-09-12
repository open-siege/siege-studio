### VEH File Format

Represents a vehicle preset for use in Starsiege.

Compressed using the LZ-Huffman-based algorithm inside of the game.

Each file begins with any one of the three tags:
* HERC
* TANK
* FLYR

The contents contain references to the following fields, with IDs found in associated script files:
* Engine - datEngine.cs
* Reactor - datReactor.cs
* Computer - datIntMounts.cs
* Shields - datEngine.cs
* Armor - datArmor.cs
* Sensors - datSensor.cs
* Special 1 and 2 - datIntMounts.cs
* Mounts 1 to 6 - datWeapon.cs
* Skin - filename to a file in the skins folder

### See Also

VEH files can also be embedded in any of the following files:
* MIS
* [Save games](Starsiege%20save%20games)

Other files which contain LZ-Huffman compressed data:
* [Save games](/siege-modules/content/siege-content-3space/src/Starsiege%20save%20games.md)
* [VOL files](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DTB files](DTB.md)
* [DIL files](DIL.md)

The core game engine:
* [Darkstar Game Engine](/siege-modules/extension/siege-extension-3space/src/darkstar.md)