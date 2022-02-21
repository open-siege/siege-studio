#include <wx/wx.h>
#include <wx/mediactrl.h>

int main(int argc, char *argv[])
{
  wxApp::SetInitializerFunction([] () -> wxAppConsole* {
      wxAppConsole::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE,
                                      "Darkstar Extender Music Player");
      return new wxApp();
  });

    // TODO handle changes in shared memory here
    wxEntryStart(argc, argv);
    auto* app = wxApp::GetInstance();
    app->CallOnInit();

    auto* frame = new wxFrame(nullptr, wxID_ANY, "Darkstar Extender Music Player");
    new wxMediaCtrl(frame, wxID_ANY);
    app->OnRun();
    return 0;
}
