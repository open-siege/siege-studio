---
_layout: landing
---

## Welcome to Siege Studio

Siege Studio is a reverse engineering effort to preserve the legacy of Dynamix and their games. It serves as a tool for exploring the files of the various games, as well as converting those files to other formats. The core of Siege Studio can be used as a library for other projects.

### Preview

![](https://media.giphy.com/media/vRus9g5a1ZNSNKVSiR/source.gif)

### File Formats

For more information on the supported file formats of Siege Studio, see the [Game Support](game-support) page.

For technical information surrounding the file formats, see the [main wiki of this project](https://github.com/open-siege/open-siege/wiki).

### Roadmap

This project falls as part of a series of projects to renovate the Earthsiege series. However, as more and more games are supported, the scope of specific sister projects may be expanded to incorporate those games.

The overall project is divided into four phases:

#### Phase 1 - Siege Studio

Siege Studio serves as the foundation for all reverse engineering efforts related to Dynamix's games. As such, it being mature is pivotal to the success of any further undertakings to modernise or remake any of said games.

Thus, the goal of phase 1 is to support as many games in the Dynamix catalogue as possible, with the Earthsiege series being the primary focus.

#### Phase 2 - Siege Extender

See [project info](darkstar-extender).

The purpose of this project is to extend the functionality of games using the Darkstar Engine, with Darkstar being the third major iteration of 3Space technology. Support will be directed towards the following games:

* Starsiege
* Starsiege: Tribes
* Front Page Sports: Ski Racing
* Kings Quest 8: Mask of Eternity

Depending on the game, the list of new features, added through the extender, should be:

* Improved scripting support with the integration of ChaiScript.
* Support for HD textures, beyond the 256x256 size limit.
* Support for new 3D model formats such as Torque DTS and DAE.
* Integration of the scripting engine with Reshade for dynamix shaders during gameplay.
* A REST API for server admins to manage servers.
* Enhanced music support with audio files such as FLAC and OGG, and tracker formats such as MOD, S3M, IT and more.
* Improved UI scaling, to enable the game front-end to support modern resolutions without artefacts or workarounds.
* JSON parsing support.
* Much more.

#### Phase 3 - Siege Shell

See [project info](siege-shell).

With the core of the Darkstar games extended, work will begin for a new UI to be built, replacing the potentially error prone shell portion of the game with a modern equivalent, with more features.

Specifically, this project will focus on:

* Starsiege
* Starsiege: Tribes

In the case of games like Starsiege and Tribes, there can be more advanced integration with services like Discord and the like, while also making a large part of the game fully open source.

Depending on progress of Siege Studio, the following games may also be supported:

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

#### Phase 4 - Open Siege

See [project info](open-siege).

With the GUI front-end portion of various games supported, the next and final component would be to create a full engine replacement, using the code from the previous projects as a base, to be able to play any of the above games without any original binaries.

The goal is to incorporate the logic from Siege Studio with OGRE 3D, and potentially other libraries, to recreate the simulation aspects of each game, and work directly with Siege Shell, and eventually fully incorporate it, as to have a fully open-source implementation of Dynamix's classic games.



