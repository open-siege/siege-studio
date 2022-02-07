#include <filesystem>
#include <future>
#include <thread>
#include <chrono>

#include <wx/wx.h>
#include <wx/splash.h>
#include <wx/progdlg.h>

namespace fs = std::filesystem;

void default_wx_deleter(wxWindowBase *control) {
    if (!control->IsBeingDeleted()) {
        delete control;
    }
}

wxAppConsole *createApp() {
    wxAppConsole::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE,
                                    "Install Starsiege");
    return new wxApp();
}

auto scaleToScreen(wxImage &background) {
    float screenX = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
    float screenY = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);

    float bgX = background.GetWidth();
    float bgY = background.GetHeight();

    auto scale = screenY / bgY;

    background.Rescale(int(bgX * scale), int(bgY * scale), wxIMAGE_QUALITY_HIGH);

    return std::make_pair(screenX, screenY);

}

int main(int argc, char **argv) {
    try {
        wxApp::SetInitializerFunction(createApp);
        wxEntryStart(argc, argv);
        auto *app = wxApp::GetInstance();
        app->CallOnInit();
        wxInitAllImageHandlers();

        auto frame = std::shared_ptr<wxFrame>(new wxFrame(nullptr, wxID_ANY, "Install Starsiege"), default_wx_deleter);

        frame->SetWindowStyle(wxMAXIMIZE | wxCLIP_CHILDREN);


        wxBitmap bitmap;
        bitmap.LoadFile("setup.bmp", wxBITMAP_TYPE_BMP);


        wxImage background;
        background.LoadFile("Title1024.BMP", wxBITMAP_TYPE_BMP);

        auto screen = std::make_unique<wxSplashScreen>(bitmap, wxSPLASH_CENTRE_ON_SCREEN,
                                                       6000, frame.get(), -1, wxDefaultPosition, wxDefaultSize,
                                                       wxBORDER_SIMPLE | wxSTAY_ON_TOP);

        auto progress = std::make_unique<wxProgressDialog>("Setup",
                                                           "Starsiege Setup is preparing the installer wizard, which will guide you through the rest of the setup process. Please wait.",
                                                           100, frame.get());


        const auto[screenX, screenY] = scaleToScreen(background);

        frame->Bind(wxEVT_PAINT, [frame, &background, screenX = screenX, screenY = screenY](auto &wx_event) {
            wxPaintDC context(frame.get());
            context.SetBrush(*wxBLACK_BRUSH);
            context.DrawRectangle(wxRect(0, 0, screenX, screenY));
            context.DrawBitmap(background, (screenX - background.GetWidth()) / 2, 0, false);
        });


        const auto testSizeY = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y) * 0.1;
        const auto testSizeX = testSizeY * 2;
        const auto testSize = wxSize(testSizeX, testSizeY);


        auto testFrame = std::shared_ptr<wxFrame>(new wxFrame(frame.get(), wxID_ANY, wxEmptyString, wxDefaultPosition, testSize, wxCLIP_CHILDREN | wxFRAME_FLOAT_ON_PARENT), default_wx_deleter);
        auto panel = std::make_unique<wxPanel>(testFrame.get());

        panel->SetSize(testSize);

        auto text = std::make_unique<wxStaticText>(panel.get(), wxID_ANY, "Performing System Tests");
        text->SetWindowStyle(wxALIGN_CENTRE_HORIZONTAL);
        text->SetPosition(wxPoint(0, (testSizeY - text->GetCharHeight()) / 2));
        text->SetSize(testSizeX, text->GetCharHeight());


        auto future = std::async(std::launch::async,
                                 [frame, testFrame, progress = progress.release(), screen = screen.release()]() {
                                     std::this_thread::sleep_for(std::chrono::seconds(6));

                                     bool skip = true;
                                     progress->Update(100, wxEmptyString, &skip);
                                     screen->Destroy();
                                     frame->Maximize();
                                     frame->SetPosition(wxPoint(0, 0));
                                     frame->Show(true);
                                     testFrame->CentreOnParent();
                                     testFrame->Show(true);
                                 });


        return app->OnRun();
    }
    catch (const std::exception &exception) {
        std::cerr << exception.what() << '\n';
        return -1;
    }

    return 0;
}