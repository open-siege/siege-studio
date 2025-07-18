# Siege Extension for Sin

## Features
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
* The game uses WinSock 2 and currently the WinSock On Zero Tier wrapper does not yet support WinSock 2.
	* Part of the problem is that Zero Tier also uses WinSock 2 so extra work has to be done to make sure only the game uses our patched WinSock library and not Zero Tier itself.
	* Work is currently in progress for a solution which may come in a future update.
