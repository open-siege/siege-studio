#include "win32_com_client.hpp"
#include "win32_com_variant.hpp"
#include "win32_property_variant.hpp"
#include "win32_stream_buf.hpp"
#include "win32_com_xml.hpp"
#include "win32_com.hpp"

namespace win32::com
{
	HRESULT init_com()
	{
		thread_local HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		thread_local auto com_handle = as_unique<HRESULT>(&result, [](auto*){ CoUninitialize(); });
		return result;
	}
}