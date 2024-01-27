#define COM_NO_WINDOWS_H

#include <vector>
#include <any>
#include <filesystem>
#include <memory>
#include <combaseapi.h>
#include <msxml6.h>

std::vector<std::any> load_data(std::filesystem::path path)
{
	HRESULT result;
	IXMLDOMDocument* document;

	result = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	result = CoCreateInstance(CLSID_DOMDocument60, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument3, reinterpret_cast<void**>(&document));

	BSTR data = nullptr;

	VARIANT_BOOL success;
	result = document->loadXML(data, &success);

	return {};
}