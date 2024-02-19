#include <win32_controls.hpp>
#include <bit>
#include <filesystem>

struct bitmap_window
{
    constexpr static std::u8string_view formats = u8".jpg .jpeg .gif .png .tag .bmp .dib .pba .dmb .db0 .db1 .db2 .hba .hb0 .hb1 .hb2";

    win32::hwnd_t self;

    bitmap_window(win32::hwnd_t self, const CREATESTRUCTW&) : self(self)
	{
	}

    auto on_create(const win32::create_message&)
    {
        short y_pos = 0;

        auto width = win32::GetClientRect(self)->right;

        auto button_instance = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
						.y = y_pos,
                        .cx = short(width),  
						.cy = 100,
						}, self, win32::button::class_name, L"Bitmap window");

        y_pos += 100;

        button_instance = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_SPLITBUTTON,
                        .y = y_pos,
						.cx = short(width),  
						.cy = 100,
						}, self, win32::button::class_name, L"Bitmap window");

        y_pos += 100;
                
        button_instance = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
                        .y = y_pos,
						.cx = short(width),  
						.cy = 100,
						}, self, win32::button::class_name, L"Bitmap window");

        y_pos += 100;

        button_instance = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,
                        .y = y_pos,
						.cx = short(width),  
						.cy = 100,
						}, self, win32::button::class_name, L"Bitmap window");

        y_pos += 100;

        button_instance = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_COMMANDLINK,
                        .y = y_pos,
						.cx = short(width),  
						.cy = 100,
						}, self, win32::button::class_name, L"Bitmap window");

        y_pos += 100;

        button_instance = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFCOMMANDLINK,
                        .y = y_pos,
						.cx = short(width),  
						.cy = 100,
						}, self, win32::button::class_name, L"Bitmap window");

        y_pos += 100;

        button_instance = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFSPLITBUTTON,
                        .y = y_pos,
						.cx = short(width),  
						.cy = 100,
						}, self, win32::button::class_name, L"Bitmap window");

        return 0;
    }

    auto on_size(win32::size_message sized)
	{
	//	win32::ForEachDirectChildWindow(self, [&](auto child) {
	//		win32::SetWindowPos(child, sized.client_size);
		//});

		return std::nullopt;
	}

    static bool is_bitmap(std::istream& raw_data)
    {
        return false;
    }
};

struct pal_window
{
    constexpr static std::u8string_view formats = u8".pal .ipl .ppl .dpl";

    win32::hwnd_t self;

    pal_window(win32::hwnd_t self, const CREATESTRUCTW&) : self(self)
	{
	}

    auto on_create(const win32::create_message&)
    {
        RECT parent_size{};

		if (GetClientRect(self, &parent_size))
		{
						
		}

        auto button_instance = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
						.cx = short(parent_size.right),  
						.cy = short(parent_size.bottom)     
						}, self, win32::button::class_name, L"Pal window");

		win32::SetWindowSubclass(*button_instance, [](win32::hwnd_t button, win32::message button_message) -> std::optional<LRESULT>
						{
							if (button_message.message == win32::command_message::id)
							{
								MessageBoxExW(GetParent(button), L"Hello world", L"Test Message", 0, 0);
								return 0;
							}

							return std::nullopt;
						});
        return 0;
    }

    auto on_size(win32::size_message sized)
	{
		win32::ForEachDirectChildWindow(self, [&](auto child) {
			win32::SetWindowPos(child, sized.client_size);
		});

		return std::nullopt;
	}

    static bool is_pal(std::istream& raw_data)
    {
        return false;
    }
};

struct pal_mapping_window
{
    constexpr static std::u8string_view formats = u8"palettes.settings.json";

    win32::hwnd_t self;
    pal_mapping_window(win32::hwnd_t self, const CREATESTRUCTW&) : self(self)
	{
	}

