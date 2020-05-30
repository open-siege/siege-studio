//   Important note about DLL memory management when your DLL uses the
//   static version of the RunTime Library:
//
//   If your DLL exports any functions that pass String objects (or structs/
//   classes containing nested Strings) as parameter or function results,
//   you will need to add the library MEMMGR.LIB to both the DLL project and
//   any other projects that use the DLL.  You will also need to use MEMMGR.LIB
//   if any other projects which use the DLL will be performing new or delete
//   operations on any non-TObject-derived classes which are exported from the
//   DLL. Adding MEMMGR.LIB to your project will change the DLL and its calling
//   EXE's to use the BORLNDMM.DLL as their memory manager.  In these cases,
//   the file BORLNDMM.DLL should be deployed along with your DLL.
//
//   To avoid using BORLNDMM.DLL, pass string information using "char *" or
//   ShortString parameters.
//
//   If your DLL uses the dynamic version of the RTL, you do not need to
//   explicitly add MEMMGR.LIB as this will be done implicitly for you

#pragma hdrstop
#pragma argsused

#include <cstdint>
#include <memory>

#include <pybind11/embed.h> // everything needed for embedding
namespace py = pybind11;
#include <string>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstdint>

#define DARKCALL __fastcall



static std::thread* pythonThread = nullptr;
int attachedCount = 0;


struct GameConsole;


int PythonMain(int argc, char *argv[])
{
	wchar_t *program = Py_DecodeLocale(argv[0], NULL);
	if (program == NULL) {
		fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
		exit(1);
    }
	Py_SetProgramName(program);  /* optional but recommended */
	Py_Initialize();
	PyRun_SimpleString("from time import time,ctime\n"
					   "print('Today is', ctime(time()))\n");
	if (Py_FinalizeEx() < 0) {
		exit(120);
	}
	PyMem_RawFree(program);
	return 0;
}


struct ConsoleConsumer
{
	virtual void DARKCALL consoleMessage(GameConsole*, const char *consoleLine) = 0;
};

std::uint32_t getConsole = 0x5E3004;
std::uint32_t addConsumerAddress = 0x5E336C;
std::uint32_t evalAddress = 0x5E2BBC;

typedef GameConsole* (DARKCALL* GetConsoleFunc) ();
typedef void (DARKCALL* AddConsoleConsumerFunc) (GameConsole* console, ConsoleConsumer*);
typedef const char* (DARKCALL* ConsoleEvalFunc) (GameConsole* console, int functionId, int argc, const char** argv);

extern char  etext, edata, end;


namespace Game
{
	GetConsoleFunc GetConsole = (GetConsoleFunc)getConsole;
	AddConsoleConsumerFunc AddConsoleConsumer = (AddConsoleConsumerFunc)addConsumerAddress;
	ConsoleEvalFunc ConsoleEval = (ConsoleEvalFunc)evalAddress;
}



struct TestConsoleConsumer : public ConsoleConsumer
{
		virtual void DARKCALL consoleMessage(GameConsole*, const char *consoleLine)
		{
			std::ofstream file("super-special-log.log", std::ios_base::app);
			file << "A message from the other side!" << std::endl;
			file << consoleLine << std::endl;
			try
			{
				py::scoped_interpreter guard{};
				pybind11::eval_file("simple.py");
			}
			catch(const std::exception& ex)
			{
				file << "An error ocurred with python" << std::endl;
				file << ex.what() << std::endl;
            }
		}
};


void runPython()
{
	try
	{
		std::ofstream file("test.log");
		file << "Hello there!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(20));
        const char* arguments[] =
        {
            "eval",
            "echo(\"Hello world from C++\");"
        };

		file << Game::ConsoleEval(nullptr, 0, 2, arguments);

		auto gameConsole = Game::GetConsole();
		Game::AddConsoleConsumer(gameConsole, new TestConsoleConsumer());

        //py::scoped_interpreter guard{};
        //py::print("Hello, World!"); // use the Python API
        //pybind11::eval_file("simple.py");
    }
    catch (const std::exception & ex)
    {
        std::ofstream file("error.log");
        file << "An error occurred: " << ex.what() << std::endl;
    }
}

extern "C" int _libmain(unsigned long reason)
{
	return 1;
}




extern "C" __declspec(dllexport) void* _cdecl MS_Malloc(std::size_t size)
{
    if (pythonThread == nullptr)
    {
       pythonThread = new std::thread(runPython);
    }
    //return 0;
	return std::malloc(size);
}

extern "C" __declspec(dllexport) void _cdecl MS_Free(void* data)
{
    std::free(data);
}

extern "C" __declspec(dllexport) void* _cdecl MS_Realloc(void* data, std::size_t size)
{
    return std::realloc(data, size);
}

extern "C" __declspec(dllexport) void* _cdecl MS_Calloc(std::size_t num, std::size_t size)
{
    return std::calloc(num, size);
}
