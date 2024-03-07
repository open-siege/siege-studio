#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcontrolbars.h>

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>                      // MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>                     // MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <cassert>
#include <array>

#pragma warning(disable: 4311)

template<typename TClass>
TClass* AllocateClass()
{
	return new TClass();
}

template<typename TClass>
BOOL CreateClass(TClass* instance, CREATESTRUCTW* params, HWND wrapper)
{
	RECT position{params->x, params->y, params->cx + params->x, params->cy + params->y};
	return instance->Create(params->style, position, CWnd::FromHandlePermanent(params->hwndParent), reinterpret_cast<UINT>(params->hMenu));
}

template<>
BOOL CreateClass(CVSListBox* instance, CREATESTRUCTW* params, HWND wrapper)
{
	RECT position{params->x, params->y, params->cx + params->x, params->cy + params->y};
	auto result = instance->Create(params->lpszName, params->style, position, CWnd::FromHandlePermanent(params->hwndParent), reinterpret_cast<UINT>(params->hMenu));

	if (result)
	{
		instance->SetStandardButtons();
	}

	return result;
}

template<>
BOOL CreateClass(CMFCRibbonBar* instance, CREATESTRUCTW* params, HWND wrapper)
{
	return instance->Create(CWnd::FromHandlePermanent(params->hwndParent), reinterpret_cast<UINT>(params->hMenu));
}

template<>
BOOL CreateClass(CButton* instance, CREATESTRUCTW* params, HWND wrapper)
{
	RECT position{params->x, params->y, params->cx + params->x, params->cy + params->y};
	return instance->Create(params->lpszName, params->style, position, CWnd::FromHandlePermanent(params->hwndParent), reinterpret_cast<UINT>(params->hMenu));
}

template<>
BOOL CreateClass(CMFCMenuButton* instance, CREATESTRUCTW* params, HWND wrapper)
{
	return CreateClass(static_cast<CButton*>(instance), params, wrapper);
}

template<>
BOOL CreateClass(CMFCColorButton* instance, CREATESTRUCTW* params, HWND wrapper)
{
	return CreateClass(static_cast<CButton*>(instance), params, wrapper);
}

template<>
BOOL CreateClass(CMFCButton* instance, CREATESTRUCTW* params, HWND wrapper)
{
	return CreateClass(static_cast<CButton*>(instance), params, wrapper);
}

template<typename TClass>
LRESULT ProcessMessage(TClass* control, UINT message, WPARAM wparam, LPARAM lparam)
{
	return ::SendMessageW(*control, message, wparam, lparam);
}

template<>
LRESULT ProcessMessage(CVSListBox* control, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (message == LB_INSERTSTRING || message == LB_ADDSTRING)
	{
		wparam = message == LB_ADDSTRING ? -1 : wparam;

		return control->AddItem(CString(reinterpret_cast<wchar_t*>(lparam)), 0, int(wparam));
	}

	if (message == LB_SETITEMDATA)
	{
		//TODO call SetItemData
	}

	if (message == LB_GETITEMDATA)
	{
		//TODO call GetItemData
	}

	if (message == LB_GETCOUNT)
	{
		return LRESULT(control->GetCount());
	}

	if (message == LB_GETSEL)
	{
		// TODO call GetSelItem
	}

	if (message == LB_GETSELCOUNT)
	{
		return LB_ERR;
	}

	if (message == LB_SETCURSEL)
	{
		// TODO call SelectItem
	}

	if (message == LB_GETTEXT)
	{
		// TODO call GetItemText
	}

	if (message == LB_GETTEXTLEN)
	{
		// TODO call GetItemText
	}

	if (message == LB_DELETESTRING)
	{
		//TODO call RemoveItem
	}

	if (message == LVM_EDITLABELW)
	{
		if (control->EditItem(int(wparam)))
		{
			auto result = ::SendMessageW(::GetWindow(*control, GW_CHILD), LVM_GETEDITCONTROL, 0, 0);
			return result;		
		}

		return 0;
	}

	return ::SendMessageW(*control, message, wparam, lparam);
}