    auto on_create(const win32::create_message&)
    {
        auto button_instance = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
						.x = 0,       
						.y = 0,
						.cx = 100,  
						.cy = 100       
						}, self, win32::button::class_name, L"Pal Mapping window");

		win32::SetWindowSubclass(*button_instance, [](win32::hwnd_t button, win32::message button_message) -> std::optional<LRESULT>
						{
							if (button_message.message == win32::command_message::id)
							{
								MessageBoxExW(GetParent(button), L"Hello world", L"Test Message", 0, 0);
								return 0;
							}

							return std::nullopt;
						});
        return 0;
    }

    auto on_size(win32::size_message sized)
	{
		win32::ForEachDirectChildWindow(self, [&](auto child) {
			win32::SetWindowPos(child, sized.client_size);
		});

		return std::nullopt;
	}
};


struct is_supported_message
{
    enum class handle_type
    {
        path,
        file,
        view
    } type;

    int size;
};


struct bmp_module
{
    HINSTANCE module_instance;
    std::uint32_t is_supported_id;

    bmp_module(win32::hwnd_t self, const CREATESTRUCTW& args)
    {
        module_instance = args.hInstance;
        win32::RegisterClassExW<bitmap_window>(WNDCLASSEXW{
            .hInstance = module_instance
            });
        win32::RegisterClassExW<pal_window>(WNDCLASSEXW{
            .hInstance = module_instance
            });
        win32::RegisterClassExW<pal_mapping_window>(WNDCLASSEXW{
            .hInstance = module_instance
            });

        SetPropW(self, win32::type_name<bitmap_window>().c_str(), std::bit_cast<void*>(bitmap_window::formats.data()));
        SetPropW(self, win32::type_name<pal_window>().c_str(), std::bit_cast<void*>(pal_window::formats.data()));
        SetPropW(self, win32::type_name<pal_mapping_window>().c_str(), std::bit_cast<void*>(pal_mapping_window::formats.data()));

        SetPropW(self, L"All Images", std::bit_cast<void*>(bitmap_window::formats.data()));
        SetPropW(self, L"All Palettes", std::bit_cast<void*>(pal_window::formats.data()));

        is_supported_id = RegisterWindowMessageW(L"is_supported_message");
    }

    ~bmp_module()
    {
       win32::UnregisterClassW<bitmap_window>(module_instance);
       win32::UnregisterClassW<pal_window>(module_instance);
       win32::UnregisterClassW<pal_mapping_window>(module_instance);
    }

    std::optional<win32::lresult_t> on_is_supported(is_supported_message message)
    {
        if (message.type == is_supported_message::handle_type::path)
        {
            return FALSE;
        }
        else if (message.type == is_supported_message::handle_type::file)
        {
            return FALSE;
        }
        else if (message.type == is_supported_message::handle_type::view)
        {
            return FALSE;
        }
    }
};

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpvReserved )  // reserved
{

    if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
    {
        if (lpvReserved != nullptr)
        {
            return TRUE; // do not do cleanup if process termination scenario
        }

        static win32::hwnd_t info_instance = nullptr;

        static std::wstring module_file_name(255, '\0');
        GetModuleFileNameW(hinstDLL, module_file_name.data(), module_file_name.size());

       std::filesystem::path module_path(module_file_name.data());


       if (fdwReason == DLL_PROCESS_ATTACH)
       {
           win32::RegisterStaticClassExW<bmp_module>(WNDCLASSEXW{
                  .hInstance = hinstDLL,
                  .lpszClassName = module_path.stem().c_str()
           });

          info_instance = *win32::CreateWindowExW(CREATESTRUCTW{
                .hInstance = hinstDLL,
                .hwndParent = HWND_MESSAGE,
                .lpszClass = module_path.stem().c_str()
            });
        }
        else if (fdwReason == DLL_PROCESS_DETACH)
        {
            DestroyWindow(info_instance);
            UnregisterClassW(module_path.stem().c_str(), hinstDLL);
        }
    }

    return TRUE;
}