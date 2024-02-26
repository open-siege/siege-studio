#ifndef BITMAPWINDOW_HPP
#define BITMAPWINDOW_HPP

#include <win32_controls.hpp>
#include <cassert>
#include <sstream>


struct bitmap_window
{
    constexpr static std::u8string_view formats = u8".jpg .jpeg .gif .png .tag .bmp .dib .pba .dmb .db0 .db1 .db2 .hba .hb0 .hb1 .hb2";

    win32::hwnd_t self;

    PROCESS_INFORMATION powershell;

    bitmap_window(win32::hwnd_t self, const CREATESTRUCTW&) : self(self)
	{
	}


    ~bitmap_window()
    {
        PostThreadMessageW(powershell.dwThreadId, WM_QUIT, 0, 0);

        for (auto i = 0; i < 10; ++i)
        {
            auto child = GetWindow(self, GW_CHILD);

            if (child == nullptr)
            {
                break;
            }

            Sleep(100);
        }

        CloseHandle(powershell.hThread);
     //   CloseHandle(powershell.hProcess);
        TerminateProcess(powershell.hProcess, 0);
    }

    static LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
    {
        CWPSTRUCT* data = std::bit_cast<CWPSTRUCT*>(lParam);
     
        if (data->message == WM_COMMAND)
        {
            DebugBreak();
        }

        if (nCode == HC_ACTION)
        {
            return 0;
        }

        return CallNextHookEx(nullptr, nCode, wParam, lParam);

    }

