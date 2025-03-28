#ifndef BITMAPWINDOW_HPP
#define BITMAPWINDOW_HPP

#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/shell.hpp>
#include <siege/platform/win/wic.hpp>
#include <siege/platform/win/threading.hpp>
#include <siege/platform/win/file.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/storage_module.hpp>
#undef NDEBUG
#include <cassert>
#include <sstream>
#include <vector>
#include <memory>
#include <istream>
#include <spanstream>
#include "bmp_controller.hpp"
#include "pal_controller.hpp"

namespace siege::views
{
  using namespace win32::wic;

  // TODO add a right click menu to open the palette in a new tab
  // TODO update icons of list view to be radio buttons instead of checkboxes
  struct bmp_view : win32::window_ref
  {
    bmp_controller controller;

    win32::list_view palette_list;
    win32::tool_bar bitmap_actions;
    win32::image_list bitmap_actions_icons;
    win32::image_list ratio_button_icons;

    std::size_t current_frame_index = 0;
    win32::wic::bitmap empty{ 1, 1, win32::wic::pixel_format::bgr_32bpp };
    win32::wic::bitmap_source* current_frame = &empty;
    win32::popup_menu frame_selection_menu;
    win32::popup_menu image_export_menu;
    win32::popup_menu export_single_menu;
    win32::popup_menu export_multiple_menu;
    win32::popup_menu remap_menu;
    win32::popup_menu remap_dither_type_menu;

    win32::gdi::bitmap preview_bitmap;
    WICRect viewport;
    WICRect previous_viewport;
    float scale = 0;
    bool is_panning = false;
    std::optional<WICBitmapDitherType> dither_type = WICBitmapDitherType::WICBitmapDitherTypeSolid;
    std::optional<POINTS> last_mouse_position = std::nullopt;
    std::function<bool()> pan_timer;
    std::function<bool()> animation_timer;

    win32::static_control static_image;
    std::list<platform::storage_module> loaded_modules;

    win32::local_atom pbmp_id = win32::local_atom(L".pbm");
    win32::local_atom bmp_id = win32::local_atom(L".bmp");
    win32::local_atom jpg_id = win32::local_atom(L".jpg");
    win32::local_atom png_id = win32::local_atom(L".png");
    win32::local_atom gif_id = win32::local_atom(L".gif");
    win32::local_atom tiff_id = win32::local_atom(L".tif");
    win32::local_atom dds_id = win32::local_atom(L".dds");

    win32::local_atom single_pba_id = win32::local_atom(L"single.pba");
    win32::local_atom single_gif_id = win32::local_atom(L"single.gif");
    win32::local_atom single_tiff_id = win32::local_atom(L"single.tif");
    win32::local_atom multiple_pbmp_id = win32::local_atom(L"multiple.pbm");
    win32::local_atom multiple_gif_id = win32::local_atom(L"multiple.pba");
    win32::local_atom multiple_tiff_id = win32::local_atom(L"multiple.tif");
    win32::local_atom multiple_jpg_id = win32::local_atom(L"multiple.jpg");
    win32::local_atom multiple_bmp_id = win32::local_atom(L"multiple.bmp");
    win32::local_atom multiple_png_id = win32::local_atom(L"multiple.png");
    win32::local_atom multiple_dds_id = win32::local_atom(L"multiple.dds");

    std::map<ATOM, std::function<std::unique_ptr<bitmap_encoder>(std::filesystem::path)>> encoder_creators;

