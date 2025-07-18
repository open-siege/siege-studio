# Siege Studio + Siege Launcher 25.29 Development Release
* Siege Launcher, the commercial version of Siege Studio, is officially released on itch.io: https://thesiegehub.itch.io/siege-launcher
* Fixed issue with the preferences view not showing correctly.
* Fixed issue with games not detecting after a second launch.
* The working directory now changes when switching tabs or when opening files.
    * Before the working directory would only change when selecting a game from the intial view or when clicking on "Open Folder as Workspace".
    * Now, each tab has a working directory associated with it and the "Open Folder as Workspace" command only changes the working directory of the initial tab.
    * When opening an executable it always gets a new working directory.
    * When opening any other file, it only gets a new working directory if it isn't inside of the folder hierarchy.
        * If it does get a new working directory, it will be relative to the nearest executable.
* Fixed various issues and crashes with wsock32-over-zero-tier.
* Fixed various controller configuration issues for supported id Tech games.
* Added notes for various Extension modules featured in Siege Launcher.
    * There are several, but the most notable ones are for:
        * [Quake](/siege-modules/extension/siege-extension-id-tech/src/quake.md) 
        * [Quake 2](/siege-modules/extension/siege-extension-id-tech/src/quake-2.md) 
        * [Quake 3](/siege-modules/extension/siege-extension-id-tech/src/quake-3.md) 
        * See [Games Supported by Siege Launcher](/docs/launcher-game-support.md) for a full list.

# Siege Studio 25.28 Development Release
* 3Space Studio renamed to Siege Studio.
* Officially released on itch.io: https://thesiegehub.itch.io/siege-studio
* New versioning format in using year and week number.
* Siege Studio is fully rewritten to use the Windows API directly on Windows instead of WxWidgets and ImGui.
    * The downside of this is that there is no UI available for Linux. 
* Support for light and dark mode UI styling based on system-wide settings.
* Support for opening DLL and EXE files and previewing several embedded resource formats.
* New game extension plugin system to support compatibilty fixes and enhanced configuration of games when launching them.
    * Online networking support for some games is available through a new WinSocks over Zero Tier wrapper which allows networking over Zero Tier in user mode, without needing a full Zero Tier install or system level network adapters.  
* New game installation plugin system to support installing games from back-up archives or disc images.
* New tool called game-unpack which uses the installation plugin system to install game back-ups. The tool can run as a command line app or in GUI mode.
* The unvol tool has been updated to run as a GUI app or a command line app on Windows. 
    * It also supports self-extraction which can be configured via Siege Studio when an archive file is open in it.
* Support games are detected and shown on start-up using the game extension system (though there is currently an issue on second-launch where games are not detected again).
* Many new file formats are support for multiple games. Documentation pending. 

# Open Siege 0.6.3 Release
Features planned to be worked on for this release:
* Siege Input
   * Add profiles for various input devices to be supported by Siege Input.
   * Improve the internal profile format by adding support for higher level descriptions of button layouts.
   * Multiple window/viewport support added on Windows.

# Open Siege 0.6.2 Release
Features planned to be worked on for this release:
* Entire project
   * Linux support targeting Ubuntu 20.04 LTS when compiling the project from source.

# Open Siege 0.6.1 Release
Features planned to be worked on for this release:
* Siege Input
   * All new UI to list all input devices in system with ability to store data in JSON files.

# Open Siege 0.6.0 Release
Features planned to be worked on for this release:
* 3Space Studio
    * Fixed UI issues such "open in new tab" and DTS view shortcuts not working.
    * "Close other tabs", "Close tabs to the left" and "Close tabs to the right" context menu for tabs.
    * Add section for DTS files to see material data.
    * The ability to set image palettes in bulk.
    * File search under the file type filter above the tree view.
    * Opening ZIP files in the Volume view.
    * Opening of CLN files in the Volume view, enabling support for exploring and extracting:
        * Uprising: Join or Die
        * Uprising 2: Lead and Destroy
        * Gulf War: Operation Desert hammer
    * Opening of ATD files in the Volume view, enabling support for exploring and extracting:
        * Die by the Sword
    * Opening of WAV, OGG and FLAC files in the Sound view.
    * Opening of PNG, JPG and GIF files in the Bitmap view.
