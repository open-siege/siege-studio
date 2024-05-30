#ifndef SCRIPT_DISPATCH_HPP
#define SCRIPT_DISPATCH_HPP

#include <unordered_set>
#include <string>
#include <functional>
#include <siege/platform/win/core/com_collection.hpp>

namespace siege::extension
{
	struct ScriptDispatch final : win32::com::ComObject, IDispatch
	{
        std::unordered_set<std::string_view> functions;
        std::unordered_set<std::string_view> variables;

        std::move_only_function<std::string(std::string_view)> eval;


        ScriptDispatch(std::unordered_set<std::string_view> functions, std::unordered_set<std::string_view> variables, std::move_only_function<std::string(std::string_view)> eval) : 
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

        HRESULT __stdcall GetIDsOfNames(const GUID& riid, wchar_t **rgszNames, UINT cNames, LCID lcid, DISPID  *rgDispId) noexcept override
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

        HRESULT __stdcall GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) noexcept override
        {
            if (ppTInfo)
            {
                *ppTInfo = nullptr;
            }
            return DISP_E_BADINDEX;
        }

        HRESULT __stdcall GetTypeInfoCount(UINT *pctinfo) override
        {
            assert(pctinfo);
            *pctinfo = 0;
            return S_OK;
        }

        HRESULT __stdcall Invoke(DISPID dispIdMember, const GUID& riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) override
        {
            assert(IsEqualGUID(riid, IID_NULL));

            if (pVarResult == nullptr)
            {
                return DISP_E_PARAMNOTOPTIONAL;
            }

            auto index = dispIdMember - 1;
            

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

            thread_local std::string exec_string;

            exec_string.clear();
            exec_string.reserve(func_iter->size() + 4 + pDispParams->cArgs * 4);
            
            exec_string.append(*func_iter);

            exec_string.append(1, '(');

            for (auto i = 0u; i < pDispParams->cArgs; ++i)
            {
                switch (pDispParams->rgvarg[i].vt)
                {   
                    case VT_NULL:
                    case VT_EMPTY:
                    {
                        exec_string.append("\"\"");
                        break;
                    }
                    case VT_BOOL:
                    {
                        exec_string.append(pDispParams->rgvarg[i].boolVal == 1 ? "True" : "False");
                        break;
                    }
                    case VT_INT:
                    {
                        exec_string.append(std::to_string(pDispParams->rgvarg[i].intVal));
                        break;
                    }
                    case VT_UINT:
                    {
                        exec_string.append(std::to_string(pDispParams->rgvarg[i].uintVal));
                        break;
                    }
                    case VT_I2:
                    {
                        exec_string.append(std::to_string(pDispParams->rgvarg[i].iVal));
                        break;
                    }
                    case VT_UI2:
                    {
                        exec_string.append(std::to_string(pDispParams->rgvarg[i].uiVal));
                        break;
                    }
                    case VT_I4:
                    {
                        exec_string.append(std::to_string(pDispParams->rgvarg[i].lVal));
                        break;
                    }
                    case VT_UI4:
                    {
                        exec_string.append(std::to_string(pDispParams->rgvarg[i].ulVal));
                        break;
                    }
                    case VT_R4:
                    {
                        exec_string.append(std::to_string(pDispParams->rgvarg[i].fltVal));
                        break;
                    }
                    case VT_R8:
                    {
                        exec_string.append(std::to_string(pDispParams->rgvarg[i].dblVal));
                        break;
                    }
                    case VT_BSTR:
                    {
                        std::wstring_view temp(pDispParams->rgvarg[i].bstrVal, ::SysStringLen(pDispParams->rgvarg[i].bstrVal));
                        exec_string.append(1, '"');
                        exec_string.append(std::string(temp.begin(), temp.end()));
                        exec_string.append(1, '"');
                        break;
                    }
                    default:
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
                    exec_string.append(1, ',');            
                }
            }

            exec_string.append(1, ')');
            exec_string.append(1, ';');

            auto result = eval(exec_string);

            *pVarResult = win32::com::Variant(std::wstring(result.begin(), result.end()));

            return S_OK;
        }
	};


}

#endif