#include <vector>
#include <string>
#include <functional>

struct SDL_Window;

namespace siege
{
    struct AppCallbacks
    {
        std::function<void(::SDL_Window*)> onWindowCreated;
        std::function<void()> onNewFrame;
    };

    int AppMain(std::vector<std::string> args, AppCallbacks callbacks);
}