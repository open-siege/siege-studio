#include <windows.h>
#include <detours.h>

#include <sstream>
#include <iostream>

#include <filesystem>
#include <string_view>

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

    std::cout << "Loading " << targetPath << " with parameters " << parameters.str() << '\n';
    if (DetourCreateProcessWithDllExA(targetPath.string().c_str(),
                                      const_cast<char*>(parameters.str().c_str()),
                                      nullptr,
                                      nullptr,
                                      FALSE,
                                      0,
                                      nullptr,
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