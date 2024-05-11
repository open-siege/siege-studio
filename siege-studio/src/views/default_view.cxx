#include "default_view.hpp"
#include <wx/commandlinkbutton.h>

#include <utility>
namespace siege::views
{
  default_view::default_view(siege::platform::file_info info)
    : info(std::move(info)) {}

  void default_view::setup_view(wxWindow& parent)
  {
    static auto first_time = true;

    auto horizontal = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    if (first_time)
    {
      auto file_path = std::filesystem::current_path() / "logo.png";

      auto text = std::make_unique<wxStaticText>(&parent, wxID_ANY, "Welcome to Siege Studio.");
      text->SetWindowStyle(wxALIGN_CENTRE_HORIZONTAL);
      auto font = text->GetFont();
      font.SetPointSize(18);
      text->SetFont(font);

      if (std::filesystem::exists(file_path))
      {
        auto image = wxImage(file_path.string(), wxBITMAP_TYPE_PNG);

        auto logo = std::make_unique<wxStaticBitmap>(&parent, wxID_ANY, wxBitmap(image));

        sizer->AddStretchSpacer(4);
        sizer->Add(logo.release(), 1, wxEXPAND, 0);
        sizer->Add(text.release(), 1, wxEXPAND, 0);
        sizer->AddStretchSpacer(4);
      }
      else
      {
        sizer->AddStretchSpacer(4);
        sizer->Add(text.release(), 2, wxEXPAND, 0);
        sizer->AddStretchSpacer(4);
      }

      horizontal->AddStretchSpacer(2);
      horizontal->Add(sizer.release(), 2, wxEXPAND, 0);
      horizontal->AddStretchSpacer(2);

      parent.SetSizer(horizontal.release());

      first_time = false;
    }
    else if (auto extension = info.filename.extension().string(); !extension.empty())
    {
      extension = extension.substr(1);
      auto text = std::make_unique<wxStaticText>(&parent, wxID_ANY, "This particular file is not yet supported by Siege Studio.\nThough, you can still read about it on our wiki.\nClick the link below to find out more.");
      text->SetWindowStyle(wxALIGN_CENTRE_HORIZONTAL);

      std::string url = "https://github.com/open-siege/open-siege/wiki/" + extension;

      auto command = std::make_unique<wxCommandLinkButton>(&parent, wxID_ANY, url);

      command->Bind(wxEVT_BUTTON, [=](auto&) {
        wxLaunchDefaultBrowser(url);
      });

      sizer->AddStretchSpacer(4);
      sizer->Add(text.release(), 1, wxEXPAND, 0);
      sizer->Add(command.release(), 1, wxEXPAND, 0);
      sizer->AddStretchSpacer(4);

      horizontal->AddStretchSpacer(2);
      horizontal->Add(sizer.release(), 2, wxEXPAND, 0);
      horizontal->AddStretchSpacer(2);

      parent.SetSizer(horizontal.release());
    }
  }
}// namespace siege::views