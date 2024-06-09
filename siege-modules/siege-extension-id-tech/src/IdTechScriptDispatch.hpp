#ifndef DARKSTAR_SCRIPT_DISPATCH_HPP
#define DARKSTAR_SCRIPT_DISPATCH_HPP

#include "ScriptDispatchBase.hpp"

namespace siege::extension
{
	struct IdTechScriptDispatch final : ScriptDispatchBase
	{
		using ScriptDispatchBase::ScriptDispatchBase;

		HRESULT __stdcall Invoke(DISPID dispIdMember, const GUID& riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) override
		{
			assert(IsEqualGUID(riid, IID_NULL));

			if (pVarResult == nullptr)
			{
				return DISP_E_PARAMNOTOPTIONAL;
			}

			auto index = dispIdMember - 1;

			if (dispIdMember == DISPID_UNKNOWN)
			{
				return DISP_E_MEMBERNOTFOUND;
			}

			if ((index + 1) > functions.size())
			{
				return DISP_E_MEMBERNOTFOUND;
			}

			if (pDispParams == nullptr)
			{
				thread_local DISPPARAMS empty_params{};
				pDispParams = &empty_params;
			}

			if (pDispParams->cNamedArgs > 0)
			{
				return DISP_E_NONAMEDARGS;
			}

			auto func_iter = functions.begin();
			std::advance(func_iter, index);

			std::string exec_string;

			exec_string.clear();
			exec_string.reserve(func_iter->size() + 4 + (pDispParams->cArgs * sizeof(std::string) + 2));

			exec_string.append(*func_iter);

			if (pDispParams->cArgs > 0)
			{
				exec_string.append(1, ' ');
			}

			for (auto i = 0u; i < pDispParams->cArgs; ++i)
			{
				if (pDispParams->rgvarg[i].vt == VT_NULL || pDispParams->rgvarg[i].vt == VT_EMPTY)
				{
					exec_string.append("\"\"");
				}
				else if (pDispParams->rgvarg[i].vt == VT_BSTR)
				{
					exec_string.append(1, '"');
					exec_string.append(win32::com::to_string(pDispParams->rgvarg[i]));
					exec_string.append(1, '"');
				}
				else
				{
					auto size = exec_string.size();

					exec_string.append(win32::com::to_string(pDispParams->rgvarg[i]));

					if (size == exec_string.size())
					{
						if (puArgErr)
						{
							*puArgErr = i;
						}
						return DISP_E_TYPEMISMATCH;
					}
				}

				if (i != (pDispParams->cArgs - 1))
				{
					exec_string.append(1, ' ');
				}
			}

			auto result = eval(exec_string);

			*pVarResult = win32::com::Variant(std::wstring(result.begin(), result.end()));

			return S_OK;
		}
	};
}

#endif