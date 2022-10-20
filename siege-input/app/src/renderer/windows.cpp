#include <imgui.h>
#include <d3d11.h>
#include <SDL_syswm.h>
#include "imgui_impl_sdl.h"
#include "imgui_impl_dx11.h"
#include "renderer.hpp"

namespace siege
{
   struct RenderContext
   {
     RenderContext()
       : d3d_device(nullptr, [](auto* self) { if (self) self->Release(); }),
         d3d_device_context(nullptr, [](auto* self) { if (self) self->Release(); }),
         swap_chain(nullptr, [](auto* self) { if (self) self->Release(); }),
         main_render_view(nullptr, [](auto* self) { if (self) self->Release(); })
     {
     }

     std::unique_ptr<ID3D11Device, void(*)(ID3D11Device*)> d3d_device;
     std::unique_ptr<ID3D11DeviceContext, void(*)(ID3D11DeviceContext*)> d3d_device_context;
     std::unique_ptr<IDXGISwapChain, void(*)(IDXGISwapChain*)> swap_chain;
     std::unique_ptr<ID3D11RenderTargetView, void(*)(ID3D11RenderTargetView*)> main_render_view;
   };

   bool CreateDeviceD3D(RenderContext& self, HWND hWnd);
   void CreateRenderTarget(RenderContext& self);

   std::shared_ptr<RenderContext> RenderInit(SDL_Window* window)
   {
     SDL_SysWMinfo wmInfo;
     SDL_VERSION(&wmInfo.version);
     SDL_GetWindowWMInfo(window, &wmInfo);
     HWND hwnd = (HWND)wmInfo.info.win.window;

     std::shared_ptr<RenderContext> context = std::make_shared<RenderContext>();

     if (!CreateDeviceD3D(*context.get(), hwnd))
     {
       return nullptr;
     }

     ImGui_ImplSDL2_InitForD3D(window);
     ImGui_ImplDX11_Init(context->d3d_device.get(), context->d3d_device_context.get());

     return context;
   }

   void Resize(RenderContext& self)
   {
     self.main_render_view.reset();
     self.swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
     CreateRenderTarget(self);
   }

   void NewFrame()
   {
     ImGui_ImplDX11_NewFrame();
     ImGui_ImplSDL2_NewFrame();
     ImGui::NewFrame();
   }

   void RenderReset(RenderContext& self, ImVec4 clear_color)
   {
     const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };

     auto* temp = self.main_render_view.get();
     self.d3d_device_context->OMSetRenderTargets(1, &temp, NULL);
     self.d3d_device_context->ClearRenderTargetView(self.main_render_view.get(), clear_color_with_alpha);
   }

   void RenderDrawData(ImDrawData* data)
   {
     ImGui_ImplDX11_RenderDrawData(data);
   }

   void RenderPresent(RenderContext& self)
   {
     self.swap_chain->Present(1, 0);
   }

   void RenderShutdown(ImGuiContext* context)
   {
     ImGui_ImplDX11_Shutdown();
     ImGui_ImplSDL2_Shutdown();
     ImGui::DestroyContext(context);
   }

   void CreateRenderTarget(RenderContext& self)
   {
     ID3D11Texture2D* pBackBuffer;
     self.swap_chain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

     ID3D11RenderTargetView* temp = nullptr;
     self.d3d_device->CreateRenderTargetView(pBackBuffer, NULL, &temp);
     self.main_render_view.reset(temp);

     pBackBuffer->Release();
   }

   bool CreateDeviceD3D(RenderContext& self, HWND hWnd)
   {
     // Setup swap chain
     DXGI_SWAP_CHAIN_DESC sd;
     ZeroMemory(&sd, sizeof(sd));
     sd.BufferCount = 2;
     sd.BufferDesc.Width = 0;
     sd.BufferDesc.Height = 0;
     sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
     sd.BufferDesc.RefreshRate.Numerator = 60;
     sd.BufferDesc.RefreshRate.Denominator = 1;
     sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
     sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
     sd.OutputWindow = hWnd;
     sd.SampleDesc.Count = 1;
     sd.SampleDesc.Quality = 0;
     sd.Windowed = TRUE;
     sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

     UINT createDeviceFlags = 0;
     //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
     D3D_FEATURE_LEVEL featureLevel;

     const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };

     ID3D11Device* temp_device;
     ID3D11DeviceContext* temp_device_context;
     IDXGISwapChain* temp_swap_chain;

     if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &temp_swap_chain, &temp_device, &featureLevel, &temp_device_context) != S_OK)
     {
       return false;
     }

     self.swap_chain.reset(temp_swap_chain);
     self.d3d_device_context.reset(temp_device_context);
     self.d3d_device.reset(temp_device);

     CreateRenderTarget(self);
     return true;
   }
}