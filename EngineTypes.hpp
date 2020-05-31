#ifndef ENGINETYPES_HPP
#define ENGINETYPES_HPP

// Most Darkstar games have functions using Borland fastcall,
// so this is considered the standard calling convention for functions
#define DARKCALL __fastcall


namespace Engine
{
	struct GameConsole;

	struct ConsoleConsumer
	{
		virtual void DARKCALL writeLine(GameConsole*, const char *consoleLine) = 0;
	};
}

#endif
