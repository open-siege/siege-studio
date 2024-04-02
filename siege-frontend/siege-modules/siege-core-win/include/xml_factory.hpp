#define COM_NO_WINDOWS_H

#include <vector>
#include <any>
#include <string_view>
#include <filesystem>
#include <memory>
#include <expected>
#include <combaseapi.h>
#include <msxml6.h>

template<typename T>
auto make_unique(T* value, void(*deleter)(T*))
{
	return std::unique_ptr<T, void(*)(T*)>(value, deleter);
}


HRESULT init_com()
{
	thread_local HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	thread_local auto com_handle = make_unique<HRESULT>(&result, [](auto*){ CoUninitialize(); });
	return result;
}

std::expected<IXMLDOMDocument3*, HRESULT> load_data(std::unique_ptr<wchar_t, void(*)(wchar_t*)> xml_data)
{
	HRESULT result = init_com();
	void* raw = nullptr;
	result = CoCreateInstance(CLSID_DOMDocument60, nullptr, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument3, &raw);

	auto document = make_unique<IXMLDOMDocument3>(reinterpret_cast<IXMLDOMDocument3*>(raw), [](auto* self) { if (self) self->Release(); });
	VARIANT_BOOL success;
	result = document->loadXML(xml_data.get(), &success);

	if (result != S_OK)
	{
		return std::unexpected(result);
	}

	if (success == VARIANT_FALSE)
	{
		return std::unexpected(S_FALSE);
	}

	return document.release();
}

auto load_data(std::string_view xml_data)
{
	auto raw_string = make_unique<wchar_t>(SysAllocStringByteLen(xml_data.data(), xml_data.size()), [](wchar_t* data) { SysFreeString(data); });
	return load_data(std::move(raw_string));
}

auto load_data(std::wstring_view xml_data)
{
	auto raw_string = make_unique<wchar_t>(SysAllocStringLen(xml_data.data(), xml_data.size()), [](wchar_t* data) { SysFreeString(data); });
	return load_data(std::move(raw_string));
}

auto load_data(std::filesystem::path path)
{
	auto file_size = std::filesystem::file_size(path);
	auto mapping_handle = make_unique<void>(OpenFileMappingW(FILE_MAP_READ, FALSE, path.c_str()), [](auto base) {CloseHandle(base);});
	auto file_data = make_unique<void>(MapViewOfFile(mapping_handle.get(), FILE_MAP_READ, 0, 0, file_size), [](auto* base) {UnmapViewOfFile(base);});
	return load_data(std::string_view(reinterpret_cast<char*>(file_data.get()), file_size));
}
