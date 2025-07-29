# Siege Extension for Soldier of Fortune 2 (Multiplayer)

## Features
* Online multiplayer support through a WinSock wrapper over Zero Tier Sockets (wsock32-on-zero-tier).
* Support for configuring key bindings for keyboard and mouse.
* Automatic configuration of controller settings to play the game with a controller.
* The ability to specific additional settings for the game from Siege Studio or Siege Launcher, such as:
	* Selection of the map to start.
	* Selection of the exe to use.
	* Resolution settings.

## Known Issues
* PS4 controllers are not currently configured correctly and the game has continuous spinning of the camera because it is detecting input from the triggers.
	* An appropriate fix will be researched and then implmented in a future update.
* Currently only input defaults are loaded from the game. But it makes sense to populate all settings from the game's primary and default configurations.
