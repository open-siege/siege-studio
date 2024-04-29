#ifndef WIN32_COM_XML_HPP
#define WIN32_COM_XML_HPP

#include <vector>
#include <any>
#include <string_view>
#include <filesystem>
#include <memory>
#include <expected>
#include <xmllite.h>
#include <siege/platform/win/core/com.hpp>

namespace win32::com
{
	std::expected<std::unique_ptr<IXmlReader, com_deleter<IXmlReader>>, HRESULT> CreateXmlReader()
	{
		com_ptr<IXmlReader> raw = nullptr;
		auto result = ::CreateXmlReader(__uuidof(IXmlReader), raw.put_void(), nullptr);

		if (result != S_OK)
		{
			return std::unexpected(result);
		}

		return raw;
	}
}

#endif