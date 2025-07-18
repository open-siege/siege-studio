# Siege Extension for Laser Arena

## Features
* Online multiplayer support through a WinSock wrapper over Zero Tier Sockets (wsock32-on-zero-tier).
* Support for configuring key bindings for keyboard and mouse.
* Automatic configuration of controller settings to play the game with a controller.
* The ability to specific additional settings for the game from Siege Studio or Siege Launcher, such as:
	* Selection of the map to start.
	* Selection of the exe to use.
	* Resolution settings.
* Adding the appriopriate command line argument to bypass the launcher.

## Known Issues
* The vid_mode setting only works for LA, while the width and height settings only work for LA_GL_, however all three settings are always present regardless of the selected executable.
	* Siege Studio/Siege Launcher doesn't yet have support for having dynamic fields based on the values of other fields when launching executables.
	* A future may implement this or solve the problem in a different way.
* Currently only input defaults are loaded from the game. But it makes sense to populate all settings from the game's primary and default configurations.
