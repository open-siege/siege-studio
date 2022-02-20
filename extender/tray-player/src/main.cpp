#include <utility>
#include <iostream>
#include <future>
#include <wx/wx.h>
#include <wx/mediactrl.h>

wxAppConsole* createApp()
{
    wxAppConsole::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE,
                                    "Darkstar Extender Music Player");
    return new wxApp();
}

int main(int argc, char *argv[])
{
    AllocConsole();
    wxApp::SetInitializerFunction(createApp);
    wxEntryStart(argc, argv);
    auto* app = wxApp::GetInstance();
    app->CallOnInit();

    auto* frame = new wxFrame(nullptr, wxID_ANY, "Darkstar Extender Music Player");

    frame->CreateStatusBar();
    frame->SetStatusText("Darkstar Extender Music Player");

    frame->Bind(wxEVT_MENU, [frame](auto &event)
    {
        frame->Close(true);
    }, wxID_EXIT);

    auto *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(new wxMediaCtrl(frame, wxID_ANY), 1, wxEXPAND, 0);

    sizer->SetSizeHints(frame);
    frame->SetSizer(sizer);

    frame->SetSize(640, 480);
    frame->Show(true);

    app->OnRun();
    return 0;
}
