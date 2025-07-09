#pragma once
#include <string>
#include <vector>

/// <summary>
/// Defines the predefined video mode which the software rendered version of the game will use.
/// </summary>
extern int vid_mode;

/// <summary>
/// Defines the volume of the background game music.
/// </summary>
extern float bgmvolume;

/// <summary>
/// The game mode of a multiplayer game.
/// </summary>
extern int deathmatch;

/// <summary>
/// Executes a batch of commands, typically inside of a cfg file.
/// </summary>
/// <param name="filename">A cfg file relative to the game working directory.</param>
void exec(std::string filename);

/// <summary>
/// Displays a concatenated string in the console output.
/// </summary>
/// <param name="strings">An array of strings to be echoed, joined together by a space</param>
void echo(std::vector<std::string> strings);

/// <summary>
/// Allows an alternative name to be defined for a command or sequence of commands.
/// </summary>
/// <param name="name">The name of the new command</param>
/// <param name="command">A string with a single command or a semi-colon separated list of commands</param>
void alias(std::string name, std::string command);

/// <summary>
/// Waits one time slice before executing the next command.
/// </summary>
void wait();

/// <summary>
/// Binds a known key to a command (either a known command or an alias).
/// </summary>
/// <param name="key">A number or letter, or a predefined constant for special keys.</param>
/// <param name="command">The command or alias to execute</param>
void bind(std::string key, std::string command);

/// <summary>
/// Unbinds a key from the previously bound command.
/// </summary>
/// <param name="key">A number or letter, or a predefined constant for special keys.</param>
void unbind(std::string key);

/// <summary>
/// Unbinds all keys. Usually used inside of a key binding config file.
/// </summary>
void unbindall();

/// <summary>
/// Redetects connected joysticks so that they can be usable by the game again. 
/// Usually required when reconnecting controllers during gameplay.
/// </summary>
void joyadvancedupdate();