    auto on_create(const win32::create_message& info)
    {
        STARTUPINFOW startup_info = STARTUPINFOW{
            .cb = sizeof(STARTUPINFOW)
        
        };

        std::wstringstream command;

        command << '\"';
        command << "Add-Type -AssemblyName System.Windows.Forms;";
        command << "Add-Type -AssemblyName System.Drawing;";
        command << "$hwnd = " << std::size_t(self) << ";";
        command << "$style = " << (WS_CHILD | WS_VISIBLE) << ";";
        command << "$wm_command = " << WM_COMMAND << ";";
        command << "$bn_clicked = " << MAKEWPARAM(0, BN_CLICKED) << ";";
        command << "$signature = '";
        command << "[DllImport(\"\"\"user32.dll\"\"\")] public static extern IntPtr SetParent(IntPtr hwndChild, IntPtr hwndNewParent);";
        command << "[DllImport(\"\"\"user32.dll\"\"\")] public static extern IntPtr SetWindowLongPtrW(IntPtr hwnd, int index, IntPtr value);";
        command << "[DllImport(\"\"\"user32.dll\"\"\")] public static extern byte PostMessageW(IntPtr hwnd, uint msg, IntPtr wparam, IntPtr lparam);';";
        command << "$type = Add-Type -MemberDefinition $signature -Name user32 -Namespace win32 -Using System.Text -PassThru;";
        command <<  "$form = New-Object System.Windows.Forms.Form;";
        command << "$form.Text = 'Win Forms Form';";
        command << "$form.Size = New-Object System.Drawing.Size(300,200);";
        command << "$form.Location = New-Object System.Drawing.Point(0, 0);";

        command << "$okButton = New-Object System.Windows.Forms.Button;";
        command << "$okButton.Location = New-Object System.Drawing.Point(75,120);";
        command << "$okButton.Size = New-Object System.Drawing.Size(75,23);";
        command << "$okButton.Text = 'Panda Wuv';";
        command << "$okButton.DialogResult = [System.Windows.Forms.DialogResult]::OK;";
        command << "$okButton.add_Click({ param($sender, $eventArgs); $type::PostMessageW($hwnd, $wm_command, $bn_clicked, $sender.Handle); });";
        command << "$form.AcceptButton = $okButton;";
        command << "$form.Controls.Add($okButton);";
        
        command << "$type::SetWindowLongPtrW($form.Handle, -16, $style);";
        command << "$type::SetParent($form.Handle, $hwnd);";

        command << "[System.Windows.Forms.Application]::Run($form)";

        command << '\"';

      
        std::wstring args = L"powershell -WindowStyle hidden -Command " + command.str();

        assert(args.size() < 32766);

        OutputDebugStringW(args.c_str());
        if (!CreateProcessW(nullptr, args.data(), 
                    nullptr, 
                    nullptr, 
                    TRUE, 
                NORMAL_PRIORITY_CLASS,
                nullptr, 
            nullptr, &startup_info, &powershell))
        {
            DebugBreak();
        }




//        auto group_box = win32::CreateWindowExW(DLGITEMTEMPLATE{
//						.style = WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
 //                       .cx = 100,
  //                      .cy = 100
//						}, self, win32::button::class_name, L"Click me");


        /*

        auto group_box = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                        .cy = 100
						}, self, win32::button::class_name, L"Colour strategy");

        assert(group_box);


        auto do_nothing = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON
						}, self, win32::button::class_name, L"Do nothing");

        auto ideal_size = win32::button::GetIdealSize(*do_nothing);
        win32::SetWindowPos(*do_nothing, *ideal_size);

        auto remap = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
						}, self, win32::button::class_name, L"Remap");

        ideal_size = win32::button::GetIdealSize(*remap);
        win32::SetWindowPos(*remap, *ideal_size);

        auto remap_unique = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
						}, self, win32::button::class_name, L"Remap (only unique colours)");

        ideal_size = win32::button::GetIdealSize(*remap_unique);
        win32::SetWindowPos(*remap_unique, *ideal_size);

        auto strategy_toolbar = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | CCS_NOPARENTALIGN | TBSTYLE_LIST,   
                        .cy = 400,
						}, self, win32::tool_bar::class_name, L"Colour strategy");

        assert(strategy_toolbar);

        win32::tool_bar::SetExtendedStyle(*strategy_toolbar, win32::tool_bar::mixed_buttons | win32::tool_bar::draw_drop_down_arrows);
     
        std::array<TBBUTTON, 3> buttons{{
            TBBUTTON{.iBitmap = I_IMAGENONE, .idCommand = 0, .fsState = TBSTATE_ENABLED, 
                                .fsStyle = BTNS_CHECKGROUP | BTNS_SHOWTEXT, .iString = INT_PTR(L"Do nothing")},
            TBBUTTON{.iBitmap = I_IMAGENONE, .idCommand = 1, .fsState = TBSTATE_ENABLED, 
                                .fsStyle = BTNS_CHECKGROUP | BTNS_SHOWTEXT, .iString = INT_PTR(L"Remap")},
            TBBUTTON{.iBitmap = I_IMAGENONE, .idCommand = 2, .fsState = TBSTATE_ENABLED, 
                                .fsStyle = BTNS_CHECKGROUP | BTNS_SHOWTEXT, .iString = INT_PTR(L"Remap (only unique colours)")},
         }};

        assert(win32::tool_bar::AddButtons(*strategy_toolbar, buttons));

        auto palettes_tree = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_CHECKBOXES, 
                        .y = 2,
                        .cy = 300,
						}, self, win32::tree_view::class_name, L"Palettes");

        auto root_item = win32::tree_view::InsertItem(*palettes_tree, TVINSERTSTRUCTW{
            .hInsertAfter = TVI_ROOT,
            .itemex = {
                .pszText = const_cast<wchar_t*>(L"menu.pal"),
                .cChildren = 1
                }
            });

        win32::tree_view::InsertItem(*palettes_tree, TVINSERTSTRUCTW{
            .hParent = *root_item,
            .hInsertAfter = TVI_LAST,
            .itemex = {
                .pszText = const_cast<wchar_t*>(L"palette 1")
                }
            });

        win32::tree_view::InsertItem(*palettes_tree, TVINSERTSTRUCTW{
            .hParent = *root_item,
            .hInsertAfter = TVI_LAST,
            .itemex = {
                .pszText = const_cast<wchar_t*>(L"palette 2")
                }
            });

        win32::tree_view::InsertItem(*palettes_tree, TVINSERTSTRUCTW{
            .hParent = *root_item,
            .hInsertAfter = TVI_LAST,
            .itemex = {
                .pszText = const_cast<wchar_t*>(L"palette 3")
                }
            });

        win32::tree_view::InsertItem(*palettes_tree, TVINSERTSTRUCTW{
            .hParent = *root_item,
            .hInsertAfter = TVI_LAST,
            .itemex = {
                .pszText = const_cast<wchar_t*>(L"palette 4")
                }
            });

        root_item = win32::tree_view::InsertItem(*palettes_tree, TVINSERTSTRUCTW{
            .hInsertAfter = TVI_ROOT,
            .itemex = {
                .pszText = const_cast<wchar_t*>(L"test.dpl")
                }
            });

        root_item = win32::tree_view::InsertItem(*palettes_tree, TVINSERTSTRUCTW{
            .hInsertAfter = TVI_ROOT,
            .itemex = {
                .pszText = const_cast<wchar_t*>(L"ui.ppl")
                }
            });

        root_item = win32::tree_view::InsertItem(*palettes_tree, TVINSERTSTRUCTW{
            .hInsertAfter = TVI_ROOT,
            .itemex = {
                .pszText = const_cast<wchar_t*>(L"other.pal")
                }
            });

        // TODO add example palette file names as root items and then palette names as children

        auto palettes_list = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD,
                        .y = 3,
                        .cy = 300,
						}, self, win32::list_view::class_name, L"Palettes");

      //  // TODO add example palette file names as groups and then palette names as items

      //  auto static_image = win32::CreateWindowExW(DLGITEMTEMPLATE{
						//.style = WS_VISIBLE | WS_CHILD,
      //                  .y = 4,
      //                  .cy = 300,
						//}, self, win32::static_control::class_name, L"Image");

        auto children = std::array{*group_box, *strategy_toolbar, *palettes_tree};
        win32::StackChildren(*win32::GetClientSize(self), children);

        auto rect = win32::GetClientRect(*group_box);
        rect->top += 15;
        rect->left += 5;

        auto radios = std::array{*do_nothing, *remap, *remap_unique};
        win32::StackChildren(SIZE{.cx = rect->right, .cy = rect->bottom - 20}, radios, win32::StackDirection::Horizontal,
                POINT{.x = rect->left, .y = rect->top});
        */
        return 0;
    }

    auto on_command(win32::command_message)
    {
   //     DebugBreak();

        MessageBoxW(self, L"Panda Wuv is the best", L"Happy Dame, Happy Hame", 0);
        return std::nullopt;
    }

    auto on_size(win32::size_message sized)
	{
		win32::ForEachDirectChildWindow(self, [&](auto child) {

//			win32::SetWindowPos(child, sized.client_size);
		});
		return std::nullopt;
	}

    static bool is_bitmap(std::istream& raw_data)
    {
        return false;
    }
};

#endif