#ifndef SIEGE_MAIN_WINDOW_HPP
#define SIEGE_MAIN_WINDOW_HPP

#include <siege/platform/win/desktop/win32_common_controls.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/siege_module.hpp>

namespace siege::views
{
	struct siege_main_window : win32::window_ref
	{	
		win32::tree_view dir_list;
		win32::tab_control tab_control;

		std::list<siege::siege_module> loaded_modules;
		std::set<std::wstring> extensions;
		std::set<std::wstring> categories;

		std::size_t open_id = 0u;

		siege_main_window(win32::hwnd_t self, const CREATESTRUCTW& params) : win32::window_ref(self), tab_control(nullptr)
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
			auto mfcModule = GetModuleHandleW(L"siege-win-mfc.dll");

			win32::window_factory factory;

			dir_list = *factory.CreateWindowExW<win32::tree_view>(CREATESTRUCTW {
							.hwndParent = *this,
							.style = WS_CHILD  | WS_VISIBLE, 
							.lpszClass = L"MFC::CMFCShellTreeCtrl"
						});
			assert(dir_list);

			tab_control = *factory.CreateWindowExW<win32::tab_control>(CREATESTRUCTW {
							.hwndParent = *this,
							.style = WS_CHILD | WS_VISIBLE | TCS_MULTILINE | TCS_RIGHTJUSTIFY, 
							.lpszClass = win32::tab_control::class_name
						});

			tab_control.InsertItem(0, TCITEMW {
							.mask = TCIF_TEXT,
							.pszText = const_cast<wchar_t*>(L"+"),
						});

		
			return 0;
		}

		auto on_size(win32::size_message sized)
		{
			auto left_size = sized.client_size;
			left_size.cx = left_size.cx / 8;
			auto right_size = sized.client_size;
			right_size.cx -= left_size.cx;

			dir_list.SetWindowPos(POINT{});
			dir_list.SetWindowPos(left_size);

			tab_control.SetWindowPos(POINT{.x = sized.client_size.cx - right_size.cx});
			tab_control.SetWindowPos(right_size);

			auto tab_rect = tab_control.GetClientRect().and_then([&](auto value) { return tab_control.MapWindowPoints(*this, value);  }).value().second;

			SendMessageW(tab_control, TCM_ADJUSTRECT, FALSE, std::bit_cast<win32::lparam_t>(&tab_rect));

			for (auto i = 0; i < tab_control.GetItemCount(); ++i)
			{
				auto tab_item = tab_control.GetItem(i);

				if (tab_item->lParam)
				{
					assert(win32::window_ref(win32::hwnd_t(tab_item->lParam)).SetWindowPos(tab_rect));
				}
			}

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

											auto tab_rect = tab_control.GetClientRect().and_then([&](auto value) { return tab_control.MapWindowPoints(*this, value);  }).value().second;
											SendMessageW(tab_control, TCM_ADJUSTRECT, FALSE, std::bit_cast<win32::lparam_t>(&tab_rect));

											auto child = plugin->CreateWindowExW(::CREATESTRUCTW{
												.hwndParent = *this,
												.cy = tab_rect.bottom - tab_rect.top,
												.cx = tab_rect.right - tab_rect.left,
												.y = tab_rect.top,
												.x = tab_rect.left,
												.style = WS_CHILD,
												.lpszClass = class_name.c_str(),
											});


											assert(child);

											::STATSTG info{};

											stream->Stat(&info, STATFLAG::STATFLAG_NONAME);
										
											auto path_ref = path->c_str();

											auto handle = ::CreateFileW(path_ref, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

											auto mapping = ::CreateFileMapping(handle, nullptr, PAGE_READONLY, 0, 0, nullptr);

											COPYDATASTRUCT data{
												.cbData = DWORD(info.cbSize.QuadPart),
												.lpData = ::MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, std::size_t(info.cbSize.QuadPart))
											};

											assert(stream->Release() == 0);
	
											child->SetPropW(L"Filename", path_ref);

											if (SendMessageW(*child, WM_COPYDATA, win32::wparam_t(win32::hwnd_t(*this)), win32::lparam_t(&data)))
											{
												child->RemovePropW(L"Filename");

												auto index = tab_control.GetItemCount() - 1;

												tab_control.InsertItem(index, TCITEMW {
														.mask = TCIF_TEXT | TCIF_PARAM,
														.pszText = const_cast<wchar_t*>(path->filename().c_str()),
														.lParam = win32::lparam_t(child->get())
													});

												SetWindowLongPtrW(*child, GWLP_ID, index);										
											}
											else
											{
												child->RemovePropW(L"Filename");
												::DestroyWindow(*child);
											}

											::UnmapViewOfFile(data.lpData);
											::CloseHandle(mapping);
											::CloseHandle(handle);
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
}

#endif