#ifndef OPEN_SIEGE_RENDER_HPP
#define OPEN_SIEGE_RENDER_HPP

#include <memory>
#include <imgui.h>
#include <SDL.h>

namespace siege
{
  struct RenderContext;

  std::shared_ptr<RenderContext> RenderInit(SDL_Window* window);
  void Resize(RenderContext&);
  void RenderReset(RenderContext&, ImVec4 clear_color);
  void RenderPresent(RenderContext&);

  void NewFrame();
  void RenderDrawData(ImDrawData* data);
  void RenderShutdown(ImGuiContext* context);
}

#endif// OPEN_SIEGE_RENDER_HPP
