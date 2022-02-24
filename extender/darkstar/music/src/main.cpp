#include <future>
#include <atomic>
#include <array>
#include <functional>
#include <unordered_map>
#include <wx/wx.h>
#include <wx/mediactrl.h>
#include <mio/mmap.hpp>
#include "wx_remote_state.hpp"

struct media_player
{
  std::unordered_map<function, std::function<function_result(function_arg, std::string_view)>> funcs;

  explicit media_player(wxMediaCtrl* media)
  {
    funcs = {
      { function::load, [media](function_arg arg, std::string_view path) {
        function_result result{};
        result.success = false;

        if (!path.empty())
        {
          result.success = std::uint32_t(media->Load(std::string(path)));
          std::this_thread::sleep_for(std::chrono::nanoseconds(250));
        }

        return result;
      } },
      { function::play, [media](auto, std::string_view) {
        function_result result{};
        result.success = std::uint32_t(media->Play());
        return result;
      } },
      { function::pause, [media](auto, std::string_view) {
        function_result result{};
        result.success = std::uint32_t(media->Pause());
        return result;
      } },
      { function::stop, [media](auto, std::string_view) {
        function_result result{};
        result.success = std::uint32_t(media->Stop());
        return result;
      } },
      { function::get_volume, [media](auto, std::string_view) {
        function_result result{};
        result.volume = float(media->GetVolume());
        return result;
      } },
      { function::set_volume, [media](function_arg arg, std::string_view) {
        function_result result{};
        result.success = std::uint32_t(media->SetVolume(double(arg.volume)));
        return result;
      } },
      { function::length, [media](auto, std::string_view) {
        function_result result{};
        result.length = std::uint32_t(media->Length());
        return result;
      } },
      { function::tell, [media](auto, std::string_view) {
        function_result result{};
        result.success = std::uint32_t(media->Tell());
        return result;
      } },
      { function::seek, [media](function_arg arg, std::string_view) {
        function_result result{};
        result.success = std::uint32_t(media->Seek(std::uint32_t(arg.position)));
        return result;
      } }
    };
  }
};

int main(int argc, char* argv[])
{
  std::error_code error;
  mio::mmap_sink channel = mio::make_mmap_sink("player.ipc", error);

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
  std::atomic_bool running = true;

  std::vector<std::string> player_paths;
  player_paths.reserve(80);
  std::unordered_map<std::string_view, media_player> players;


  auto task = std::async(std::launch::async, [app, &running, &channel, &player_paths, &players, frame]() {
    while (running)
    {
      auto* volatile data = reinterpret_cast<process_data*>(channel.data());

      std::atomic_int executions = 0;

      for (auto& player : data->players)
      {
        std::string_view path = player.path.data();
        if (path.empty())
        {
          break;
        }

        auto info_iter = std::find_if(player.functions.begin(), player.functions.end(), [](const auto& info) { return info.status == function_status::busy; } );

        if (info_iter != player.functions.end() && running)
        {
          ++executions;
          app->CallAfter([info_iter, path, &player_paths, &players, &executions, frame]() {

            auto existing_player = players.find(path);

            if (existing_player == players.end())
            {
              player_paths.emplace_back(path);
              auto& back_path = player_paths.back();
              const auto [iter, added] = players.emplace(std::string_view(back_path), media_player(new wxMediaCtrl(frame, wxID_ANY, back_path)));
              existing_player = iter;
            }


            info_iter->result = existing_player->second.funcs.at(info_iter->function)(info_iter->arg, path);
            info_iter->status = function_status::idle;
            --executions;
          });
        }
      }

      while (executions > 0)
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
