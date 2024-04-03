#ifndef WIN32_COM_XML_HPP
#define WIN32_COM_XML_HPP

#include <vector>
#include <any>
#include <string_view>
#include <filesystem>
#include <memory>
#include <expected>
#include <xmllite.h>
#include "win32_com.hpp"

namespace win32::com
{
	std::expected<std::unique_ptr<IXmlReader, void(*)(IXmlReader*)>, HRESULT> CreateXmlReader()
	{
		IXmlReader* raw = nullptr;
		auto result = CreateXmlReader(__uuidof(IXmlReader), (void**)&raw, nullptr);

		if (result != S_OK)
		{
			return std::unexpected(result);
		}

		return as_unique<IXmlReader>(raw);
	}
}

#endif