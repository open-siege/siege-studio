## Open Siege

### Summary
When a certain critical mass is reached in terms of file format support in Siege Studio, it will be possible to combine the code from that project with an engine such as OGRE 3D to produce a fully playable, open-source reimplementation of games such as Starsiege and many others.

Since Siege Studio already supports a large amount of formats for Darkstar, the initial focus will be on:
* Starsiege
* Starsiege: Tribes

In order to achieve this, full support for the following formats would be required:
* [MIS](https://github.com/open-siege/open-siege/wiki/MIS)
* [DIG](https://github.com/open-siege/open-siege/wiki/DIG)
* [DIS](https://github.com/open-siege/open-siege/wiki/DIS)
* [DIL](https://github.com/open-siege/open-siege/wiki/DIL)
* [DTF](https://github.com/open-siege/open-siege/wiki/DTF)
* [DTB](https://github.com/open-siege/open-siege/wiki/DTB)
* [CS](https://github.com/open-siege/open-siege/wiki/CS)

Other formats, like PBM/BMP, DTS, DML, PAL/PPL are already supported, but may require additional improvements.

Game logic for each game would be rewritten, and possibly other engines, such as a physics engine, will be incorporated along side OGRE 3D.

As far as humanly possible, existing mods and scripts will be supported.

### Future Game Support
Other games may be supported. It depends on the level of support Siege Studio can attain for their file formats.

Specifically, to implement some of the older games, some of the following file formats would need to be supported:
* [WLD](https://github.com/open-siege/open-siege/wiki/WLD)
* [MSN](https://github.com/open-siege/open-siege/wiki/MSN)
* [BND](https://github.com/open-siege/open-siege/wiki/BND)
* [DAT](https://github.com/open-siege/open-siege/wiki/DAT)
* [TBL](https://github.com/open-siege/open-siege/wiki/TBL)
* [SCR](https://github.com/open-siege/open-siege/wiki/SCR)
* [Older DTS formats (in progress at the moment)](https://github.com/open-siege/open-siege/wiki/DTS)

