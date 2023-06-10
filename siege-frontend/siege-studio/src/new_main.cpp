#define SDL_MAIN_HANDLED

#include <imgui.h>
#include "interface/main.hpp"

int main(int argc, char** argv)
{
  siege::AppCallbacks callbacks;

  callbacks.onNewFrame = {
    []() {
        ImGui::Begin("Input Info");

        ImGui::End();

        ImGui::Begin("Other Info");

        ImGui::End();
    }
  };

  return siege::AppMain(std::vector<std::string>(argv, argv + argc), std::move(callbacks));
}