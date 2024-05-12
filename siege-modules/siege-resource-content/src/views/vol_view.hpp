#ifndef VOL_VIEW_HPP
#define VOL_VIEW_HPP

#include <siege/platform/win/desktop/win32_common_controls.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/win32_menu.hpp>
#include <spanstream>
#include "vol_controller.hpp"

namespace siege::views
{
	struct vol_view : win32::window
	{
		constexpr static auto formats = std::array<std::wstring_view, 20>{{
				L".vol", L".rmf", L".mis", L".map", L".rbx", L".tbv", L".zip", L".vl2", L".pk3",
					L".iso", L".mds", L".cue", L".nrg", L".7z", L".tgz", L".rar", L".cab", L".z", L".cln", L".atd"
			}};

		vol_controller controller;

		win32::list_view table;

		vol_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window(self)
		{
		}

		auto on_create(const win32::create_message& data)
		{
			auto factory = win32::window_factory(win32::window_ref(*this));

			table = *factory.CreateWindowExW<win32::list_view>(CREATESTRUCTW{
							.style = WS_VISIBLE | WS_CHILD | LVS_REPORT,
				});

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

		std::optional<win32::lresult_t> on_notify(win32::header_notify_message message)
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

							item.sub_items = {
								std::filesystem::relative(file->archive_path, file->folder_path).wstring(),
								std::to_wstring(file->size)
							};

							table.InsertRow(item);
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