    bmp_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
    {
      auto bmp_creator = [](auto path) {
        return std::unique_ptr<bitmap_encoder>(new bmp_bitmap_encoder(path));
      };
      encoder_creators.emplace(bmp_id, bmp_creator);
      encoder_creators.emplace(multiple_bmp_id, bmp_creator);

      auto jpg_creator = [](auto path) {
        return std::unique_ptr<bitmap_encoder>(new jpg_bitmap_encoder(path));
      };
      encoder_creators.emplace(jpg_id, jpg_creator);
      encoder_creators.emplace(multiple_jpg_id, jpg_creator);

      auto png_creator = [](auto path) {
        return std::unique_ptr<bitmap_encoder>(new png_bitmap_encoder(path));
      };
      encoder_creators.emplace(png_id, png_creator);
      encoder_creators.emplace(multiple_png_id, png_creator);

      auto tiff_creator = [](auto path) {
        return std::unique_ptr<bitmap_encoder>(new tiff_bitmap_encoder(path));
      };
      encoder_creators.emplace(tiff_id, tiff_creator);
      encoder_creators.emplace(single_tiff_id, tiff_creator);
      encoder_creators.emplace(multiple_tiff_id, tiff_creator);

      auto gif_creator = [](auto path) {
        return std::unique_ptr<bitmap_encoder>(new gif_bitmap_encoder(path));
      };
      encoder_creators.emplace(gif_id, gif_creator);
      encoder_creators.emplace(single_gif_id, gif_creator);
      encoder_creators.emplace(multiple_gif_id, gif_creator);

      auto dds_creator = [](auto path) {
        return std::unique_ptr<bitmap_encoder>(new dds_bitmap_encoder(path));
      };
      encoder_creators.emplace(dds_id, dds_creator);
      encoder_creators.emplace(multiple_dds_id, dds_creator);
    }

    ~bmp_view()
    {
      if (pan_timer)
      {
        pan_timer();
      }

      if (animation_timer)
      {
        animation_timer();
      }
    }

