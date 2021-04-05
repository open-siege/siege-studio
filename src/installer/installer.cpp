#include <filesystem>

#include <wx/wx.h>

namespace fs = std::filesystem;

void default_wx_deleter(wxWindowBase* control)
{
    if (!control->IsBeingDeleted())
    {
        delete control;
    }
}

wxAppConsole* createApp()
{
    wxAppConsole::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE,
                                    "Install Starsiege");
    return new wxApp();
}

int main(int argc, char** argv)
{
  try
  {
    wxApp::SetInitializerFunction(createApp);
    wxEntryStart(argc, argv);
    auto* app = wxApp::GetInstance();
    app->CallOnInit();
    wxInitAllImageHandlers();

    auto frame = std::shared_ptr<wxFrame>(new wxFrame(nullptr, wxID_ANY, "Install Starsiege"), default_wx_deleter);

    auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    sizer->SetSizeHints(frame.get());

    frame->SetSizer(sizer.release());
    frame->CreateStatusBar();
    frame->SetStatusText("Starsiege Installer");
    frame->Maximize();
    frame->Show(true);

    return app->OnRun();
  }
  catch (const std::exception& exception)
  {
    std::cerr << exception.what() << '\n';
    return -1;
  }

  return 0;
}