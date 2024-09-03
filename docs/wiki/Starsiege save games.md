### Starsiege Save Game File Format

These are files which can be created while playing a single-player campaign.

They go into the savedGames folder with no extension, and contain no header information.

Contents of the file are compressed using the LZ-Huffman-based algorithm inside of the game.

### Structure
The file contains the following information inside of it:
  * The name of the player, chosen when creating the campaign.
  * The face image selected by the player, also chosen when creating the campaign.
  * An array of all possible pilots which could be recruited in that campaign.
    * This information is derived from the various data blocks found in:
      * DatPilot_hu.cs
      * DatPilot_cy.cs
  * The campaign information.
    * This information is derived from the various data blocks of the same structure as found in:
      * campaign/Human/campaign.cs
      * campaign/Cybrid (Advanced)/campaign.cs
  * There is a big blob of bytes between the campaign information and the end of the file. It is currently unknown the exact contents of these bytes. However, it is most likely planetary inventory information.
    * If the data is planetary inventory, then the data is typically defined in any of the following files:
      * hu_planet_init.cs
      * cy_planet_init.cs
  * In the middle of the big blob is the player's team and race (also defined in the campaign data blocks from above).
  * Near the end of the file, the following is present:
    * The squad logo, chosen when creating the campaign.
    * The squad name, also chosen when creating the campaign.
    * The hercs under the player's control, as [VEH](VEH) data.
    * The stats of the missions which the player has completed.
    
### See Also
Other files which contain LZ-Huffman compressed data:
* [DIL files](DIL.md)
* [VOL files](VOL.md)
* [DTB files](DTB.md)
* [VEH files](VEH.md)

The core game engine:
* [Darkstar Game Engine](darkstar.md)

