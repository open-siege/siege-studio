#include <utility>
#include <wx/wx.h>
#include <windows.h>

wxAppConsole* createApp()
{
    wxAppConsole::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE,
                                        "Siege Extender Tester");
    return new wxApp();
}

int main(int argc, char *argv[])
{
    AllocConsole();
    wxApp::SetInitializerFunction(createApp);
    wxEntryStart(argc, argv);
    auto* app = wxApp::GetInstance();
    app->CallOnInit();

    auto* frame = new wxFrame(nullptr, wxID_ANY, "Siege Extender Tester");

    frame->CreateStatusBar();
    frame->SetStatusText("Siege Extender Tester");

    frame->Bind(wxEVT_MENU, [frame](auto &event)
    {
        frame->Close(true);
    }, wxID_EXIT);

    auto *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(new wxButton(frame, -1, "Click me!"), 1, wxEXPAND, 0);

    sizer->Add(new wxButton(frame, -1, "Don't Click me!"), 1, wxEXPAND, 0);

    sizer->SetSizeHints(frame);
    frame->SetSizer(sizer);

    frame->SetSize(640, 480);
    frame->Show(true);

    app->OnRun();
    return 0;
}
