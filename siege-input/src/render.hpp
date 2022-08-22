#ifndef OPEN_SIEGE_RENDER_HPP
#define OPEN_SIEGE_RENDER_HPP

#include <memory>
#include <imgui.h>
#include <SDL.h>

struct Siege_RenderContext;

std::shared_ptr<Siege_RenderContext> Siege_Init(SDL_Window* window);
void Siege_Resize(Siege_RenderContext&);
void Siege_RenderReset(Siege_RenderContext&, ImVec4 clear_color);
void Siege_RenderPresent(Siege_RenderContext&);

void Siege_NewFrame();
void Siege_RenderDrawData(ImDrawData* data);
void Siege_Shutdown(ImGuiContext* context);

#endif// OPEN_SIEGE_RENDER_HPP
