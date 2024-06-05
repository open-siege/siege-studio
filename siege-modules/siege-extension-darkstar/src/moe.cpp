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
#include "DarkstarScriptDispatch.hpp"
#include "MessageHandler.hpp"

extern "C"
{
#define DARKCALL __attribute__((regparm(3)))

	static DARKCALL char* (*ConsoleEval)(void*, std::int32_t, std::int32_t, const char**) = nullptr;

	using namespace std::literals;

	constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 4>, 1> verification_strings = { {
	std::array<std::pair<std::string_view, std::size_t>, 3>{{
		{"alias"sv, std::size_t(0x61cc48)},
		{"quit"sv, std::size_t(0x61cbec)},
		{"KQGame::PlayFromCD"sv, std::size_t(0x5b0170)}
		}} 
	};

	constexpr static std::array<std::pair<std::string_view, std::string_view>, 28> function_name_ranges{ {
		{"KQMusic::play"sv, "KQMusic::delete"sv},
		{"deleteObjects"sv, "killGame"sv},
		{"KQObject::setLoc"sv, "KQObject::setGroupsActive3DFar"sv},
		{"KQCamera::follow"sv, "KQCamera::setHeight"sv},
		{"profile"sv, "KQGame::letCatchUp"sv},
		{"KQMonster::god"sv, "KQConner::removeFromTrap"sv},
		{"KQEmitter::setFollowObject"sv, "KQEmitter::reinit"sv},
		{"KQMap::activate"sv, "KQMap::stopLoc"sv},
		{"Heap::check"sv, "Heap::findLeaks"sv},
		{"swapSurfaces"sv, "messageCanvasDevice"sv},
		{"newDirectionalLight"sv, "newPointLight"sv},
		{"newSky"sv, "globeLines"sv},
		{"loadInterior"sv, "loadInterior"sv},
		{"alias"sv, "quit"sv},
		{"KQSound::setSpeechVol"sv, "KQSound::setMixLimit"sv},
		
		} };

	constexpr static std::array<std::pair<std::string_view, std::string_view>, 13> variable_name_ranges{ {
		{"KQPortal::ConLoc"sv, "NewWorld"sv},
		{"ConsoleWorld::FrameRate"sv, "allowAltEnter"sv},
		{"SetFirstPerson::WasThirdPerson"sv, "ConInv::Boots"sv}
		} };

	inline void set_gog_exports()
	{
	}

	constexpr std::array<void(*)(), 5> export_functions = { {
			set_gog_exports,
		} };


	static auto* TrueSetWindowsHookExA = SetWindowsHookExA;

	HHOOK WINAPI WrappedSetWindowsHookExA(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId)
	{
		win32::com::init_com();

		if (dwThreadId == 0)
		{
			dwThreadId = ::GetCurrentThreadId();
		}

		return TrueSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
	}

	static std::array<std::pair<void**, void*>, 1> detour_functions{ {
		{ &(void*&)TrueSetWindowsHookExA, WrappedSetWindowsHookExA }
	} };

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
						.lpszName = L"siege::extension::maskOfEternity::ScriptHost",
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

				auto window = ::FindWindowExW(HWND_MESSAGE, nullptr, win32::type_name<siege::extension::MessageHandler>().c_str(), L"siege::extension::maskOfEternity::ScriptHost");
				::DestroyWindow(window);
				auto self = win32::window_module(hinstDLL);

				self.UnregisterClassW<siege::extension::MessageHandler>();
			}
		}

		return TRUE;
	}
}


