### DTB File Format

A DTB file typically contains multiple textures relating to a mission of the game.

Contents of the file are compressed using the LZ-Huffman-based algorithm inside of the game.

Typically there is a lightmap and a heightmap, and there are multiple files which are linked together via
a [DTF file](DTF.md).

### See Also

DTB files are typically embedded into [VOL files](/siege-modules/foundation/siege-resource/src/VOL.md).

Other files which contain LZ-Huffman compressed data:
* [Save games](/siege-modules/foundation/siege-configuration/src/Starsiege%20save%20games.md)
* [VOL files](/siege-modules/foundation/siege-resource/src/VOL.md)
* [VEH files](/siege-modules/foundation/siege-configuration/src/VEH.md)
* [DIL files](DIL.md)

The core game engine:
* [Darkstar Game Engine](/siege-modules/extension/siege-extension-3space/src/darkstar.md)