#ifndef HTTPCLIENTHPP
#define HTTPCLIENTHPP

#include <thread>
#include <memory>
#include <unordered_map>
#include <string>
#include <bit>
#include <winhttp.h>
#include "framework.h"
#include "windows.hpp"


constexpr static auto WM_HTTP_HEAD = WM_COUT + 1;
constexpr static auto WM_HTTP_GET = WM_HTTP_HEAD + 1;
constexpr static auto WM_HTTP_POST = WM_HTTP_HEAD + 2;
constexpr static auto WM_HTTP_PATCH = WM_HTTP_HEAD + 3;
constexpr static auto WM_HTTP_PUT = WM_HTTP_HEAD + 4;
constexpr static auto WM_HTTP_DELETE = WM_HTTP_HEAD + 5;

struct http_client
{
	std::thread worker;

	http_client() : worker(http_client::exec)
	{

	}

	static void exec()
	{
		std::shared_ptr<void> session(WinHttpOpen(L"A WinHTTP Example Program/1.0",
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS,
			0), WinHttpCloseHandle);

		MSG msg;


		std::unordered_map<std::wstring, std::shared_ptr<void>> connections;

		std::wstring port_and_domain;
		static_assert(sizeof(wchar_t) >= sizeof(INTERNET_PORT));
		port_and_domain.push_back(INTERNET_DEFAULT_HTTPS_PORT);
		port_and_domain.append(L"www.microsoft.com");

		connections.emplace(port_and_domain, std::shared_ptr<void>(WinHttpConnect(session.get(),
			port_and_domain.c_str() + 1,
			port_and_domain[0],
			0)));

		while (GetMessageW(&msg, nullptr, 0, 0))
		{
			auto cracked_message = make_window_message(msg);

			std::visit(overloaded{
				[&](::message& message) -> std::optional<LRESULT> {
					if (message.message == WM_HTTP_GET)
					{
						thread_local std::vector<std::byte> get_buffer;
						auto connection = connections.begin(); // TODO get connection for real


						auto request = std::shared_ptr<void>(WinHttpOpenRequest(connection->second.get(), L"GET", nullptr, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0), WinHttpCloseHandle);

						auto result = WinHttpAddRequestHeaders(request.get(),
							L"If-Modified-Since: Mon, 20 Nov 2000 20:00:00 GMT",
							(ULONG)-1L,
							WINHTTP_ADDREQ_FLAG_ADD);

						result = WinHttpSendRequest(request.get(),
							WINHTTP_NO_ADDITIONAL_HEADERS,
							0,
							WINHTTP_NO_REQUEST_DATA,
							0,
							0,
							0);

						result = WinHttpReceiveResponse(request.get(), nullptr);

						DWORD response_size;
						DWORD bytes_read;
						do
						{
							response_size = 0;
							WinHttpQueryDataAvailable(request.get(), &response_size);

							get_buffer.assign(response_size, std::byte{});


							WinHttpReadData(request.get(), get_buffer.data(), response_size, &bytes_read);

						} while (response_size > 0);
					}
					return std::nullopt;
				},
				[&](auto&)  -> std::optional<LRESULT> {
					return std::nullopt;
				}
				}, cracked_message);

			std::this_thread::yield();
		}
	}

	void get()
	{
		PostThreadMessageW(GetThreadId(worker.native_handle()), WM_HTTP_GET, 0, 0);
	}
};


#endif