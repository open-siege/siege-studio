#include <string>
#include <optional>
#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>

struct button_data : public wxClientData
{
  bool pressed;

  explicit button_data(bool pressed) : pressed(pressed) {}
};

struct texture_info
{
  std::string filename;
  wxRect dimensions;
  std::optional<wxRect> middle;
  std::vector<wxImage> image_components;
};

struct widget_info
{
  wxImage main_image;
  texture_info normal_texture;
  texture_info pressed_texture;
  texture_info disabled_texture;
  texture_info focused_texture;
};

std::vector<wxImage> get_texture_components(const texture_info& info, std::optional<wxImage> background = std::nullopt)
{
  std::vector<wxImage> results;

  if (!background.has_value())
  {
    background = wxImage();
    background.value().LoadFile(info.filename, wxBITMAP_TYPE_PNG);
  }

  if (info.middle.has_value())
  {
    results.reserve(3);
    wxImage buttonImage = background.value().GetSubImage(info.dimensions);

    auto middle_dimensions = info.middle.value();

    results.emplace_back(buttonImage.GetSubImage(wxRect(0, middle_dimensions.GetY(), middle_dimensions.GetX(), middle_dimensions.GetHeight())));
    results.emplace_back(buttonImage.GetSubImage(middle_dimensions));

    auto left_over_width = info.dimensions.GetWidth() - middle_dimensions.GetWidth() - middle_dimensions.GetX();
    results.emplace_back(buttonImage.GetSubImage(wxRect(middle_dimensions.GetX() + middle_dimensions.GetWidth(), middle_dimensions.GetY(), left_over_width, middle_dimensions.GetHeight())));
  }
  else
  {
    results.emplace_back(background.value().GetSubImage(info.dimensions));
  }

  return results;
}

auto getScreenScale(const wxSize& size, const wxImage& background)
{
  double screenY = size.GetHeight();
  double bgY = background.GetHeight();

  return screenY / bgY;
}

wxImage scaleToScreen(const wxSize& size, const wxImage& background)
{
  auto scale = getScreenScale(size, background);

  double bgX = background.GetWidth();
  double bgY = background.GetHeight();

  auto finalX = int(bgX * scale);
  auto finalY = int(bgY * scale);

  if (background.GetWidth() != finalX && background.GetHeight() != finalY)
  {
    return background.Copy().Rescale(finalX, finalY, wxIMAGE_QUALITY_HIGH);
  }

  return background;
}

void draw_texture_components(const std::vector<wxImage>& components, const wxSize& size, wxGraphicsContext& context)
{
  if (components.size() == 3)
  {
    auto left = scaleToScreen(size, components[0]);
    auto middle = scaleToScreen(size, components[1]);
    auto right = scaleToScreen(size, components[2]);
    const auto leftWidthScaled = left.GetWidth();
    const auto middleWidthScaled = middle.GetWidth();
    const auto middleHeightScaled = middle.GetHeight();
    const auto rightWidthScaled = right.GetWidth();

    context.DrawBitmap(left, 0, 0, leftWidthScaled, middleHeightScaled);

    const auto finalSize = size.GetWidth() - rightWidthScaled;

    auto segmentCount = int((finalSize - leftWidthScaled) / middleWidthScaled);
    auto xPos = leftWidthScaled;

    for (auto i = 0; i < segmentCount; ++i)
    {
      context.DrawBitmap(middle, xPos, 0, middleWidthScaled, middleHeightScaled);
      xPos += middleWidthScaled;
    }

    auto remainingWidth = finalSize - leftWidthScaled - (segmentCount * middleWidthScaled);

    if (remainingWidth > 0)
    {
      auto extra = middle.GetSubImage(wxRect(0, 0, remainingWidth, middle.GetHeight()));
      context.DrawBitmap(extra, xPos, 0, extra.GetWidth(), extra.GetHeight());
    }

    context.DrawBitmap(right, finalSize, 0, rightWidthScaled, middleHeightScaled);
  }
}

wxAppConsole* createApp()
{
  wxAppConsole::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE,
    "Starsiege");
  return new wxApp();
}

