#ifndef SCRIPT_DISPATCH_HPP
#define SCRIPT_DISPATCH_HPP

#include <unordered_set>
#include <string>
#include <functional>
#include <siege/platform/win/core/com_collection.hpp>

namespace siege::extension
{
	struct ScriptDispatchBase : win32::com::ComObject, IDispatch
	{
		std::unordered_set<std::string_view> functions;
		std::unordered_set<std::string_view> variables;

		std::move_only_function<std::string_view(std::string_view)> eval;


		ScriptDispatchBase(std::unordered_set<std::string_view> functions, std::unordered_set<std::string_view> variables, std::move_only_function<std::string_view(std::string_view)> eval) :
			functions(std::move(functions)),
			variables(std::move(variables)),
			eval(std::move(eval))
		{
		}

		HRESULT __stdcall QueryInterface(const GUID& riid, void** ppvObj) noexcept override
		{
			return ComQuery<IUnknown, IDispatch>(*this, riid, ppvObj)
				.or_else([&]() { return ComQuery<IDispatch>(*this, riid, ppvObj); })
				.value_or(E_NOINTERFACE);
		}

		[[maybe_unused]] ULONG __stdcall AddRef() noexcept override
		{
			return ComObject::AddRef();
		}

		[[maybe_unused]] ULONG __stdcall Release() noexcept override
		{
			return ComObject::Release();
		}

		HRESULT __stdcall GetIDsOfNames(const GUID& riid, wchar_t** rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) noexcept override
		{
			assert(IsEqualGUID(riid, IID_NULL));

			if (cNames >= 0)
			{
				assert(rgszNames);

				std::wstring_view temp_view = rgszNames[0];
				std::string temp(temp_view.begin(), temp_view.end());

				auto function_iter = std::find_if(functions.begin(), functions.end(), [&](auto& name) {
					return CompareStringA(lcid, NORM_IGNORECASE, name.data(), name.size(), temp.data(), temp.size()) == CSTR_EQUAL;
					});

				if (function_iter != functions.end())
				{
					*rgDispId = std::distance(functions.begin(), function_iter) + 1;
					return S_OK;
				}


				*rgDispId = DISPID_UNKNOWN;
			}

			return DISP_E_UNKNOWNNAME;
		}

		HRESULT __stdcall GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) noexcept override
		{
			if (ppTInfo)
			{
				*ppTInfo = nullptr;
			}
			return DISP_E_BADINDEX;
		}

		HRESULT __stdcall GetTypeInfoCount(UINT* pctinfo) override
		{
			assert(pctinfo);
			*pctinfo = 0;
			return S_OK;
		}
	};
}

#endif