struct parent_wrapper
		{
			static LRESULT OwnerDrawProc(HWND hWnd,
					  UINT uMsg,
					  WPARAM wParam,
					  LPARAM lParam,
					  UINT_PTR uIdSubclass,
					  DWORD_PTR dwRefData)
			{
					if (uIdSubclass == WM_DRAWITEM)
					{
						if (uMsg == WM_MEASUREITEM || 
							uMsg == WM_DRAWITEM || 
							uMsg == WM_COMPAREITEM ||
							uMsg == WM_DELETEITEM)
						{
							HWND child = GetDlgItem(hWnd, int(wParam));

							if (child != nullptr && ::SendMessageW(child, uMsg, wParam, lParam))
							{
								return TRUE;
							}				
						}
						else if (uMsg == WM_NCDESTROY)
						{	
							RemoveWindowSubclass(hWnd, OwnerDrawProc, WM_DRAWITEM);
						}
					}

					return DefSubclassProc(hWnd, uMsg, wParam, lParam);
			}
		
		static LRESULT CwndProc(HWND hWnd,
					  UINT uMsg,
					  WPARAM wParam,
					  LPARAM lParam,
					  UINT_PTR uIdSubclass,
					  DWORD_PTR dwRefData)
		{
					if (DWORD_PTR(uIdSubclass) == dwRefData && uMsg == WM_NCDESTROY)
					{
						CWnd* self = (CWnd*)dwRefData;
						self->Detach();
						delete self;

						RemoveWindowSubclass(hWnd, CwndProc, uIdSubclass);
					}

					return DefSubclassProc(hWnd, uMsg, wParam, lParam);
			}
		};

template<typename TClass>
struct class_wrapper
{
	static LRESULT WndProc(HWND self, UINT message, WPARAM wparam, LPARAM lparam)
			{
//				OutputDebugStringW(L"class_wrapper::WndProc Enter\n");
				AFX_MANAGE_STATE(AfxGetStaticModuleState());
//				OutputDebugStringW(L"class_wrapper::WndProc Afx State set\n");

				TClass* control = nullptr;

				if (message == WM_NCCREATE)
				{
					CREATESTRUCTW* data = (CREATESTRUCTW*)lparam;
					OutputDebugStringW(L"class_wrapper::WndProc non client create\n");
					control = AllocateClass<TClass>();

					
					OutputDebugStringW(L"class_wrapper::WndProc new CCheckListBox\n");
					SetWindowLongPtrW(self, 0, (LONG_PTR)control);

					CWnd* parent = CWnd::FromHandlePermanent(data->hwndParent);

					if (parent == nullptr)
					{
						OutputDebugStringW(L"class_wrapper::WndProc Creating CWnd wrapper\n");

						parent = new CWnd();
						parent->Attach(data->hwndParent);

						SetWindowSubclass(data->hwndParent, parent_wrapper::CwndProc, UINT_PTR(parent), DWORD_PTR(parent));
						SetWindowSubclass(data->hwndParent, parent_wrapper::OwnerDrawProc, WM_DRAWITEM, 0);
						assert(CWnd::FromHandlePermanent(data->hwndParent));
					}

					return TRUE;
				}
				else
				{
					control = (TClass*)::GetWindowLongPtrW(self, 0);
				}

				if (control == nullptr)
				{
					OutputDebugStringW(L"class_wrapper::WndProc list is nul\n");
					return 0;
				}

				if (message == WM_CREATE)
				{
					OutputDebugStringW(L"class_wrapper::WndProc WM_CREATE\n");
					CREATESTRUCTW* data = (CREATESTRUCTW*)lparam;

					OutputDebugStringW(L"class_wrapper::WndProc CCheckListBox::Create");
					if(!CreateClass<TClass>(control, data, self))
					{
						delete control;
						SetWindowLongPtrW(self, 0, 0);
						return -1;
					}

					return 0;
				}

				if (message == WM_NCDESTROY)
				{
					OutputDebugStringW(L"class_wrapper::WndProc WM_NCDESTROY\n");
					auto result = ::SendMessageW(*control, message, wparam, lparam);
					delete control;
					SetWindowLongPtrW(self, 0, 0);
					return result;
				}

		if (message == WM_WINDOWPOSCHANGED || message == WM_SIZE || message == WM_MOVE)
		{
			DefWindowProc(self, message, wparam, lparam);
		}

		if (message == WM_WINDOWPOSCHANGED)
		{
			auto* windowPos = (WINDOWPOS*)lparam;
			control->SetWindowPos(CWnd::FromHandle(windowPos->hwndInsertAfter), windowPos->x, windowPos->y, windowPos->cx, windowPos->cy, windowPos->flags); 
			return 0;
		}

		if (message == WM_SIZE)
		{
			UINT width = LOWORD(lparam);
			UINT height = HIWORD(lparam);

			control->SetWindowPos(CWnd::FromHandle(nullptr), 0, 0, width, height, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING); 

			return 0;
		}

		if (message == WM_MOVE)
		{
			UINT x = LOWORD(lparam);
			UINT y = HIWORD(lparam);	
			control->SetWindowPos(CWnd::FromHandle(nullptr), x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING); 

			return 0;
		}

		return ProcessMessage(control, message, wparam, lparam);
	}

