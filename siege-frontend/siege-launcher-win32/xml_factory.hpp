#define COM_NO_WINDOWS_H

#include <vector>
#include <any>
#include <filesystem>
#include <memory>
#include <combaseapi.h>
#include <msxml6.h>

template<typename T>
auto make_unique(T* value, void(*deleter)(T*))
{
	return std::unique_ptr<T, void(*)(T*)>(value, deleter);
}

std::vector<std::any> load_data(std::filesystem::path path)
{
	HRESULT result;

	result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	void* raw = nullptr;
	result = CoCreateInstance(CLSID_DOMDocument60, nullptr, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument3, &raw);

	auto document = make_unique<IXMLDOMDocument3>(reinterpret_cast<IXMLDOMDocument3*>(raw), [](IXMLDOMDocument3* self) { if (self) self->Release(); });
	auto mapping_handle = make_unique<void>(OpenFileMappingW(FILE_MAP_READ, FALSE, path.c_str()), [](HANDLE base) {CloseHandle(base);});
	auto file_data = make_unique<void>(MapViewOfFile(mapping_handle.get(), FILE_MAP_READ, 0, 0, 0), [](void* base) {UnmapViewOfFile(base);});

	auto unparsed_data = make_unique<wchar_t>(SysAllocString(reinterpret_cast<wchar_t*>(file_data.get())), [](wchar_t* data) { SysFreeString(data); });

	VARIANT_BOOL success;
	result = document->loadXML(unparsed_data.get(), &success);

	return {};
}