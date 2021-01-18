#ifndef DARKSTARHOOK_GAMECONSOLE_HPP
#define DARKSTARHOOK_GAMECONSOLE_HPP

#include <map>
#include <string>
#include <vector>
#include <Darkstar/Proxy.hpp>

namespace Hook {

    class GameConsole {
        Core::GameConsole *current;

        template<typename TStringCollection>
        void copyArguments(std::vector<const char *> &arguments, const TStringCollection &args) {
            for (const std::string &argument : args) // access by const reference
            {
                arguments.push_back(argument.c_str());
            }
        }

    public:
        GameConsole() {
            current = DarkstarGetConsole();
        }

        Core::GameConsole *getRaw() {
            return current;
        }

        std::string cls();

        std::string sqrt(const std::string &someNumber);

        std::string floor(const std::string &someNumber);

        std::string echo(const std::string &message);

        template<typename TStringCollection>
        std::string echoRange(const TStringCollection &args) {
            std::vector<const char *> arguments;
            arguments.reserve(args.size() + 1);
            arguments.push_back("echo");
            copyArguments(arguments, args);
            return DarkstarConsoleEcho(current, 0, arguments.size(), arguments.data());
        }

        template<typename TStringCollection>
        std::string dbecho(const TStringCollection &args) {
            std::vector<const char *> arguments;
            arguments.reserve(args.size() + 1);
            arguments.push_back("dbecho");
            copyArguments(arguments, args);
            return DarkstarConsoleDbEcho(current, 0, arguments.size(), arguments.data());
        }

        template<typename TStringCollection>
        std::string strcat(const TStringCollection &args) {
            std::vector<const char *> arguments;
            arguments.reserve(args.size() + 1);
            arguments.push_back("strcat");
            copyArguments(arguments, args);
            return DarkstarConsoleStrCat(current, 0, arguments.size(), arguments.data());
        }

        std::string quit();

        std::string exec(const std::string &filename);

        std::string eval(const std::string &someCode);

        std::string
        exportVariables(const std::string &varableGlob, const std::string &filename, const std::string &append);

        std::string deleteVariables(const std::string &varableGlob);

        std::string
        exportFunctions(const std::string &functionGlob, const std::string &filename, const std::string &append);

        std::string deleteFunctions(const std::string &functionGlob);

        std::string trace();

        std::string debug();

        void addConsumer(Core::ExternalConsoleConsumer *consumer);

        void addCommandFunc(int id, const std::string &name, Core::ExternalConsoleCallbackFunc func, int runLevel = 0);

        void addCommand(int id, const std::string &name, Core::ExternalConsoleCallback *callback, int runLevel = 0);

        std::string removeCommand(const std::string &name);
    };
}

#endif //DARKSTARHOOK_GAMECONSOLE_HPP
