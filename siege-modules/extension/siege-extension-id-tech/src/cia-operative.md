# Siege Extension for CIA Operative - Solo Missions

## Features
* Support for configuring key bindings for keyboard and mouse.
* Automatic configuration of controller settings to play the game with a controller.
* The ability to specific additional settings for the game from Siege Studio or Siege Launcher, such as:
	* Selection of the map to start.
	* Selection of the exe to use.
	* Resolution settings.
* Adding the appriopriate command line argument to bypass the launcher.

## Known Issues
* Currently only input defaults are loaded from the game. But it makes sense to populate all settings from the game's primary and default configurations.
* Zero Tier settings are shown because the executable contains references to wsock32. 
	* However, the game itself doesn't appear to have working multiplayer (at least from current testing).
	* Some additional logic has to be added to Siege Launcher/Siege Studio to flag games as not having network support.