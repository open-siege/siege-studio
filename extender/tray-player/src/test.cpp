#include <wx/wx.h>
#include <optional>
#include <string_view>
#include <filesystem>
#include "music_player.hpp"

template<typename Callback>
wxButton* bindEvent(wxButton* button, Callback&& callback)
{
  button->Bind(wxEVT_BUTTON, std::forward<Callback>(callback));
  return button;
}

std::optional<std::filesystem::path> get_music_path()
{
  constexpr static auto filetypes = "Music files|*.mp3;*.ogg;*.m4a;*.wav;*.wma;*.flac;*.mid;*.midi";
  auto dialog = std::make_unique<wxFileDialog>(nullptr, "Open a music file", "", "", filetypes, wxFD_OPEN, wxDefaultPosition);

  if (dialog->ShowModal() == wxID_OK)
  {
    const auto buffer = dialog->GetPath().ToAscii();
    return std::string_view{ buffer.data(), buffer.length() };
  }

  return std::nullopt;
}

int main(int argc, char* argv[])
{
  wxApp::SetInitializerFunction([]() -> wxAppConsole* {
    wxAppConsole::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE,
      "Darkstar Extender Music Player Test App");
    return new wxApp();
  });

  wxEntryStart(argc, argv);
  auto* app = wxApp::GetInstance();
  app->CallOnInit();

  music_player player;

  auto* frame = new wxFrame(nullptr, wxID_ANY, "Darkstar Extender Music Player");

  auto *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(bindEvent(new wxButton(frame, -1, "Load"), [&player](auto &event)
  {
    auto path = get_music_path();

    if (path.has_value())
    {
      player.load(path.value());
    }

  }), 1, wxEXPAND, 0);

  sizer->Add(bindEvent(new wxButton(frame, -1, "Play"), [&player](auto &event)
  {
    player.play();
  }), 1, wxEXPAND, 0);

  sizer->Add(bindEvent(new wxButton(frame, -1, "Pause"), [&player](auto &event)
  {
    player.pause();
  }), 1, wxEXPAND, 0);

  sizer->SetSizeHints(frame);
  frame->SetSizer(sizer);

  frame->SetSize(640, 480);
  frame->Show(true);

  app->OnRun();
  return 0;
}
