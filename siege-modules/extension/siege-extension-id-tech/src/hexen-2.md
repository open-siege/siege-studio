# Siege Extension for Hexen 2

## Features
* Online multiplayer support through a WinSock wrapper over Zero Tier Sockets (wsock32-on-zero-tier).
* Support for configuring key bindings for keyboard and mouse.
* Automatic configuration of controller settings to play the game with a controller.
* The ability to specific additional settings for the game from Siege Studio or Siege Launcher, such as:
	* Selection of the map to start.
	* Selection of the exe to use.
	* Resolution settings.

## Known Issues
* When hosting a game with Zero Tier enabled, it is not possible for a client to connect while it is running. 
	* It is not clear what the cause of this issue is, but if you start connecting as the server is starting up, it does connect - indicating some kind of race condition.
	* Hosting normally does not exhibit this behavior so it is likely an issue with wsock32-on-zero-tier.
	* A possible workaround could be for the host to use the full Zero Tier client and launch their server with the -ip flag, but this has not been tested.
* The vid_mode setting only works for Hexen 2, while the width and height settings only work for GL Hexen 2, however all three settings are always present regardless of the selected executable.
	* Siege Studio/Siege Launcher doesn't yet have support for having dynamic fields based on the values of other fields when launching executables.
	* A future may implement this or solve the problem in a different way.
* Currently only input defaults are loaded from the game. But it makes sense to populate all settings from the game's primary and default configurations.