    auto wm_create()
    {
      std::filesystem::path app_path = std::filesystem::path(win32::module_ref::current_application().GetModuleFileName()).parent_path();
      loaded_modules = platform::storage_module::load_modules(std::filesystem::path(app_path));

      bitmap_actions = *win32::CreateWindowExW<win32::tool_bar>(::CREATESTRUCTW{
        .hwndParent = *this,
        .style = WS_VISIBLE | WS_CHILD | TBSTYLE_FLAT | TBSTYLE_WRAPABLE | BTNS_CHECKGROUP | WS_CLIPSIBLINGS });

      bitmap_actions.InsertButton(-1, { .iBitmap = 0, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Zoom In" });

      bitmap_actions.InsertButton(-1, { .iBitmap = 1, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Zoom Out" });

      bitmap_actions.InsertButton(-1, { .iBitmap = 2, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECK, .iString = (INT_PTR)L"Pan" });

      bitmap_actions.InsertButton(-1, { .fsStyle = BTNS_SEP });

      bitmap_actions.InsertButton(-1, { .iBitmap = 3, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN | BTNS_WHOLEDROPDOWN, .iString = (INT_PTR)L"Frame" });

      bitmap_actions.InsertButton(-1, { .fsStyle = BTNS_SEP });
      bitmap_actions.InsertButton(-1, { .iBitmap = 4, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Save" });

      bitmap_actions.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS);
      bitmap_actions.bind_nm_click(std::bind_front(&bmp_view::bitmap_actions_nm_click, this));
      bitmap_actions.bind_tbn_dropdown(std::bind_front(&bmp_view::bitmap_actions_tbn_dropdown, this));

      std::wstring temp = L"menu.pal";

      palette_list = [&] {
        auto palette_list = *win32::CreateWindowExW<win32::list_view>(::CREATESTRUCTW{
          .hwndParent = *this,
          .style = WS_VISIBLE | WS_CHILD | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER | WS_CLIPSIBLINGS,
          .lpszName = L"Palettes" });

        palette_list.SetView(win32::list_view::view_type::details_view);
        palette_list.EnableGroupView(true);
        palette_list.SetTileViewInfo(LVTILEVIEWINFO{
          .dwFlags = LVTVIF_FIXEDWIDTH,
          .sizeTile = SIZE{ .cx = this->GetClientSize()->cx, .cy = 50 },
        });

        palette_list.SetExtendedListViewStyle(0,
          LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_AUTOSIZECOLUMNS | LVS_EX_DOUBLEBUFFER);

        palette_list.win32::list_view::InsertColumn(-1, LVCOLUMNW{ .pszText = const_cast<wchar_t*>(L"Available Palettes") });

        auto header = palette_list.GetHeader();

        auto style = header.GetWindowStyle();
        header.SetWindowStyle(style | HDS_FLAT | HDS_NOSIZING | HDS_FILTERBAR);
        return palette_list;
      }();

      palette_list.bind_lvn_item_changed(std::bind_front(&bmp_view::palette_list_item_changed, this));
      palette_list.bind_nm_rclick(std::bind_front(&bmp_view::palette_list_nm_rclick, this));

      static_image = *win32::CreateWindowExW<win32::static_control>(::CREATESTRUCTW{
        .hwndParent = *this,
        .style = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | SS_BITMAP | SS_REALSIZEIMAGE | SS_CENTERIMAGE });

      image_export_menu.AppendMenuW(MF_OWNERDRAW, 1, L"Save As");
      image_export_menu.AppendMenuW(MF_OWNERDRAW | MF_POPUP, (UINT_PTR)export_single_menu.get(), L"Export Current Frame");
      image_export_menu.AppendMenuW(MF_OWNERDRAW | MF_POPUP | MF_DISABLED, (UINT_PTR)export_multiple_menu.get(), L"Export All Frames");

      export_single_menu.AppendMenuW(MF_OWNERDRAW, pbmp_id, L"Export As Phoenix BMP/PBM");
      export_single_menu.AppendMenuW(MF_OWNERDRAW, bmp_id, L"Export As Microsoft BMP");
      export_single_menu.AppendMenuW(MF_OWNERDRAW, png_id, L"Export As PNG");
      export_single_menu.AppendMenuW(MF_OWNERDRAW, jpg_id, L"Export As JPG");
      export_single_menu.AppendMenuW(MF_OWNERDRAW, gif_id, L"Export As GIF");
      export_single_menu.AppendMenuW(MF_OWNERDRAW, tiff_id, L"Export As TIFF");
      export_single_menu.AppendMenuW(MF_OWNERDRAW, dds_id, L"Export As DDS");

      export_multiple_menu.AppendMenuW(MF_OWNERDRAW, single_pba_id, L"Export As Single Phoenix BMP Array (PBA)");
      export_multiple_menu.AppendMenuW(MF_OWNERDRAW, single_gif_id, L"Export As Single GIF");
      export_multiple_menu.AppendMenuW(MF_OWNERDRAW, single_tiff_id, L"Export As Single TIFF");
      export_multiple_menu.AppendMenuW(MF_OWNERDRAW, multiple_pbmp_id, L"Export As Multiple Phoenix BMPs");
      export_multiple_menu.AppendMenuW(MF_OWNERDRAW, multiple_gif_id, L"Export As Multiple GIFs");
      export_multiple_menu.AppendMenuW(MF_OWNERDRAW, multiple_tiff_id, L"Export As Multiple TIFFs");
      export_multiple_menu.AppendMenuW(MF_OWNERDRAW, multiple_bmp_id, L"Export As Multiple Microsoft BMPs");
      export_multiple_menu.AppendMenuW(MF_OWNERDRAW, multiple_png_id, L"Export As Multiple PNGs");
      export_multiple_menu.AppendMenuW(MF_OWNERDRAW, multiple_dds_id, L"Export As Multiple DDSs");


      remap_menu.AppendMenuW(MF_OWNERDRAW | MF_POPUP, (UINT_PTR)remap_dither_type_menu.get(), L"Color Remapping");
      remap_menu.AppendMenuW(MF_OWNERDRAW, 1, L"No Color Remapping");

      remap_dither_type_menu.AppendMenuW(MF_OWNERDRAW, 2 + WICBitmapDitherTypeErrorDiffusion, L"With Dithering");
      remap_dither_type_menu.AppendMenuW(MF_OWNERDRAW, 2 + WICBitmapDitherTypeSolid, L"Without Dithering");

      wm_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });

      return 0;
    }

    void set_is_panning(bool is_panning)
    {
      this->is_panning = is_panning;

      auto state = SendMessageW(bitmap_actions, TB_GETSTATE, 2, 0);

      if (state != -1)
      {
        SendMessageW(bitmap_actions, TB_SETSTATE, 2, is_panning ? MAKEWORD(state | TBSTATE_CHECKED, 0) : MAKEWORD(state & ~TBSTATE_CHECKED, 0));
      }

      if (is_panning && !pan_timer)
      {
        pan_timer = win32::SetTimer(this->ref(), 50, [this](auto, auto, auto, auto) {
          resize_preview(true);
        });
      }
      else if (!is_panning && pan_timer)
      {
        pan_timer();
        pan_timer = nullptr;
      }
    }

    LRESULT bitmap_actions_tbn_dropdown(win32::tool_bar, const NMTOOLBAR& message)
    {
      POINT mouse_pos;
      if (message.iItem == 4 && ::GetCursorPos(&mouse_pos))
      {
        auto selection = ::TrackPopupMenu(frame_selection_menu, TPM_CENTERALIGN | TPM_RETURNCMD, mouse_pos.x, mouse_pos.y, 0, *this, nullptr);

        if (selection)
        {
          auto frame_count = controller.get_frame_count();
          if (selection - 1 == frame_count)
          {
            if (animation_timer)
            {
              ::CheckMenuItem(frame_selection_menu, selection, MF_BYCOMMAND | MF_UNCHECKED);
              animation_timer();
              animation_timer = nullptr;
            }
            else
            {
              ::CheckMenuItem(frame_selection_menu, selection, MF_BYCOMMAND | MF_CHECKED);
              animation_timer = win32::SetTimer(ref(), 100, [this, frame_count](auto, auto, auto, auto) {
                current_frame_index++;

                if (current_frame_index >= frame_count)
                {
                  current_frame_index = 0;
                }

                previous_viewport = WICRect{};
                current_frame = &controller.get_frame(current_frame_index);
                resize_preview(true);
              });
            }
          }
          else
          {
            previous_viewport = WICRect{};
            current_frame_index = selection - 1;
            auto frame_size = controller.get_size(current_frame_index);
            current_frame = &controller.get_frame(current_frame_index);
            resize_preview(true);
          }
        }
      }
      else if (message.iItem == 6 && ::GetCursorPos(&mouse_pos))
      {
        auto selection = ::TrackPopupMenu(image_export_menu, TPM_CENTERALIGN | TPM_RETURNCMD, mouse_pos.x, mouse_pos.y, 0, *this, nullptr);

        auto creator = encoder_creators.find(selection);

        if (creator != encoder_creators.end())
        {
          auto dialog = win32::com::CreateFileSaveDialog();

          if (dialog)
          {
            auto save_dialog = *dialog;

            std::wstring temp(20, '\0');
            temp.resize(GetAtomNameW(creator->first, temp.data(), temp.size()));

            std::filesystem::path filename(temp);
            auto extension = filename.has_extension() ? filename.extension() : filename.filename();

            auto extension_str = extension.wstring().substr(1);

            // TODO set list of available extensions
            // TODO default extension is not applied. Perhaps because there is no list of extensions
            save_dialog->SetDefaultExtension(extension_str.c_str());

            auto result = save_dialog->Show(nullptr);

            if (result == S_OK)
            {
              auto selection = save_dialog.GetResult();

              if (selection)
              {
                auto path = selection->GetFileSysPath();

                if (path)
                {
                  if (filename.stem() == "single")
                  {
                    filename = path->replace_extension(extension);
                    std::ofstream stream(filename, std::ios::trunc);
                    auto encoder = creator->second(filename);

                    for (auto i = 0u; i < controller.get_frame_count(); ++i)
                    {
                      auto frame_size = controller.get_size(i);
                      auto ref = controller.get_frame(i);
                      auto frame = encoder->create_new_frame();
                      frame.write_source(ref);
                      frame.commit();
                    }

                    encoder->commit();
                  }
                  else if (filename.stem() == "multiple")
                  {
                    filename = path->replace_extension(extension);

                    auto stem = filename.stem().wstring();
                    for (auto i = 0u; i < controller.get_frame_count(); ++i)
                    {
                      filename.replace_filename(stem + std::to_wstring(i + 1)).replace_extension(extension);
                      std::ofstream stream(filename, std::ios::trunc);
                      auto encoder = creator->second(filename);


                      auto frame = encoder->create_new_frame();

                      auto ref = controller.get_frame(i);
                      frame.write_source(ref);
                      frame.commit();
                      encoder->commit();
                    }
                  }
                  else
                  {
                    filename = path->replace_extension(extension);
                    std::ofstream stream(filename, std::ios::trunc);
                    auto encoder = creator->second(filename);
                    auto frame = encoder->create_new_frame();
                    frame.write_source(*current_frame);
                    frame.commit();
                    encoder->commit();
                  }

                  win32::launch_shell_process(filename.parent_path());
                }
              }
            }
          }
        }
      }

      return TBDDRET_NODEFAULT;
    }

    BOOL bitmap_actions_nm_click(win32::tool_bar, const NMMOUSE& message)
    {
      if (message.dwItemSpec == 0)
      {
        this->viewport.Width -= 10;
        this->viewport.Height -= 10;

        resize_preview(true);
        return TRUE;
      }
      else if (message.dwItemSpec == 1)
      {
        this->viewport.Width += 10;
        this->viewport.Height += 10;

        resize_preview(true);
        return TRUE;
      }
      else if (message.dwItemSpec == 2)
      {
        set_is_panning(!is_panning);
        return TRUE;
      }
      return FALSE;
    }

    void recreate_image_list(std::optional<SIZE> possible_size)
    {

      SIZE icon_size = possible_size.or_else([this] {
                                      return bitmap_actions_icons.GetIconSize();
                                    })
                         .or_else([] {
                           return std::make_optional(SIZE{
                             .cx = win32::get_system_metrics(SM_CXSIZE),
                             .cy = win32::get_system_metrics(SM_CYSIZE) });
                         })
                         .value();

      if (bitmap_actions_icons)
      {
        bitmap_actions_icons.reset();
      }

      std::vector icons{
        win32::segoe_fluent_icons::zoom_in,
        win32::segoe_fluent_icons::zoom_out,
        win32::segoe_fluent_icons::pan_mode,
        win32::segoe_fluent_icons::picture,
        win32::segoe_fluent_icons::save,
      };

      bitmap_actions_icons = win32::create_icon_list(icons, icon_size);

      std::vector radio_icons{
        win32::segoe_fluent_icons::radio_btn_off,
        win32::segoe_fluent_icons::radio_btn_on
      };

      ratio_button_icons = win32::create_icon_list(radio_icons, icon_size);
    }

    void palette_list_nm_rclick(win32::list_view sender, const NMITEMACTIVATE& info)
    {
      auto point = info.ptAction;

      if (ClientToScreen(palette_list, &point))
      {
        auto result = remap_menu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, point, ref());

        if (result == 1)
        {
          dither_type = std::nullopt;
        }
        else
        {
          dither_type = (WICBitmapDitherType)(result - 2);
        }
        ListView_SetCheckState(sender, info.iItem, TRUE);
      }
    }

    void palette_list_item_changed(win32::list_view sender, const NMLISTVIEW& info)
    {
      // Taken from ListView_GetCheckState
      BOOL is_checked = (BOOL)(UINT)(info.uNewState >> 12) - 1;
      if (is_checked)
      {
        auto* palette = (content::pal::palette*)info.lParam;

        if (palette)
        {
          std::vector<win32::color> colors;

          colors.reserve(palette->colours.size());

          for (auto& color : palette->colours)
          {
            colors.emplace_back(RGBTRIPLE{ .rgbtBlue = (BYTE)color.blue, .rgbtGreen = (BYTE)color.green, .rgbtRed = (BYTE)color.red });
          }

          auto wic_palette = win32::wic::palette(colors);
          previous_viewport = WICRect{};
          resize_preview(true, wic_palette);
        }


        for (auto i = 0; i < ListView_GetItemCount(sender); ++i)
        {
          if (i == info.iItem)
          {
            continue;
          }

          ListView_SetCheckState(sender, i, FALSE);
        }
      }
      else
      {
      }
    }

    auto wm_mouse_button_down(std::size_t wparam, POINTS mouse_position)
    {
      if (wparam & MK_MBUTTON || (is_panning && wparam & MK_LBUTTON) || (is_panning && wparam & MK_RBUTTON))
      {
        set_is_panning(!is_panning);
      }

      return 0;
    }

    auto wm_mouse_move(std::size_t wparam, POINTS mouse_position)
    {
      if (is_panning)
      {
        if (wparam & MK_LBUTTON || wparam & MK_MBUTTON || wparam & MK_RBUTTON)
        {
          set_is_panning(false);
          return 0;
        }

        if (last_mouse_position && mouse_position.x > last_mouse_position->x)
        {
          viewport.X += mouse_position.x - last_mouse_position->x;
        }
        else if (last_mouse_position && mouse_position.x < last_mouse_position->x)
        {
          viewport.X -= last_mouse_position->x - mouse_position.x;
        }

        if (last_mouse_position && mouse_position.y > last_mouse_position->y)
        {
          viewport.Y -= mouse_position.y - last_mouse_position->y;
        }
        else if (last_mouse_position && mouse_position.y < last_mouse_position->y)
        {
          viewport.Y += last_mouse_position->y - mouse_position.y;
        }
        last_mouse_position = mouse_position;
      }
      return 0;
    }

    std::optional<win32::lresult_t> wm_size(std::size_t type, SIZE client_size)
    {
      if (type == SIZE_MINIMIZED || type == SIZE_MAXHIDE || type == SIZE_MAXSHOW)
      {
        return std::nullopt;
      }

      auto top_size = SIZE{ .cx = client_size.cx, .cy = client_size.cy / 12 };

      recreate_image_list(bitmap_actions.GetIdealIconSize(SIZE{ .cx = client_size.cx / (LONG)bitmap_actions.ButtonCount(), .cy = top_size.cy }));
      SendMessageW(bitmap_actions, TB_SETIMAGELIST, 0, (LPARAM)bitmap_actions_icons.get());
      ListView_SetImageList(palette_list, ratio_button_icons, LVSIL_STATE);

      bitmap_actions.SetWindowPos(POINT{}, SWP_DEFERERASE | SWP_NOREDRAW);
      bitmap_actions.SetWindowPos(top_size, SWP_DEFERERASE);
      bitmap_actions.SetButtonSize(SIZE{ .cx = top_size.cx / (LONG)bitmap_actions.ButtonCount(), .cy = top_size.cy });

      auto left_size = SIZE{ .cx = (client_size.cx / 3) * 2, .cy = client_size.cy };
      auto right_size = SIZE{ .cx = client_size.cx - left_size.cx, .cy = client_size.cy - top_size.cy };

      const auto top_left_width = left_size.cx / 6;

      static_image.SetWindowPos(POINT{ .y = top_size.cy });
      static_image.SetWindowPos(SIZE{ .cx = left_size.cx, .cy = left_size.cy - top_size.cy });

      palette_list.SetWindowPos(POINT{ .x = left_size.cx, .y = top_size.cy });
      palette_list.SetWindowPos(right_size);
      palette_list.SetColumnWidth(0, right_size.cx);

      resize_preview();

      return 0;
    }

    void resize_preview(bool force = false, std::optional<win32::wic::palette> palette = std::nullopt)
    {
      if (scale != 0 && current_frame)
      {
        auto frame_size = controller.get_size(current_frame_index);

        if (this->viewport.Width > frame_size.width)
        {
          this->viewport.Width = frame_size.width;
        }

        if (this->viewport.Height > frame_size.height)
        {
          this->viewport.Height = frame_size.height;
        }

        if (this->viewport.Height <= 0)
        {
          this->viewport.Height += 10;
        }

        if (this->viewport.Width <= 0)
        {
          this->viewport.Width += 10;
        }

        if (this->viewport.Height <= 0)
        {
          this->viewport.Height += 10;
        }

        if (this->viewport.X < 0)
        {
          this->viewport.X = 0;
        }

        if (this->viewport.Y < 0)
        {
          this->viewport.Y = 0;
        }

        if (this->viewport.X > frame_size.width - viewport.Width)
        {
          this->viewport.X = frame_size.width - viewport.Width;
        }

        if (this->viewport.Y > frame_size.height - viewport.Height)
        {
          this->viewport.Y = frame_size.height - viewport.Height;
        }

        if (viewport.X + viewport.Width > frame_size.width)
        {
          viewport.Width = frame_size.width - viewport.X;
        }

        if (viewport.Y + viewport.Y > frame_size.height)
        {
          viewport.Height = frame_size.height - viewport.Y;
        }
        SIZE preview_size = { .cx = (LONG)(frame_size.width * scale), .cy = (LONG)(frame_size.height * scale) };

        auto existing_size = preview_bitmap.get_size();

        auto update_bitmap = [&] {
          if (palette)
          {
            auto bitmap = current_frame->handle().as<IWICBitmap>();

            if (auto raw_palette = palette->handle(); bitmap && !dither_type)
            {
              bitmap->SetPalette(raw_palette.get());
              current_frame
                ->clip(viewport)
                .scale((std::uint32_t)preview_size.cx, (std::uint32_t)preview_size.cy, WICBitmapInterpolationModeFant)
                .copy_pixels(preview_size.cx * sizeof(std::uint32_t), preview_bitmap.get_pixels_as_bytes());
            }
            else if (dither_type)
            {
              current_frame
                ->clip(viewport)
                .convert(win32::wic::bitmap_source::to_format{
                  .format = win32::wic::pixel_format::bgr_32bpp,
                  .palette_type = palette_type::WICBitmapPaletteTypeCustom })
                .convert(win32::wic::bitmap_source::to_format{
                  .format = win32::wic::pixel_format::indexed_8bpp,
                  .dither_type = *dither_type,
                  .palette = *palette,
                  .palette_type = palette_type::WICBitmapPaletteTypeCustom })
                .scale((std::uint32_t)preview_size.cx, (std::uint32_t)preview_size.cy, WICBitmapInterpolationModeFant)
                .copy_pixels(preview_size.cx * sizeof(std::uint32_t), preview_bitmap.get_pixels_as_bytes());
            }
          }
          else
          {
            current_frame
              ->clip(viewport)
              .scale((std::uint32_t)preview_size.cx, (std::uint32_t)preview_size.cy, WICBitmapInterpolationModeFant)
              .copy_pixels(preview_size.cx * sizeof(std::uint32_t), preview_bitmap.get_pixels_as_bytes());
          }

          auto pixels = preview_bitmap.get_pixels();

          // prevents bitmaps from being copied
          for (auto& pixel : pixels)
          {
            pixel.rgbReserved = 0;
          }

          if (static_image.GetBitmap() == preview_bitmap.get())
          {
            RedrawWindow(static_image, nullptr, nullptr, RDW_NOERASE | RDW_NOFRAME | RDW_INVALIDATE);
          }
          else
          {
            auto old_bitmap = static_image.SetImage(preview_bitmap.get());

            if (old_bitmap != preview_bitmap.get())
            {
              ::DeleteObject(old_bitmap);
            }
          }
        };

        if (!(existing_size.cx == preview_size.cx && existing_size.cy == preview_size.cy))
        {
          preview_bitmap = win32::gdi::bitmap(preview_size, win32::gdi::bitmap::skip_shared_handle);

          update_bitmap();
        }
        else if (force && std::memcmp(&viewport, &previous_viewport, sizeof(viewport)) != 0)
        {
          update_bitmap();
        }
        previous_viewport = viewport;
      }
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        recreate_image_list(std::nullopt);
        SendMessageW(bitmap_actions, TB_SETIMAGELIST, 0, (LPARAM)bitmap_actions_icons.get());
        ListView_SetImageList(palette_list, ratio_button_icons, LVSIL_STATE);

        return 0;
      }

      return std::nullopt;
    }

    auto wm_copy_data(win32::copy_data_message<char> message)
    {
      std::ispanstream stream(message.data);

      if (bmp_controller::is_bmp(stream))
      {
        static std::unordered_map<std::filesystem::path, siege::platform::storage_module::context_ptr> loaded_contexts;

        auto pending_load = controller.load_palettes_async(
          std::nullopt, [&](auto path) {
          
          siege::platform::storage_info info{
            .type = siege::platform::storage_info::file
          };
          info.info.path = path.c_str();

          auto resource_module = std::find_if(loaded_modules.begin(), loaded_modules.end(), [&](auto& module) {
            return module.stream_is_resource_reader(info);
          });

          if (resource_module == loaded_modules.end())
          {
            return std::set<std::filesystem::path>{};
          }

          auto context = resource_module->create_reader_context(info);

          if (!context)
          {
            return std::set<std::filesystem::path>{};
          }

          auto files = resource_module->get_reader_file_listing(*context.value(), path);

          if (!files)
          {
            return std::set<std::filesystem::path>{};
          }

          std::set<std::filesystem::path> results;

          for (auto& file : files.value())
          {
            auto is_pal = std::any_of(pal_controller::formats.begin(), pal_controller::formats.end(), [&](auto& ext) { return file.extension().c_str() == ext; });

            if (is_pal)
            {
              results.emplace(file);
            }
          }

          if (!results.empty())
          {
            loaded_contexts.emplace(path, std::move(context.value()));
          }

          return results; },

          [&](auto path) {
            auto storage_path = path.parent_path();

            auto existing = loaded_contexts.find(storage_path);

            if (existing != loaded_contexts.end())
            {
              // TODO create bound wrapper so that we don't have to find the module again
              auto result = loaded_modules.begin()->extract_file_contents(*existing->second.get(), path);

              if (result)
              {
                return std::move(result.value());
              }
            }

            return std::vector<char>{};
          });

        auto count = controller.load_bitmap(stream, pending_load);

        frame_selection_menu = win32::popup_menu();

        std::wstring temp;
        temp.reserve(10);

        std::vector<siege::platform::bitmap::size> frame_sizes;

        if (count > 1)
        {
          frame_sizes.reserve(count);
          for (auto i = 0u; i < count; ++i)
          {
            frame_sizes.emplace_back(controller.get_size(i));
          }

          if (std::all_of(frame_sizes.begin(), frame_sizes.end(), [first = frame_sizes[0]](auto& size) {
                return size == first;
              }))
          {
            frame_selection_menu.AppendMenuW(MF_OWNERDRAW, frame_sizes.size() + 1, L"Animate");
          }
        }

        for (auto i = 0u; i < count; ++i)
        {
          temp.assign(L"Frame ");
          temp.append(std::to_wstring(i + 1));
          frame_selection_menu.AppendMenuW(MF_OWNERDRAW, i + 1, temp.c_str());
        }

        if (count > 0)
        {
          auto size = static_image.GetClientSize();
          auto frame_size = controller.get_size(current_frame_index);
          current_frame = &controller.get_frame(current_frame_index);

          if (!(current_frame->get_pixel_format() == pixel_format::indexed_1bpp || current_frame->get_pixel_format() == pixel_format::indexed_2bpp || current_frame->get_pixel_format() != pixel_format::indexed_4bpp || current_frame->get_pixel_format() != pixel_format::indexed_8bpp))
          {
            dither_type = WICBitmapDitherType::WICBitmapDitherTypeErrorDiffusion;
          }

          viewport = WICRect();
          viewport.Width = (INT)frame_size.width;
          viewport.Height = (INT)frame_size.height;

          scale = size->cx > size->cy ? size->cx / (float)frame_size.width : size->cy / (float)frame_size.height;

          SIZE preview_size = { .cx = (LONG)(frame_size.width * scale), .cy = (LONG)(frame_size.height * scale) };

          resize_preview();

          win32::queue_user_work_item([this, pending_load]() {
                auto& palettes = pending_load.get();
                loaded_contexts.clear();
                auto selection = controller.get_selected_palette();
                auto p = 1u;
                std::vector<win32::list_view_group> groups;

                auto index = 0;
                auto selected_index = 0;

                for (auto& pal : palettes)
                {
                  std::vector<win32::list_view_item> items;
                  items.reserve(pal.children.size());

                  auto c = 1u;

                  for (auto& child : pal.children)
                  {
                    auto& child_item = items.emplace_back(win32::list_view_item(L"Palette " + std::to_wstring(c++)));

                    child_item.lParam = (LPARAM)&child;
                    child_item.mask = LVIF_TEXT | LVIF_PARAM;

                    if (selection.first->path == pal.path && selection.second == (c - 2))
                    {
                      selected_index = index;
                    }

                    index++;
                  }

                  auto& new_group = groups.emplace_back(pal.path.filename().wstring(), std::move(items));
                  new_group.state = LVGS_COLLAPSIBLE;
                }

                palette_list.InsertGroups(groups);

                ListView_SetCheckState(palette_list, selected_index, TRUE);
                ListView_SetItemState(palette_list, selected_index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED); });

          return TRUE;
        }

        return FALSE;
      }

      return FALSE;
    }
  };
}// namespace siege::views

#endif