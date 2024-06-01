#ifndef WIN32_COM_VARIANT_HPP
#define WIN32_COM_VARIANT_HPP

#include <string_view>
#include <string>
#include <algorithm>
#include <memory>
#include <oaidl.h>
#include <cassert>

namespace win32::com
{
	struct Variant : ::VARIANT
	{
		Variant() noexcept
		{
			::VariantInit(this);
		}

		Variant(const ::VARIANT& other) : Variant()
		{
			assert(::VariantCopy(this, &other) == S_OK);
		}

		Variant(const Variant& other) : Variant()
		{
			assert(::VariantCopy(this, &other) == S_OK);
		}

		~Variant() noexcept
		{
			assert(::VariantClear(this) == S_OK);
		}

		Variant(const std::wstring& other) : Variant()
		{
			this->vt = VT_BSTR;
			this->bstrVal = ::SysAllocStringLen(other.data(), other.size());
		}

		operator std::wstring() const
		{
			if (this->vt == VT_BSTR && this->bstrVal)
			{
				return std::wstring(this->bstrVal, ::SysStringLen(this->bstrVal));
			}

			if ((this->vt & VT_BSTR && this->vt & VT_BYREF) && this->pbstrVal)
			{
				return std::wstring(*this->pbstrVal, ::SysStringLen(*this->pbstrVal));
			}

			return L"";
		}

		Variant(std::wstring_view other) : Variant()
		{
			this->vt = VT_BSTR;
			this->bstrVal = ::SysAllocStringLen(other.data(), other.size());
			assert(this->bstrVal != other.data());
			assert(::SysStringLen(this->bstrVal) == other.size());
		}

		operator std::wstring_view() const
		{
			if (this->vt == VT_BSTR && this->bstrVal)
			{
				return std::wstring_view(this->bstrVal, ::SysStringLen(this->bstrVal));
			}

			if ((this->vt & VT_BSTR && this->vt & VT_BYREF) && this->pbstrVal)
			{
				return std::wstring_view(*this->pbstrVal, ::SysStringLen(*this->pbstrVal));
			}

			return L"";
		}

		Variant(IUnknown& object)
		{
			this->vt = VT_UNKNOWN;
			this->punkVal = &object;
			object.AddRef();
		}

		Variant(std::unique_ptr<IUnknown, void(*)(IUnknown*)>& object) : Variant(*object)
		{
		}

		operator std::unique_ptr<IUnknown, void(*)(IUnknown*)>() const
		{
			std::unique_ptr<IUnknown, void(*)(IUnknown*)> temp(nullptr, [](IUnknown* self) { self->Release(); });

			if (this->vt == VT_UNKNOWN && this->punkVal)
			{
				temp.reset(this->punkVal);
				temp->AddRef();
				return temp;
			}

			if ((this->vt & VT_UNKNOWN && this->vt & VT_BYREF) && this->ppunkVal)
			{
				temp.reset(*this->ppunkVal);
				temp->AddRef();
				return temp;
			}

			return temp;
		}

		Variant(IDispatch& object)
		{
			this->vt = VT_DISPATCH;
			this->pdispVal = &object;
			object.AddRef();
		}

		Variant(std::unique_ptr<IDispatch, void(*)(IDispatch*)>& object) : Variant(*object)
		{
		}

		operator std::unique_ptr<IDispatch, void(*)(IDispatch*)>() const
		{
			std::unique_ptr<IDispatch, void(*)(IDispatch*)> temp(nullptr, [](IDispatch* self) { self->Release(); });

			if (this->vt == VT_DISPATCH && this->pdispVal)
			{
				temp.reset(this->pdispVal);
				temp->AddRef();
				return temp;
			}

			if ((this->vt & VT_DISPATCH && this->vt & VT_BYREF) && this->ppdispVal)
			{
				temp.reset(*this->ppdispVal);
				temp->AddRef();
				return temp;
			}

			return temp;
		}

		Variant(std::unique_ptr<IStream, void(*)(IStream*)>& object) : Variant(*object)
		{

		}

		operator std::unique_ptr<IStream, void(*)(IStream*)>() const
		{
			std::unique_ptr<IStream, void(*)(IStream*)> temp(nullptr, [](IStream* self) { self->Release(); });

			if (this->vt == VT_UNKNOWN && this->punkVal)
			{
				IStream* raw = nullptr;
				if (this->punkVal->QueryInterface<IStream>(&raw) == S_OK)
				{
					temp.reset(raw);
				}

				return temp;
			}

			if ((this->vt & VT_UNKNOWN && this->vt & VT_BYREF) && this->ppunkVal)
			{
				IStream* raw = nullptr;

				if ((*this->ppunkVal)->QueryInterface<IStream>(&raw) == S_OK)
				{
					temp.reset(raw);
				}

				return temp;
			}

			return temp;
		}
	};

	inline std::string to_string(const Variant& variant)
	{
		switch (variant.vt)
		{
		case VT_NULL:
		case VT_EMPTY:
		{
			return std::string();
		}
		case VT_BOOL:
		{
			return variant.boolVal == 1 ? "true" : "false";
		}
		case VT_INT:
		{
			return std::to_string(variant.intVal);
		}
		case VT_UINT:
		{
			return std::to_string(variant.uintVal);
		}
		case VT_I2:
		{
			return std::to_string(variant.iVal);
		}
		case VT_UI2:
		{
			return std::to_string(variant.uiVal);
		}
		case VT_I4:
		{
			return std::to_string(variant.lVal);
		}
		case VT_UI4:
		{
			return std::to_string(variant.lVal);
		}
		case VT_R4:
		{
			return std::to_string(variant.fltVal);
		}
		case VT_R8:
		{
			return std::to_string(variant.dblVal);
		}
		case VT_BSTR:
		{
			std::wstring_view wide_temp(variant.bstrVal, ::SysStringLen(variant.bstrVal));
			std::string narrow_temp(wide_temp.size(), '\0');
			std::transform(wide_temp.begin(), wide_temp.end(), narrow_temp.begin(), [](auto wide_char) { return static_cast<char>(wide_char); });
			return narrow_temp;
		}
		default:
		{
			return std::string();
		}
		}
	}
}

#endif