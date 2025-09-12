# Siege Extension for Daikatana

## Features
* Online multiplayer support through a WinSock wrapper over Zero Tier Sockets (wsock32-on-zero-tier).
* Support for configuring key bindings for keyboard and mouse.
* Automatic configuration of controller settings to play the game with a controller.
* The ability to specific additional settings for the game from Siege Studio or Siege Launcher, such as:
	* Selection of the map to start.
	* Selection of the exe to use.
	* Resolution settings.

## Known Issues
* When playing with a non-XInput controller, looking up and down will not work correctly on the right stick.
	* This is a quirk of the Quake 2 engine and can be fixed by executing "+mlook" in the console, which allows looking to work correctly.
	* Siege Studio/Siege Launcher simulates a keypress of the key bound to +mlook but only has code to check XInput controllers for input.
	* This will be addressed in a future version.
* Currently only input defaults are loaded from the game. But it makes sense to populate all settings from the game's primary and default configurations.
