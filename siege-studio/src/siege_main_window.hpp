#ifndef SIEGE_MAIN_WINDOW_HPP
#define SIEGE_MAIN_WINDOW_HPP

#include <siege/platform/win/desktop/win32_controls.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include "siege-plugin.hpp"

struct siege_main_window : win32::window
{
	using window::window;
	win32::tab_control tab_control;
	std::list<siege::siege_plugin> loaded_modules;
	
	std::set<std::wstring> extensions;
	std::set<std::wstring> categories;

	std::size_t open_id = 0u;

	siege_main_window(win32::hwnd_t self, const CREATESTRUCTW& params) : win32::window(self), tab_control(nullptr)
	{
		open_id = RegisterWindowMessageW(L"COMMAND_OPEN");
		std::wstring full_app_path(256, '\0');

		GetModuleFileName(params.hInstance, full_app_path.data(), full_app_path.size());

		std::filesystem::path app_path = std::filesystem::path(full_app_path).parent_path();

		for (auto const& dir_entry : std::filesystem::directory_iterator{app_path}) 
		{
			if (dir_entry.path().extension() == ".dll")
			{
				try
				{
					loaded_modules.emplace_back(dir_entry.path());			
				}
				catch(...)
				{
				}
			}
		}

		for (auto& module : loaded_modules)
		{
			auto module_exts = module.GetSupportedExtensions();
			std::copy(module_exts.begin(), module_exts.end(), std::inserter(extensions, extensions.begin()));

			auto category_exts = module.GetSupportedFormatCategories(LOCALE_USER_DEFAULT);
			std::copy(category_exts.begin(), category_exts.end(), std::inserter(categories, categories.begin()));
		}
	}

	auto on_create(const win32::create_message&)
	{
		auto parent_size = this->GetClientRect();

		assert(parent_size);

		auto mfcModule = GetModuleHandleW(L"siege-win-mfc.dll");

		win32::window_factory factory;

		auto left_size = (parent_size->right - parent_size->left) / 9;
		auto dir_list = factory.CreateWindowExW(CREATESTRUCTW {
						.hwndParent = *this,
						.cy = parent_size->bottom - parent_size->top,
						.cx = left_size,
						.y = 0,
						.x = 0,
						.style = WS_CHILD  | WS_VISIBLE, 
						.lpszClass = L"MFC::CMFCShellTreeCtrl"
					});
		assert(dir_list);

		auto tab_control_instance = factory.CreateWindowExW<win32::tab_control>(CREATESTRUCTW {
						.hwndParent = *this,
						.cy = parent_size->bottom - parent_size->top,
						.cx = parent_size->right  - parent_size->left - left_size - 10,
						.y = 0,
						.x = left_size + 10,
						.style = WS_CHILD | WS_VISIBLE | TCS_MULTILINE | TCS_RIGHTJUSTIFY, 
						.lpszClass = win32::tab_control::class_name
					});

		assert(tab_control_instance);
		tab_control = std::move(*tab_control_instance);

		auto children = std::array<win32::window_ref, 2>{win32::window_ref(*dir_list), tab_control.ref()};
        win32::StackChildren(*this->GetClientSize(), children, win32::StackDirection::Horizontal);

		parent_size = tab_control.GetClientRect();

		for (auto& plugin : loaded_modules)
		{
			/*for (auto& window : plugin.data->available_classes)
			{
				auto child = win32::CreateWindowExW(win32::window_params<RECT>{
					.parent = self,
					.class_name = window.first.c_str(),
					.class_module = plugin.module.get(),
					.position = *parent_size
				});

				assert(child);

				win32::tab_control::InsertItem(*tab_control_instance, index, TCITEMW {
						.mask = TCIF_TEXT | TCIF_PARAM,
						.pszText = const_cast<wchar_t*>(window.first.c_str()),
						.lParam = win32::lparam_t(*child)
					});

				SendMessageW(*tab_control_instance, TCM_ADJUSTRECT, FALSE, std::bit_cast<win32::lparam_t>(&parent_size));

				SetWindowLongPtrW(*child, GWLP_ID, index);
				index++;
			}

			SendMessageW(*tab_control_instance, TCM_SETCURSEL, 0, 0);
			NMHDR notification{.hwndFrom = *tab_control_instance, .code = TCN_SELCHANGE};
			SendMessageW(*this, WM_NOTIFY, 0, std::bit_cast<LPARAM>(&notification));*/
		}

		tab_control.InsertItem(0, TCITEMW {
						.mask = TCIF_TEXT,
						.pszText = const_cast<wchar_t*>(L"+"),
					});

		
		return 0;
	}

	auto on_size(win32::size_message sized)
	{
		/*auto tab = ::FindWindowExW(self, nullptr, win32::tab_control::class_name, nullptr);
		assert(tab);

		win32::SetWindowPos(tab, POINT{}, sized.client_size);
			
		RECT temp {.left = 0, .top = 0, .right = sized.client_size.cx, .bottom = sized.client_size.cy };

		SendMessageW(tab, TCM_ADJUSTRECT, FALSE, std::bit_cast<win32::lparam_t>(&temp));

		for (auto i = 0; i < win32::tab_control::GetItemCount(tab); ++i)
		{
			auto tab_item = win32::tab_control::GetItem(tab, i);
			assert(win32::SetWindowPos(win32::hwnd_t(tab_item->lParam), temp));		
		}*/

		return std::nullopt;
	}

