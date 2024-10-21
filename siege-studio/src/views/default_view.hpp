#ifndef SIEGE_DEFAULT_VIEW_HPP
#define SIEGE_DEFAULT_VIEW_HPP

#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <utility>

namespace siege::views
{
  using namespace std::literals;
  constexpr static std::array<std::wstring_view, 97> supported_games = {{
          L"Rise of the Dragon"sv,
          L"Heart of China"sv,
          L"The Adventures of Willy Beamish"sv,
          L"Quarky & Quaysoo's Turbo Science"sv,
          L"The Incredible Machine"sv,
          L"The Even More Incredible Machine"sv,
          L"Sid & Al's Incredible Toons"sv,
          L"The Incredible Machine 2"sv,
          L"The Incredible Machine 3"sv,
          L"3-D Ultra Pinball"sv,
          L"Trophy Bass"sv,
          L"3-D Ultra Pinball: Creep Night"sv,
          L"Hunter Hunted"sv,
          L"Front page Sports: Trophy Bass 2"sv,
          L"MissionForce: CyberStorm"sv,
          L"Outpost 2: Divided Destiny"sv,
          L"3-D Ultra Minigolf"sv,
          L"3-D Ultra Pinball: The Lost Continent"sv,
          L"3-D Ultra NASCAR Pinball"sv,
          L"Cyberstorm 2: Corporate Wars"sv,
          L"3-D Ultra MiniGolf Deluxe"sv,
          L"3-D Ultra Radio Control Racers"sv,
          L"3-D Ultra Cool Pool"sv,
          L"3-D Ultra Lionel Train Town"sv,
          L"3-D Ultra Pinball: Thrillride"sv,
          L"3-D Ultra Lionel Train Town Deluxe"sv,
          L"Maximum Pool"sv,
          L"Return of the Incredible Machine: Contraptions"sv,
          L"The Incredible Machine: Even More Contraptions"sv,
          L"Minigolf Maniacs"sv,
          L"Red Baron"sv,
          L"A-10 Tank Killer 1.5"sv,
          L"Nova 9: The Reutnr Gir Draxon"sv,
          L"Aces of the Pacific"sv,
          L"Aces Over Europe"sv,
          L"Betrayal at Krondor"sv,
          L"Aces of the Deep"sv,
          L"Metaltech: Battledrome"sv,
          L"Metaltech: Earthsiege"sv,
          L"Command: Aces of the Deep"sv,
          L"Earthsiege 2"sv,
          L"Silent Thunder: A-10 Tank Killer 2"sv,
          L"Red Baron 2"sv,
          L"Pro Pilot '98"sv,
          L"Red Baron 3D"sv,
          L"Pro Pilot '99"sv,
          L"Kid Pilot"sv,
          L"Curse You! Red Baron"sv,
          L"Front Page Sports: Ski Racing"sv,
          L"King's Quest: Mask of Eternity"sv,
          L"Driver's Education '98"sv,
          L"Starsiege"sv,
          L"Starsiege: Tribes"sv,
          L"Driver's Education '99"sv,
          L"Field & Stream: Trophy Bass 3D"sv,
          L"Field & Stream: Trophy Bass 4"sv,
          L"Field & Stream: Trophy Hunting 4"sv,
          L"Field & Stream: Trophy Hunting 5"sv,
          L"Tribes 2"sv,
          L"ShadowCaster"sv,
          L"CyClones"sv,
          L"World of Aden: Thunderscape"sv,
          L"In Pursuit of Greed"sv,
          L"Necrodome"sv,
          L"Take No Prisoners"sv,
          L"MageSlayer"sv,
          L"Quake"sv,
          L"Hexen 2"sv,
          L"Laser Arena"sv,
          L"CIA Operative: Solo Missions"sv,
          L"Quake 2"sv,
          L"Heretic 2"sv,
          L"SiN"sv,
          L"Kingpin: Life of Crime"sv,
          L"Soldier of Fortune"sv,
          L"Daikatana"sv,
          L"Anachronox"sv,
          L"Quake 3 Arena"sv,
          L"Heavy Metal: F.A.K.K 2"sv,
          L"Star Trek: Voyager - Elite Force"sv,
          L"American McGee's Alice"sv,
          L"Return to Castle Wolfenstein"sv,
          L"Medal of Honor: Allied Assault"sv,
          L"Star Wars Jedi Knight 2: Jedi Outcast"sv,
          L"Soldier of Fortune 2: Double Helix"sv,
          L"Medal of Honor: Allied Assault - Spearhead"sv,
          L"Wolfenstein: Enemy Territory"sv,
          L"Star Trek: Elite Force 2"sv,
          L"Medal of Honor: Allied Assault - Breakthrough"sv,
          L"Star Wars Jedi Knight: Jedi Academy"sv,
          L"Call of Duty"sv,
          L"Call of Duty: United Offensive"sv,
          L"Medal of Honor: Pacific Assault"sv,
          L"Iron Grip: Warlord"sv,
          L"Dark Salvation"sv,
          L"Quake Live"sv,
          L"Space Trader: Merchant Marine"sv,
  }};

  struct default_view final : win32::window_ref
  {
    inline static auto first_time = true;

    win32::static_control heading;//"Welcome to Siege Studio."
    win32::static_control logo;

    win32::gdi::icon logo_icon;

    //      std::string url = "https://github.com/open-siege/open-siege/wiki/" + extension;
    //"This particular file is not yet supported by Siege Studio.\nThough, you can still read about it on our wiki.\nClick the link below to find out more."
    default_view(win32::hwnd_t self, const CREATESTRUCTW& params) : win32::window_ref(self)
    {
      logo_icon.reset((HICON)::LoadImageW(params.hInstance, L"AppIcon", IMAGE_ICON, 0, 0, 0));
    }

    auto wm_create()
    {
      win32::window_factory factory(ref());

      heading = *factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE });

      ::SetWindowTextW(heading, L"Welcome to Siege Studio");

      logo = *factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE | SS_ICON | SS_REALSIZECONTROL });

      ::SendMessageW(logo, STM_SETIMAGE, IMAGE_ICON, (LPARAM)logo_icon.get());

      wm_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });

      return 0;
    }

    auto wm_size(std::size_t type, SIZE client_size)
    {
      if (type == SIZE_MAXIMIZED || type == SIZE_RESTORED)
      {
        auto top_size = SIZE{ .cx = client_size.cx, .cy = client_size.cy / 2 };
        auto bottom_size = SIZE{ .cx = client_size.cy / 2, .cy = client_size.cy / 2 };

        heading.SetWindowPos(POINT{});
        heading.SetWindowPos(top_size);

        logo.SetWindowPos(POINT{ .x = (client_size.cx - bottom_size.cx) / 2, .y = (client_size.cy - top_size.cy) / 2 });
        logo.SetWindowPos(bottom_size);
      }

      return 0;
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        win32::apply_theme(heading);
        win32::apply_theme(logo);
        win32::apply_theme(*this);

        return 0;
      }

      return std::nullopt;
    }

    auto wm_command()
    {
      // wxLaunchDefaultBrowser(url);
    }
  };
}// namespace siege::views

#endif