wxStaticText* create_button(widget_info& info,
  wxWindow* parent,
  wxWindowID id,
  const wxString& label = wxEmptyString,
  const wxPoint& pos = wxDefaultPosition,
  const wxSize& size = wxDefaultSize,
  long style = 0)
{
  auto* new_button = new wxStaticText(parent, id, label, pos, size, style);
  auto* client_data = new button_data(false);

  new_button->SetBackgroundColour(wxColor(0, 0, 0, 255));
  new_button->SetForegroundColour(wxColour(255, 193, 0));
  auto font = new_button->GetFont();
  font.MakeBold();
  font.SetPointSize(16);
  new_button->SetFont(font);

  new_button->SetClientObject(client_data);

  new_button->Bind(wxEVT_LEFT_DOWN, [new_button, client_data](auto& wx_event) {
    client_data->pressed = true;
    new_button->Refresh();
  });

  new_button->Bind(wxEVT_LEFT_UP, [new_button, client_data](auto& wx_event) {
    client_data->pressed = false;

    auto event = wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED, new_button->GetId());
    event.SetInt(0);
    event.SetEventObject(new_button);
    new_button->GetEventHandler()->ProcessEvent(event);

    new_button->Refresh();
  });

  new_button->Bind(wxEVT_SIZE, [new_button, &info, client_data](auto&) {
    auto size = new_button->GetSize();
    auto& texture = client_data->pressed ? info.pressed_texture : info.normal_texture;

    if (!new_button->IsEnabled())
    {
      texture = info.disabled_texture;
    }

    texture.image_components = get_texture_components(texture, info.main_image);
    texture.image_components[0] = scaleToScreen(size, texture.image_components[0]);
    texture.image_components[1] = scaleToScreen(size, texture.image_components[1]);
    texture.image_components[2] = scaleToScreen(size, texture.image_components[2]);
    new_button->Refresh();
  });

  new_button->Bind(wxEVT_PAINT, [new_button, &info, client_data](auto& wx_event) mutable {
    auto size = new_button->GetClientSize();
    auto& texture = client_data->pressed ? info.pressed_texture : info.normal_texture;

    if (!new_button->IsEnabled())
    {
      texture = info.disabled_texture;
    }

    wxBufferedPaintDC painter(new_button);

    wxGraphicsContext* context = wxGraphicsContext::Create(painter);

    if (context)
    {
      context->SetBrush(new_button->GetBackgroundColour());
      context->DrawRectangle(size.GetX(), size.GetY(), size.GetWidth(), size.GetHeight());
      draw_texture_components(texture.image_components, size, *context);

      context->SetFont(new_button->GetFont(), new_button->GetForegroundColour());
      auto extent = new_button->GetTextExtent(new_button->GetLabelText());

      context->DrawText(new_button->GetLabelText(), (size.GetWidth() - extent.GetWidth()) / 2.0, (size.GetHeight() - extent.GetHeight()) / 2.0);

      delete context;
    }
  });

  return new_button;
}

int main(int argc, char* argv[])
{
  wxApp::SetInitializerFunction(createApp);

  wxEntryStart(argc, argv);
  auto* app = wxApp::GetInstance();
  app->CallOnInit();
  wxInitAllImageHandlers();

  auto* frame = new wxFrame(nullptr, wxID_ANY, "Starsiege");

  widget_info info{};
  info.normal_texture = {
    "besieged-theme.png",
    wxRect(357, 532, 195, 137),
    wxRect(59, 0, 75, 137)
  };

  info.pressed_texture = {
    "besieged-theme.png",
    wxRect(357, 820, 195, 137),
    wxRect(59, 0, 75, 137)
  };

  info.disabled_texture = {
    "besieged-theme.png",
    wxRect(357, 964, 195, 137),
    wxRect(59, 0, 75, 137)
  };

  info.main_image = wxImage();
  info.main_image.LoadFile(info.normal_texture.filename, wxBITMAP_TYPE_PNG);

  info.normal_texture.image_components = get_texture_components(info.normal_texture, info.main_image);
  info.disabled_texture.image_components = get_texture_components(info.disabled_texture, info.main_image);
  info.pressed_texture.image_components = get_texture_components(info.pressed_texture, info.main_image);

  auto* panel = new wxPanel(frame, wxID_ANY);
  panel->Bind(wxEVT_PAINT, [panel](auto& wx_event) {
    auto size = panel->GetSize();
    wxBufferedPaintDC context(panel);

    context.SetBrush(*wxBLACK_BRUSH);
    context.DrawRectangle(wxRect(size));
  });

  auto* panel_sizer = new wxBoxSizer(wxVERTICAL);
  panel_sizer->AddStretchSpacer(1);
  panel_sizer->Add(create_button(info, panel, wxID_ANY, "TUTORIAL"), 2, wxEXPAND | wxLEFT | wxRIGHT, 16);
  panel_sizer->AddStretchSpacer(1);
  panel_sizer->Add(create_button(info, panel, wxID_ANY, "TRAINING"), 2, wxEXPAND | wxLEFT | wxRIGHT, 16);
  panel_sizer->AddStretchSpacer(1);
  panel_sizer->Add(create_button(info, panel, wxID_ANY, "CAMPAIGN"), 2, wxEXPAND | wxLEFT | wxRIGHT, 16);
  panel_sizer->AddStretchSpacer(1);
  panel_sizer->Add(create_button(info, panel, wxID_ANY, "MULTIPLAYER"), 2, wxEXPAND | wxLEFT | wxRIGHT, 16);
  panel_sizer->AddStretchSpacer(1);
  panel_sizer->Add(create_button(info, panel, wxID_ANY, "OPTIONS"), 2, wxEXPAND | wxLEFT | wxRIGHT, 16);
  panel_sizer->AddStretchSpacer(1);
  panel_sizer->Add(create_button(info, panel, wxID_ANY, "QUIT"), 2, wxEXPAND | wxLEFT | wxRIGHT, 16);
  panel_sizer->AddStretchSpacer(1);

  panel->SetSizer(panel_sizer);

  auto* sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(panel, 33, wxEXPAND, 1);
  sizer->AddStretchSpacer(66);


  sizer->SetSizeHints(frame);
  frame->SetSizer(sizer);

  frame->SetSize(640, 480);
  frame->Show(true);

  app->OnRun();
  return 0;
}