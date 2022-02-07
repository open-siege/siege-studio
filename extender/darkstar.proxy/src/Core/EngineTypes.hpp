#ifndef ENGINETYPES_HPP
#define ENGINETYPES_HPP

// Most Darkstar games have functions using Borland fastcall,
// so this is considered the standard calling convention for functions
#ifdef __GNUC__
    #define DARKCALL __attribute__((regparm(3)))
#else
// If we ever use C++ Builder again, this is the proper calling convetion.
// This compiles with Visual C++ but it won't work,
// since its version of fastcall uses two registers instead of three.
    #define DARKCALL __fastcall
#endif

#define APICALL __cdecl

// Visual C++ can't invoke DARKCALL methods
// so we make them as private to catch issues.
#ifdef _MSC_VER
    #define COREACCESS private:
#else
    #define COREACCESS
#endif

#include <cstdint>

namespace Core
{
	struct GameRoot;
	struct GameConsole;
	struct GameManager;
	struct GameObject;

   	using ConsoleCallbackFunc = const char* (DARKCALL*)(GameConsole*, std::int32_t, std::int32_t, const char**);
	using ManagedString = const char*;

	template<typename T>
	struct DynamicArray
	{
		std::uint32_t size;
		std::uint32_t capacity;
		std::uint16_t chunkSize;
		std::uint16_t settings;
        T* data;
    };

	struct ConsoleConsumer
	{
    COREACCESS
		virtual void DARKCALL writeLine(GameConsole*, const char *consoleLine) = 0;
	};

	struct ConsoleCallback
	{
    COREACCESS
		virtual const char* DARKCALL executeCallback(GameConsole* console,
				std::int32_t callbackId,
				std::int32_t argc,
				const char** argv) = 0;
	};

	struct GamePlugin : ConsoleCallback
	{
		GameManager* manager;
		GameConsole* console;
    COREACCESS
        virtual void DARKCALL destroy() = 0;
        virtual void DARKCALL setManager(GameManager* manager) = 0;
		virtual void DARKCALL init() = 0;
		virtual void DARKCALL startFrame() = 0;
		virtual void DARKCALL endFrame() = 0;

        // unlike Borland C++ of old, destructors seem to go at the end of the vtable
        virtual DARKCALL ~GamePlugin() = default;
	};
}

#endif
