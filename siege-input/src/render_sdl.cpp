#include <memory>
#include <imgui.h>
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "render.hpp"

struct Siege_RenderContext
{
  Siege_RenderContext()
      : renderer(nullptr, SDL_DestroyRenderer)
  {
  }

  std::unique_ptr<SDL_Renderer, void(*) (SDL_Renderer*)> renderer;
};


std::shared_ptr<Siege_RenderContext> Siege_Init(SDL_Window* window)
{
  auto context = std::make_shared<Siege_RenderContext>();
  context->renderer.reset(SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED));

  if (!context->renderer)
  {
    SDL_Log("Error creating SDL_Renderer!");
    return nullptr;
  }

  if (!ImGui_ImplSDL2_InitForSDLRenderer(window, context->renderer.get()))
  {
    return nullptr;
  }

  if (!ImGui_ImplSDLRenderer_Init(context->renderer.get()))
  {
    return nullptr;
  }

  return context;
}

void Siege_Resize(Siege_RenderContext&)
{
}

void Siege_NewFrame()
{
  ImGui_ImplSDLRenderer_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
}

void Siege_RenderReset(Siege_RenderContext& context, ImVec4 clear_color)
{
  SDL_SetRenderDrawColor(context.renderer.get(), (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
  SDL_RenderClear(context.renderer.get());
}

void Siege_RenderDrawData(ImDrawData* data)
{
  ImGui_ImplSDLRenderer_RenderDrawData(data);
}

void Siege_RenderPresent(Siege_RenderContext& context)
{
  SDL_RenderPresent(context.renderer.get());
}

void Siege_Shutdown(ImGuiContext* context)
{
  ImGui_ImplSDLRenderer_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext(context);
}