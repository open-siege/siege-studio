# Siege Extension for Heretic 2

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
* When playing with a controller, the movement and camera behaviour are different compared to how it works with keyboard and mouse.
	* Only when moving forward or backward, the camera becomes detached and turning it rotates it around the player. 
	* However, the rotation does not affect turning of the character, who continues to move in the original direction.
	* Stopping movement causes the camera to reset again. 
	* This is an issue with the game itself, though it would be nice to investigate a workaround or a console variable which could be activated to tweak the behaviour.
* Currently only input defaults are loaded from the game. But it makes sense to populate all settings from the game's primary and default configurations.