	static WNDCLASSEXW* CreateDescriptor(HINSTANCE instance, wchar_t* classname)
	{
		static WNDCLASSEXW result{};

		result.cbSize = sizeof(WNDCLASSEXW);
		result.lpfnWndProc = WndProc;
		result.hInstance = instance;
		result.lpszClassName = classname;
		result.cbWndExtra = sizeof(void*);

		return &result;
	}
};


struct CMFCLibrary : public CWinApp
{
	BOOL InitInstance() override
	{
		OutputDebugStringW(L"CMFCLibrary::InitInstance enter\n");
		
		bool result = true;

		
		// common controls
		::RegisterClassExW(class_wrapper<CButton>::CreateDescriptor(this->m_hInstance, L"Mfc::CButton"));
		::RegisterClassExW(class_wrapper<CComboBox>::CreateDescriptor(this->m_hInstance, L"Mfc::CComboBox"));
		::RegisterClassExW(class_wrapper<CDateTimeCtrl>::CreateDescriptor(this->m_hInstance, L"Mfc::CDateTimeCtrl"));
		::RegisterClassExW(class_wrapper<CEdit>::CreateDescriptor(this->m_hInstance, L"Mfc::CEdit"));
		::RegisterClassExW(class_wrapper<CComboBoxEx>::CreateDescriptor(this->m_hInstance, L"Mfc::CComboBoxEx"));
		::RegisterClassExW(class_wrapper<CListBox>::CreateDescriptor(this->m_hInstance, L"Mfc::CListBox"));
		::RegisterClassExW(class_wrapper<CListCtrl>::CreateDescriptor(this->m_hInstance, L"Mfc::CListCtrl"));

		
		
		// extended controls
		// list boxes
		::RegisterClassExW(class_wrapper<CDragListBox>::CreateDescriptor(this->m_hInstance, L"Mfc::CDragListBox"));
		::RegisterClassExW(class_wrapper<CVSListBox>::CreateDescriptor(this->m_hInstance, L"Mfc::CVSListBox"));
		::RegisterClassExW(class_wrapper<CCheckListBox>::CreateDescriptor(this->m_hInstance, L"Mfc::CCheckListBox"));
		
		
		// buttons
		::RegisterClassExW(class_wrapper<CMFCMenuButton>::CreateDescriptor(this->m_hInstance, L"Mfc::CMFCMenuButton"));
		::RegisterClassExW(class_wrapper<CMFCButton>::CreateDescriptor(this->m_hInstance, L"Mfc::CMFCButton"));
		::RegisterClassExW(class_wrapper<CMFCColorButton>::CreateDescriptor(this->m_hInstance, L"Mfc::CMFCColorButton"));
		
		
		// lists
		::RegisterClassExW(class_wrapper<CMFCPropertyGridCtrl>::CreateDescriptor(this->m_hInstance, L"Mfc::CMFCPropertyGridCtrl"));


		// TODO to fix
		::RegisterClassExW(class_wrapper<CMFCRibbonBar>::CreateDescriptor(this->m_hInstance, L"Mfc::CMFCRibbonBar"));

		return CWinApp::InitInstance();
	}

	int ExitInstance() override
	{
		OutputDebugStringW(L"CMFCLibrary::ExitInstance unregistering class\n");
		
		if (!::UnregisterClassW(L"Mfc::CCheckListBox", m_hInstance))
		{
			OutputDebugStringW(L"CMFCLibrary::ExitInstance could not unregister Mfc::CCheckListBox\n");			
		}

		return CWinApp::ExitInstance();
	}
} library{};