	auto on_notify(win32::notify_message notification)
	{
		auto [sender, id, code] = notification;

		
		if (code == TCN_SELCHANGING)
		{
			auto current_index = SendMessageW(sender, TCM_GETCURSEL, 0, 0);
			auto tab_item = win32::tab_control(sender).GetItem(current_index);

			::ShowWindow(win32::hwnd_t(tab_item->lParam), SW_HIDE);
		}
		else if (code == TCN_SELCHANGE)
		{
			auto current_index = SendMessageW(sender, TCM_GETCURSEL, 0, 0);
			auto tab_item = win32::tab_control(sender).GetItem(current_index);
			

			if (tab_item->lParam == 0)
			{
				return 0;
			}

			auto temp_window = win32::window_ref(win32::hwnd_t(tab_item->lParam));

			temp_window.SetWindowPos(HWND_TOP);

			auto temp = temp_window.GetClientRect();

			::MapWindowPoints(sender, *win32::window_ref(sender).GetParent(), std::bit_cast<POINT*>(&temp), 2);

			SendMessageW(sender, TCM_ADJUSTRECT, FALSE, std::bit_cast<win32::lparam_t>(&temp.value()));

			temp_window.SetWindowPos(*temp);
			
			ShowWindow(win32::hwnd_t(tab_item->lParam), SW_SHOW);
		}

		return 0;
	}

	std::optional<LRESULT> on_destroy(const win32::destroy_message& command) {	
				PostQuitMessage(0);
				return 0;
	}

	std::optional<LRESULT> on_command(const win32::command_message& command) {					
				if (command.notification_code == 0 && command.identifier == open_id)
				{
					auto dialog = win32::com::CreateFileOpenDialog();

					if (dialog)
					{
						struct filter : COMDLG_FILTERSPEC
						{
							std::wstring name;
							std::wstring spec;

							filter(std::wstring name, std::wstring spec) noexcept : name(std::move(name)), spec(std::move(spec)) 
							{
								this->pszName = this->name.c_str();
								this->pszSpec = this->spec.c_str();
							}
						};

						std::vector<filter> temp;

						temp.reserve(extensions.size());

						for (auto& extension : extensions)
						{
							temp.emplace_back(L"", L"*" + extension);
						}

						std::vector<COMDLG_FILTERSPEC> filetypes(temp.begin(), temp.end());
						dialog.value()->SetFileTypes(filetypes.size(), filetypes.data());
						auto result = dialog.value()->Show(nullptr);

						if (result == S_OK)
						{
							auto item = dialog.value()->GetResult();

							if (item)
							{
								auto path = item.value()->GetFileSysPath();

								IStream* stream = nullptr;

								if (SHCreateStreamOnFileEx(path->c_str(), STGM_READ, 0, FALSE, nullptr, &stream) == S_OK)
								{
									auto plugin = std::find_if(loaded_modules.begin(), loaded_modules.end(), [&](auto& module) {
										return 	module.IsStreamSupported(*stream);
									});			

									if (plugin != loaded_modules.end())
									{
										auto class_name = plugin->GetWindowClassForStream(*stream);

										auto parent_size = this->GetClientRect();

										auto child = plugin->CreateWindowExW(win32::window_params<RECT>{
											.parent = *this,
											.class_name = class_name.c_str(),
											.position = *parent_size
										});

										assert(child);

										::STATSTG info{};

										stream->Stat(&info, STATFLAG::STATFLAG_NONAME);
										
										auto handle = ::CreateFileW(path->c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

										auto mapping = ::CreateFileMapping(handle, nullptr, PAGE_READONLY, 0, 0, nullptr);

										COPYDATASTRUCT data{
											.cbData = DWORD(info.cbSize.QuadPart),
											.lpData = ::MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, std::size_t(info.cbSize.QuadPart))
										};

										assert(stream->Release() == 0);
	
										SendMessageW(*child, WM_COPYDATA, win32::wparam_t(win32::hwnd_t(*this)), win32::lparam_t(&data));


										::UnmapViewOfFile(data.lpData);
										::CloseHandle(mapping);
										::CloseHandle(handle);

										auto index = tab_control.GetItemCount() - 1;

										tab_control.InsertItem(index, TCITEMW {
												.mask = TCIF_TEXT | TCIF_PARAM,
												.pszText = const_cast<wchar_t*>(path->filename().c_str()),
												.lParam = win32::lparam_t(child->get())
											});

										SendMessageW(tab_control, TCM_ADJUSTRECT, FALSE, std::bit_cast<win32::lparam_t>(&parent_size));

										SetWindowLongPtrW(*child, GWLP_ID, index);
									}
								}
							}
						}
					}
				}

				if (command.identifier == 101)
				{

				}
				else if (command.identifier == 100)
				{
					DestroyWindow(*this);
					return 0;
				}

				return std::nullopt;
	}
};
#endif