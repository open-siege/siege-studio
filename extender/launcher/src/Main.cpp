#include <windows.h>
#include <detours.h>

#include <string>
#include <string_view>
#include <sstream>
#include <iostream>

#include <filesystem>
#include <string_view>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define DARKSTAR_MODE_STR TOSTRING(DARKSTAR_MODE)

constexpr static auto mode = std::string_view(DARKSTAR_MODE_STR);

int main(int argc, const char **argv) {
    if (argc <= 0) {
        return 1;
    }

    std::filesystem::path ourPath = argv[0];
    std::filesystem::path targetPath = argv[0];
    targetPath = std::filesystem::current_path() / targetPath.replace_extension("").replace_extension("exe").filename();

    std::stringstream parameters;

    for (auto i = 1; i < argc; ++i) {
        parameters << argv[i] << " ";
    }

    STARTUPINFO info{};
    PROCESS_INFORMATION childInfo{};

    std::vector<char> environment;
    constexpr static auto mode_param = std::string_view{"DARKSTAR_MODE="};
    environment.reserve(mode.size() + mode_param.size() + 2);

    environment.insert(environment.end(), mode_param.data(), mode_param.data() + mode_param.size());
    environment.insert(environment.end(), mode.data(), mode.data() + mode.size());
    environment.push_back('\0');
    environment.push_back('\0');

    std::cout << "Loading " << targetPath << " with parameters " << parameters.str() << '\n';
    if (DetourCreateProcessWithDllExA(targetPath.string().c_str(),
                                      const_cast<char*>(parameters.str().c_str()),
                                      nullptr,
                                      nullptr,
                                      FALSE,
                                      0,
                                      reinterpret_cast<void*>(environment.data()),
                                      nullptr,
                                      &info,
                                      &childInfo,
                                      "darkstar.dll",
                                      nullptr) == TRUE) {
        WaitForSingleObject(childInfo.hProcess, INFINITE);
    } else {
        std::cerr << "Could not load " << targetPath << " because " << GetLastError() << '\n';
    }
}