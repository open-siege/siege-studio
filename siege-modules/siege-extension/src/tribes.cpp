#include <cstring>
#include <cstdint>
#include <algorithm>
#include <array>
#include <unordered_set>
#include <utility>
#include <thread>
#include <string_view>
#include <fstream>
#include <siege/platform/win/core/file.hpp>
#include <siege/platform/win/desktop/window_module.hpp>
#include <siege/platform/win/desktop/window_impl.hpp>
#include <detours.h>
#include "ScriptDispatch.hpp"
#include "MessageHandler.hpp"

extern "C"
{
#define DARKCALL __attribute__((regparm(3)))

	static DARKCALL char* (*ConsoleEval)(void*, std::int32_t, std::int32_t, const char**) = nullptr;

	using namespace std::literals;

	constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 4>, 4> verification_strings = { {
	std::array<std::pair<std::string_view, std::size_t>, 4>{{
		{"cls"sv, std::size_t(0x6fb741)},
		{"trace"sv, std::size_t(0x6fb7af)},
		{"Console::logBufferEnabled"sv, std::size_t(0x6fb7b5)},
		{"Console::logMode"sv, std::size_t(0x6fb7fa)},
		}},
	std::array<std::pair<std::string_view, std::size_t>, 4>{{
		{"cls"sv, std::size_t(0x657211)},
		{"trace"sv, std::size_t(0x65727f)},
		{"Console::logBufferEnabled"sv, std::size_t(0x657285)},
		{"Console::logMode"sv, std::size_t(0x6572ca)},
		}}
	} };

	constexpr static std::array<std::pair<std::string_view, std::string_view>, 28> function_name_ranges{ {
		{"File::findFirst"sv, "File::getPath"sv},
		{"setMaterialProperty"sv, "addExportText"sv},
		{"spawnPlayer"sv, "Client::cmdObservePlayer"sv},
		{"AI::Spawn"sv, "Object::getName"sv},
		{"ME::Create"sv, "ME::RebuildCommandMap"sv},
		{"loadObject"sv, "getNextObject"sv},
		{"HTMLOpen"sv, "HTMLOpenAndGoWin"sv},
		{"swapSurfaces"sv, "isGfxDriver"sv},
		{"netStats"sv, "net::kick"sv},
		{"newRedbook"sv, "rbSetPlayMode"sv},
		{"newInputManager"sv, "popActionMap"sv},
		{"simTreeCreate"sv, "simTreeRegScript"sv},
		{"newSfx"sv, "sfxGetMaxBuffers"sv},
		{"newToolWindow"sv, "saveFileAs"sv},
		{"newTerrain"sv, "lightTerrain"sv},
		{"GuiEditMode"sv, "windowsKeyboardDisable"sv},
		{"LS::Create"sv, "LS::parseCommands"sv},
		{"ircConnect"sv, "ircEcho"sv},
		{"BaseRep::getFirst"sv, "Object::getName"sv},
		{"FGTextList::sort"sv, "FGTextList::sort"sv},
		{"CmdInventory::getVisibleSet"sv, "CmdInventory::setFavorites"sv},
		{"FGSkin::set"sv, "FGSkin::cycleArmor"sv},
		{"FGArray::addEntry"sv, "FGArray::getSelectedText"sv},
		{"FGCombo::addEntry"sv, "FGCombo::selectNext"sv},
		{"Server::ResortList"sv, "Server::ResortList"sv},
		{"FGSlider::setDiscretePositions"sv, "FGSlider::setDiscretePositions"sv},
		{"FGMasterList::addEntry"sv, "FGMasterList::getSelected"sv},
		{"cls"sv, "trace"sv}
		} };

	constexpr static std::array<std::pair<std::string_view, std::string_view>, 13> variable_name_ranges{ {
		{"Console::ForeRGB"sv, "Console::LastLineTimeout"sv},
		{"$pref::mipcap"sv, "$OpenGL::TNB"sv},
		{"GFXMetrics::EmittedPolys"sv, "GFXMetrics::numPaletteDLs"sv},
		{"pref::sfx2DVolume"sv, "pref::sfx3DVolume"sv},
		{"GuiEdit::GridSnapX"sv, "pref::politeGui"sv},
		{"Console::logBufferEnabled"sv, "Console::logMode"sv},
		{"DNet::ShowStats"sv, "DNet::PacketLoss"sv}
		}};

	HRESULT __stdcall ExecutableIsSupported(_In_ const wchar_t* filename) noexcept
	{
		if (filename == nullptr)
		{
			return E_POINTER;
		}

		if (!std::filesystem::exists(filename))
		{
			return E_INVALIDARG;
		}


		try
		{
			win32::file filw(std::filesystem::path(filename), GENERIC_READ, FILE_SHARE_READ, std::nullopt, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL);

			auto file_size = filw.GetFileSizeEx();
			auto mapping = filw.CreateFileMapping(std::nullopt, PAGE_READONLY, 0, 0, L"");

			if (mapping && file_size)
			{
				auto view = mapping->MapViewOfFile(FILE_MAP_READ, (std::size_t)file_size->QuadPart);
				std::string_view data((char*)view.get(), (std::size_t)file_size->QuadPart);

				if (data.empty())
				{
					return S_FALSE;
				}

				auto is_executable = data.find("MZ") == 0 && data.find("PE") != std::string_view::npos;

				bool has_all_verification_strings = is_executable && std::all_of(verification_strings[0].begin(), verification_strings[0].end(), [&](auto& item) {
					return data.find(item.first) != std::string_view::npos;
					});

				bool has_all_functions = has_all_verification_strings && std::all_of(function_name_ranges.begin(), function_name_ranges.end(), [&](auto& item) {

					auto first_index = data.find(item.first);

					if (first_index != std::string_view::npos)
					{
						return data.find(item.second, first_index) != std::string_view::npos;
					}

					return false;
					});

				bool has_all_variables = has_all_functions && std::all_of(variable_name_ranges.begin(), variable_name_ranges.end(), [&](auto& item) {

					auto first_index = data.find(item.first);

					if (first_index != std::string_view::npos)
					{
						return data.find(item.second, first_index) != std::string_view::npos;
					}
					return false;
					});

				return has_all_variables ? S_OK : S_FALSE;
			}

			return S_FALSE;

		}
		catch (...)
		{
			return S_FALSE;
		}
	}


	inline void set_18_exports()
	{
		ConsoleEval = (decltype(ConsoleEval))0x5f26f0;
	}

	inline void set_19_exports()
	{
		ConsoleEval = (decltype(ConsoleEval))0x5f28f4;
	}

	inline void set_110_exports()
	{
		ConsoleEval = (decltype(ConsoleEval))0x5f33a8;
	}

	inline void set_1105_exports()
	{
		ConsoleEval = (decltype(ConsoleEval))0x5f33bc;
	}

	inline void set_111_exports()
	{
		ConsoleEval = (decltype(ConsoleEval))0x5f3700;
	}

	constexpr std::array<void(*)(), 5> export_functions = { {
			set_18_exports,
			set_19_exports,
			set_110_exports,
			set_1105_exports,
			set_111_exports,
		} };


	static auto* TrueSetWindowsHookExA = SetWindowsHookExA;
	static auto* TrueAllocConsole = AllocConsole;
	
	BOOL WINAPI WrappedAllocConsole()
	{
		win32::com::init_com();

		return TrueAllocConsole();
	}

	HHOOK WINAPI WrappedSetWindowsHookExA(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId)
	{
		win32::com::init_com();

		if (dwThreadId == 0)
		{
			dwThreadId = ::GetCurrentThreadId();
		}

		return TrueSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
	}

	static std::array<std::pair<void**, void*>, 2> detour_functions{	{
		{ &(void*&)TrueSetWindowsHookExA, WrappedSetWindowsHookExA },
		{ &(void*&)TrueAllocConsole, WrappedAllocConsole } 
	}};

	BOOL WINAPI DllMain(
		HINSTANCE hinstDLL,
		DWORD fdwReason,
		LPVOID lpvReserved) noexcept
	{
		if (DetourIsHelperProcess())
		{
			return TRUE;
		}

		if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
		{
			auto app_module = win32::module_ref(::GetModuleHandleW(nullptr));

			auto value = app_module.GetProcAddress<std::uint32_t*>("DisableSiegeExtensionModule");

			if (value && *value == -1)
			{
				return TRUE;
			}
		}

		if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
		{
			if (fdwReason == DLL_PROCESS_ATTACH)
			{
				int index = 0;
				try
				{
					auto app_module = win32::module_ref(::GetModuleHandleW(nullptr));

					std::unordered_set<std::string_view> functions;
					std::unordered_set<std::string_view> variables;

					bool module_is_valid = false;

					for (const auto& item : verification_strings)
					{
						win32::module_ref temp((void*)item[0].second);

						if (temp != app_module)
						{
							continue;
						}

						module_is_valid = std::all_of(item.begin(), item.end(), [](const auto& str) {
							return std::memcmp(str.first.data(), (void*)str.second, str.first.size()) == 0;
							});


						if (module_is_valid)
						{
							export_functions[index]();

							std::string_view string_section((const char*)ConsoleEval, 1024 * 1024 * 2);


							for (auto& pair : function_name_ranges)
							{
								auto first_index = string_section.find(pair.first.data(), 0, pair.first.size() + 1);

								if (first_index != std::string_view::npos)
								{
									auto second_index = string_section.find(pair.second.data(), first_index, pair.second.size() + 1);

									if (second_index != std::string_view::npos)
									{
										auto second_ptr = string_section.data() + second_index;
										auto end = second_ptr + std::strlen(second_ptr) + 1;

										for (auto start = string_section.data() + first_index; start != end; start += std::strlen(start) + 1)
										{
											std::string_view temp(start);

											if (temp.size() == 1)
											{
												continue;
											}

											if (!std::all_of(temp.begin(), temp.end(), [](auto c) { return std::isalnum(c) != 0; }))
											{
												break;
											}

											functions.emplace(temp);
										}
									}
								}
							}

							break;
						}
						index++;
					}

					if (!module_is_valid)
					{
						return FALSE;
					}

					DetourRestoreAfterWith();

					DetourTransactionBegin();
					DetourUpdateThread(GetCurrentThread());

					std::for_each(detour_functions.begin(), detour_functions.end(), [](auto& func) { DetourAttach(func.first, func.second); });

					DetourTransactionCommit();

					auto self = win32::window_module_ref(hinstDLL);
					auto atom = self.RegisterClassExW(win32::static_window_meta_class<siege::extension::MessageHandler>{});

					auto type_name = win32::type_name<siege::extension::MessageHandler>();

					auto host = std::make_unique<siege::extension::ScriptDispatch>(std::move(functions), std::move(variables), [](std::string_view eval_string) -> std::string_view {
						std::array<const char*, 2> args{ "eval", eval_string.data() };

						// Luckily this function is static and doesn't need the console instance object nor
						// an ID to identify the callback. It doesn't even check for "eval" and skips straight to the second argument.
						auto result = ConsoleEval(nullptr, 0, 2, args.data());

						if (result == nullptr)
						{
							return "";
						}


						return result;
						});

					// TODO register multiple script hosts
					if (auto message = self.CreateWindowExW(CREATESTRUCTW{
						.lpCreateParams = host.release(),
						.hwndParent = HWND_MESSAGE,
						.style = WS_CHILD,
						.lpszName = L"siege::extension::tribes::ScriptHost",
						.lpszClass = win32::type_name<siege::extension::MessageHandler>().c_str()
						}); message)
					{
					}
				}
				catch (...)
				{
					return FALSE;
				}
			}
			else if (fdwReason == DLL_PROCESS_DETACH)
			{
				DetourTransactionBegin();
				DetourUpdateThread(GetCurrentThread());

				std::for_each(detour_functions.begin(), detour_functions.end(), [](auto& func) { DetourDetach(func.first, func.second); });
				DetourTransactionCommit();

				auto window = ::FindWindowExW(HWND_MESSAGE, nullptr, win32::type_name<siege::extension::MessageHandler>().c_str(), L"siege::extension::starsiege::ScriptHost");
				::DestroyWindow(window);
				auto self = win32::window_module(hinstDLL);

				self.UnregisterClassW<siege::extension::MessageHandler>();
			}
		}

		return TRUE;
	}
}


