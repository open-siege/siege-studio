#ifndef VOL_VIEW_HPP
#define VOL_VIEW_HPP

#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/menu.hpp>
#include <siege/platform/siege_module.hpp>
#include <siege/platform/shared.hpp>
#include <spanstream>
#include <map>
#include <set>
#include "vol_controller.hpp"

namespace siege::views
{
	struct vol_view : win32::window_ref
	{
		vol_controller controller;

		win32::list_view table;

		std::list<siege_module> modules;

		std::set<std::wstring> all_categories;
		std::map<std::wstring_view, std::set<std::wstring>> category_extensions;
		std::map<std::wstring_view, win32::wparam_t> categories_to_groups;
		std::map<std::wstring_view, std::wstring_view> extensions_to_categories;

		vol_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
		{
		}

		auto on_create(const win32::create_message& data)
		{
			auto app_path = std::filesystem::path(win32::module_ref().current_application().GetModuleFileNameW());
			modules = siege_module::LoadSiegeModules(app_path.parent_path());

			for (const auto& module : modules)
			{
				auto categories = module.GetSupportedFormatCategories(LOCALE_USER_DEFAULT);

				for (auto& category : categories)
				{
					auto& stored_category = *all_categories.insert(std::move(category)).first;
					auto extensions = module.GetSupportedExtensionsForCategory(stored_category);

					auto existing = category_extensions.find(stored_category);

					if (existing == category_extensions.end())
					{
						existing = category_extensions.emplace(stored_category, std::move(extensions)).first;
					}
					else
					{
						std::transform(extensions.begin(), extensions.end(), std::inserter(existing->second, existing->second.end()), [&](auto& ext) {
							return std::move(ext);
						});
					}

					for (auto& item : existing->second)
					{
						extensions_to_categories.emplace(item, stored_category);
					}
				}
			}

			auto factory = win32::window_factory(ref());

			table = *factory.CreateWindowExW<win32::list_view>(CREATESTRUCTW{
							.style = WS_VISIBLE | WS_CHILD | LVS_REPORT,
				});

			int id = 1;
			for (auto& item : category_extensions)
			{
				auto index = table.InsertGroup(-1, LVGROUP{
					.pszHeader = const_cast<wchar_t*>(item.first.data()),
					.iGroupId = id,
					.state = LVGS_COLLAPSIBLE,
					});

				if (index != -1)
				{
					categories_to_groups.emplace(item.first, id++);
				}
			}

			if (!categories_to_groups.empty())
			{
				table.EnableGroupView(true);
			}

			table.InsertColumn(-1, LVCOLUMNW{
					.pszText = const_cast<wchar_t*>(L"Filename"),
				});

			table.InsertColumn(-1, LVCOLUMNW{
					.pszText = const_cast<wchar_t*>(L"Path"),
				});

			table.InsertColumn(-1, LVCOLUMNW{
					.pszText = const_cast<wchar_t*>(L"Size (in bytes)"),
				});

			table.InsertColumn(-1, LVCOLUMNW{
				  .pszText = const_cast<wchar_t*>(L"Compression Method"),
				});

			auto header = table.GetHeader();

			auto style = header.GetWindowStyle();
			header.SetWindowStyle(style | HDS_FILTERBAR);

			return 0;
		}

		std::optional<win32::lresult_t> on_notify(win32::list_view_item_activation message)
		{
			switch (message.hdr.code)
			{
				case NM_DBLCLK:
				{
					auto root = this->GetAncestor(GA_ROOT);

					if (root)
					{
						std::array<wchar_t, 256> temp{};

						auto item_info = table.GetItem(LVITEMW {
							.mask = LVIF_TEXT,
							.iItem = message.iItem,
							.pszText = temp.data(),
							.cchTextMax = 256
							});

						if (item_info)
						{
							auto items = controller.get_contents();

							auto item = std::find_if(items.begin(), items.end(), [&](auto& content) {
								if (auto* file = std::get_if<siege::platform::file_info>(&content))
								{
									return std::wstring_view(file->filename.c_str()) == item_info->pszText;
								}

								return false;
								});

							if (item != items.end())
							{
								auto data = controller.load_content_data(*item);

								root->SetPropW(L"Filename", temp.data());
								root->CopyData(*this, COPYDATASTRUCT {
										.cbData = DWORD(data.size()),
										.lpData = data.data()
									});

								root->RemovePropW(L"Filename");
							}
						}
					}
					return 0;
				}
				default:
				{
					return std::nullopt;
				}
			}
		}

		std::optional<win32::lresult_t> on_notify(win32::header_notification message)
		{
			switch (message.hdr.code)
			{
				case HDN_FILTERCHANGE:
				{
					return 0;
				}
				case HDN_ENDFILTEREDIT:
				{
					return 0;
				}
				default:
				{
					return std::nullopt;
				}
			}
		}

		auto on_size(win32::size_message sized)
		{
			table.SetWindowPos(sized.client_size);
			auto column_count = table.GetColumnCount();

			auto column_width = sized.client_size.cx / column_count;

			for (auto i = 0u; i < column_count; ++i)
			{
				table.SetColumnWidth(i, column_width);
			}

			return 0;
		}

		auto on_copy_data(win32::copy_data_message<char> message)
		{
			std::spanstream stream(message.data);

			if (vol_controller::is_vol(stream))
			{
				std::optional<std::filesystem::path> path;

				if (wchar_t* filename = this->GetPropW<wchar_t*>(L"Filename"); filename)
				{
					path = filename;
				}

				auto count = controller.load_volume(stream, path);

				if (count > 0)
				{
					auto contents = controller.get_contents();

					for (auto& content : contents)
					{
						if (auto* file = std::get_if<siege::platform::file_info>(&content))
						{
							win32::list_view_item item{ file->filename };

							auto extension = platform::to_lower(file->filename.extension().wstring());
							auto category = extensions_to_categories.find(extension);

							if (category != extensions_to_categories.end())
							{
								item.iGroupId = categories_to_groups[category->second];
								item.sub_items = {
									std::filesystem::relative(file->archive_path, file->folder_path).wstring(),
									std::to_wstring(file->size)
								};

								table.InsertRow(item);
							}
						}
					}

					return TRUE;
				}

				return FALSE;
			}

			return FALSE;
		}
	};

}

#endif