* CLI Tools
    * unvol.exe to support all volume formats supported in 3Space Studio (currently only Darkstar Volumes are supported).      
* lib3space
   * File format support:
      * Archive files:
         * ZIP
         * CLN
         * ATD
     * Other new formats are in the UI only because:
        * SFML is used to load the files.
        * SFML is intentionally not added as a dependency to lib3space.

# Open Siege 0.5.3 Release
Many bugs fixed introduced with 0.5.2 (and possibly 0.5.1). Opening palettes or other files shouldn't cause crashes to the desktop.

Some additional features were added, such as:
* The ability to search for a file inside of the volume view.
* When saving a default palette, it gets persisted to a settings file (assists with bulk exports).
* The ability to convert all bitmap files in a workspace to Windows BMP format.
* Minor UI improvements to the bitmap view

# Open Siege 0.5.2 Release
All projects merged together. This release was a test to ensure AppVeyor worked as expected.
It is very buggy and it is advised to skip to 0.5.3

# 3Space Studio 0.5.1 Release
Configured automated builds using AppVeyor, with new tags being used to create GitHub releases based off of the artifacts of an AppVeyor build.

Functionally the app is the same as what was present with the previous release. The contents of the package have also been updated.

# 3Space Studio 0.5.0 Release
A second release of 3Space Studio and a major revision of the app to support more file formats.

This release is the foundation for new features to come, both for 3Space Studio and also for our sister project, Darkstar Extender: https://github.com/matthew-rindel/darkstar-extender

Here is a summary of all the new features added to 3Space Studio:

* Support for browsing and extracting VOL, RBX, TBV and RMF files from various Dynamix games (DYN was supported but disabled due to several bugs which will be fixed in a later release).
* Archives can be extracted in bulk, for quick access to all files of a supported game.
* Starsiege mission (MIS) files are treated as archives and any herc/tank/flyer presets can be extracted.
* Phoenix and Microsoft palette formats are supported for previewing and use with the bitmap viewer.
* Phoenix and Microsoft bitmap formats can be viewed and customised, especially with palette remapping and exports (for Phoenix bitmaps). In future releases, more formats will be supported and more export options will be added.
* For Earthsiege, the ability to play SFX files and convert them to WAV in bulk.
* For Darkstar DTS files, the ability to convert files into OBJ (in bulk or for an individual file). For bulk conversions, the default frame of animation is used. For export of the currently viewed file, it uses the animation settings on-screen.
* The first steps towards automated testing and continuous integration have been taken, so that new releases can be delivered more easily and frequently.

Read the README for more information: https://github.com/matthew-rindel/3space-studio

Requires the Visual C++ 2019 runtime:

* https://aka.ms/vs/16/release/VC_redist.x86.exe
* https://aka.ms/vs/16/release/VC_redist.x64.exe

Some visuals to help you:
https://www.thesiegehub.com/3space-studio-0-5-0-screenshots/

# 3Space Studio 0.1.2 Release
The first of many releases of 3Space Studio.

Animation is a big step forward in the reverse engineering of Darkstar DTS files.

Use the numpad to control the camera. Numlock needs to be turned on for best results.

The easiest way to make use of dts-to-obj, dts-to-json and json-to-dts is to drag the relevant file/files and drop it/them onto the executable.

Expect bugs. Find existing bugs logged here: https://share.clickup.com/b/h/5-15151441-2/aeb3a2cc99994ef

Read the README for more information: https://github.com/matthew-rindel/3space-studio

Required the Visual C++ 2019 64-bit runtime: https://aka.ms/vs/16/release/VC_redist.x64.exe

Some visuals to help you:
https://thesiegehub.files.wordpress.com/2020/11/3space-studio.gif

https://www.thesiegehub.com/dts-json-conversion-shenanigans/
