## Siege Extender

### Summary
Several games using the Darkstar engine have a built in scripting system.

Through the use of data gathered from investigations made with Binary Ninja, and further code written to take advantage of Microsoft Detours and the Borland C++ ABI, it is possible to extend these games via their scripting engines.

The primary target of this work is:
* Starsiege
* Starsiege: Tribes
* Front Page Sports: Ski Racing
* Kings Quest 8: Mask of Eternity

In terms of the scripting present in each game, Ski Racing and Mask of Eternity have their own version of the CScript engine, while Starsiege and Starsiege Tribes share the next iteration of that engine.

### Planned Features
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

See [project source](https://github.com/open-siege/open-siege).