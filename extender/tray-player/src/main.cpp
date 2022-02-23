#include <future>
#include <atomic>
#include <array>
#include <functional>
#include <unordered_map>
#include <wx/wx.h>
#include <wx/mediactrl.h>
#include <mio/mmap.hpp>
#include "wx_remote_state.hpp"

int main(int argc, char* argv[])
{
  std::error_code error;
  mio::mmap_sink channel = mio::make_mmap_sink("player.ipc.0", error);

  wxApp::SetInitializerFunction([]() -> wxAppConsole* {
    wxAppConsole::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE,
      "Darkstar Extender Music Player");
    return new wxApp();
  });

  // TODO handle changes in shared memory here
  wxEntryStart(argc, argv);
  auto* app = wxApp::GetInstance();
  app->CallOnInit();

  auto* frame = new wxFrame(nullptr, wxID_ANY, "Darkstar Extender Music Player");
  auto* media = new wxMediaCtrl(frame, wxID_ANY);

  std::atomic_bool running = true;

  std::unordered_map<function, std::function<function_result(function_arg)>> funcs = {
    { function::load, [media, &channel](function_arg arg) {
        auto* volatile path_data = reinterpret_cast<const char*>(channel.data() + sizeof(std::array<function_info, function_length>));
       function_result result{};
       std::string_view real_path(path_data, arg.length);
       result.success = false;

       if (!real_path.empty())
       {
         result.success = std::uint32_t(media->Load(std::string(real_path)));
         std::this_thread::sleep_for(std::chrono::nanoseconds(250));
       }

       return result;
     } },
    { function::play, [media](auto) {
       function_result result{};
       result.success = std::uint32_t(media->Play());
       return result;
     } },
    { function::pause, [media](auto) {
       function_result result{};
       result.success = std::uint32_t(media->Pause());
       return result;
     } },
    { function::stop, [media](auto) {
       function_result result{};
       result.success = std::uint32_t(media->Stop());
       return result;
     } },
    { function::get_volume, [media](auto) {
       function_result result{};
       result.volume = float(media->GetVolume());
       return result;
     } },
    { function::set_volume, [media](function_arg arg) {
       function_result result{};
       result.success = std::uint32_t(media->SetVolume(double(arg.volume)));
       return result;
     } },
    { function::length, [media](auto) {
       function_result result{};
       result.length = std::uint32_t(media->Length());
       return result;
     } },
    { function::tell, [media](auto) {
       function_result result{};
       result.success = std::uint32_t(media->Tell());
       return result;
     } },
    { function::seek, [media](function_arg arg) {
       function_result result{};
       result.success = std::uint32_t(media->Seek(std::uint32_t(arg.position)));
       return result;
     } }
  };

  auto task = std::async(std::launch::async, [app, &running, &channel, &funcs]() {
    while (running)
    {
      auto* volatile functions = reinterpret_cast<std::array<function_info, function_length>*>(channel.data());

      auto info_iter = std::find_if(functions->begin(), functions->end(), [](const auto& info) { return info.status == function_status::busy; } );

      std::atomic_bool executed = true;

      if (info_iter != functions->end() && running)
      {
        executed = false;
        app->CallAfter([info_iter, &funcs, &executed]() {
          info_iter->result = funcs.at(info_iter->function)(info_iter->arg);
          info_iter->status = function_status::idle;
          executed = true;
        });
      }

      while (!executed)
      {
        std::this_thread::yield();
      }
    }
  });

  frame->SetWindowStyleFlag(wxFRAME_NO_TASKBAR);

  app->OnRun();
  running = false;
  task.wait();
  return 0;
}
