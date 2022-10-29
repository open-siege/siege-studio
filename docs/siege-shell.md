## Siege Shell

### Summary
A common trend for Dynamix's games is for the shell/menus to be locked at a specific resolution, typically 640x480, while gameplay, especially if the game is hardware accelerated, runs at whatever resolution the hardware supports, or resolutions far above 640x480.

As the next step in making parts of these games open-source and re-implementing them, this project will provide a new executable to run with the game, and depending on the game, will launch the original game, directly into the gameplay.

When launching the original game, all possible compatibility options can be applied, and/or with modern configuration settings defined.

Primarily, the initial parts of this project will focus on:
* Starsiege
* Starsiege: Tribes

These two games only have one executable responsible for the whole game, however, using Darkstar Extender and with full file support for the game from 3Space Studio, it is possible to launch the original game directly into a mission.

### Planned Features
The possible features, specifically for Starsiege and Starsiege: Tribes include:
* Improved network discovery for multiplayer matches.
* Support for direct integration with tools such as Zero Tier for creating private lobbies without needing a central server.
* User interfaces which can scale to modern resolutions.
* Ease of access for special features such as the mission editor and other modding content.
* Removal of any configuration limits latent in the original games.
* Much more.

### Future Game Support
In terms of supporting other games, it is possible, but further work on 3Space Studio. These are the possible future games which can be supported:
* Metaltech: Earthsiege
* Metaltech: Battledrome
* Earthsiege 2
* A-10 Tank Killer 1.5
* Red Baron
* Aces Over Europe
* Aces of the Pacific
* Aces of the Deep
* Command: Aces of the Deep
* Silent Thunder: A-10 Tank Killer 2
* Front Page Sports: Ski Racing
* King's Quest: Mask of Eternity

Many of the games in the list above have two executables, one for the shell, and one for the game itself.

In this regard, the shell executables can be replaced with a new version which can launch the game, configured to run on modern systems.

#### DOS 3Space Games
The following games are DOS based:
* Metaltech: Earthsiege
* Metaltech: Battledrome
* A-10 Tank Killer 1.5
* Red Baron
* Aces Over Europe
* Aces of the Pacific
* Aces of the Deep

In order for support for these games to be implemented, the following formats, as a minimum, have to be supported in 3Space Studio:
* [TTM](https://github.com/open-siege/open-siege/wiki/TTM)
* [SCR](https://github.com/open-siege/open-siege/wiki/SCR)
* [BMP (the versions specific to these games)](https://github.com/open-siege/open-siege/wiki/BMP)
* [PAL (the versions specific to these games)](https://github.com/open-siege/open-siege/wiki/PAL)
* [DPL (which is currently in progress - for the Metaltech games only)](https://github.com/open-siege/open-siege/wiki/DPL)
* [DBM (which is currently in progress - for the Metaltech games only)](https://github.com/open-siege/open-siege/wiki/DBM)
* [DBA (which is currently in progress - for the Metaltech games only)](https://github.com/open-siege/open-siege/wiki/DBA)
* [DTS/DCS (primary 3D format for the Metaltech games and Aces of the Deep)](https://github.com/open-siege/open-siege/wiki/DTS)
* [TBL (primary 3D format all the other games)](https://github.com/open-siege/open-siege/wiki/TBL)
* The save game/career/vehicle configuration formats for each game

Each of these games have separated executables, and generally the shell executable generates some files before launching the sim executable with specific command line parameters.

The new shell can have options for configuring DOSBox for launching the games, to remap keys and/or ensure the best quality settings are used for audio and graphics.

#### Windows 3Space Games
Various sequels to the above games, with the same dual executable architecture, are present on Windows:
* Earthsiege 2
* Command: Aces of the Deep
* Silent Thunder: A-10 Tank Killer 2

For Earthsiege 2 and Command: Aces of the Deep, the list of files to support is the same as for the DOS games above. 

For Silent Thunder, many of the file formats are supported already, with the following remaining:
* [PAL/PPL support (Phoenix Palette files specifically)](https://github.com/open-siege/open-siege/wiki/PAL)
* [DTS (3Space 2.5 era DTS files, which is currently in progress)](https://github.com/open-siege/open-siege/wiki/DTS)

In terms of adding fixes for these games, specific compatibility fixes may be applied, and/or Darkstar Extender can be used, in a very limited capacity, to detour Win32 APIs which the game depends on but which may have issues with modern Windows. 

#### Windows Darkstar Games
The games are: 
* Front Page Sports: Ski Racing
* King's Quest: Mask of Eternity

These games, in particular, have one executable for the shell and the sim/gameplay.

However, because they have a scripting engine and will be supported by Darkstar Extender, it will be possible to force the games to start levels directly.

Graphics files for these games are already supported, however Mask of Eternity requires support for extracting its VOL files with 3Space Studio.

Both games work best with a Glide wrapper, and the shell can configure those to their best settings.

If using DirectDraw/Direct3D, the best possible settings can be applied, if possible.
