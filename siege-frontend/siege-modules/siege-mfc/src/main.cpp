#include "targetver.h"

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcontrolbars.h>
#include <afxcmn.h>    

#include <map>
#include <array>
#include <cassert>


#pragma warning(disable: 4311)

template<typename TClass>
TClass* AllocateClass()
{
	return new TClass();
}

template<typename TClass>
BOOL CreateClass(TClass* instance, CREATESTRUCTW* params)
{
	RECT position{params->x, params->y, params->cx + params->x, params->cy + params->y};
	return instance->Create(params->style, position, CWnd::FromHandlePermanent(params->hwndParent), reinterpret_cast<UINT>(params->hMenu));
}

template<>
BOOL CreateClass(CVSListBox* instance, CREATESTRUCTW* params)
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
BOOL CreateClass(CMFCRibbonBar* instance, CREATESTRUCTW* params)
{
	return instance->Create(CWnd::FromHandlePermanent(params->hwndParent), reinterpret_cast<UINT>(params->hMenu));
}

template<>
BOOL CreateClass(CButton* instance, CREATESTRUCTW* params)
{
	RECT position{params->x, params->y, params->cx + params->x, params->cy + params->y};
	return instance->Create(params->lpszName, params->style, position, CWnd::FromHandlePermanent(params->hwndParent), reinterpret_cast<UINT>(params->hMenu));
}

template<>
BOOL CreateClass(CMFCMenuButton* instance, CREATESTRUCTW* params)
{
	return CreateClass(static_cast<CButton*>(instance), params);
}

template<>
BOOL CreateClass(CMFCColorButton* instance, CREATESTRUCTW* params)
{
	return CreateClass(static_cast<CButton*>(instance), params);
}

template<>
BOOL CreateClass(CMFCButton* instance, CREATESTRUCTW* params)
{
	return CreateClass(static_cast<CButton*>(instance), params);
}

template<>
BOOL CreateClass(CMFCLinkCtrl* instance, CREATESTRUCTW* params)
{
	return CreateClass(static_cast<CButton*>(instance), params);
}

template<>
BOOL CreateClass(CSplitButton* instance, CREATESTRUCTW* params)
{
	return CreateClass(static_cast<CButton*>(instance), params);
}

template<>
BOOL CreateClass(CMFCColorPickerCtrl* instance, CREATESTRUCTW* params)
{
	return CreateClass(static_cast<CButton*>(instance), params);
}

template<>
BOOL CreateClass(CMFCTabCtrl* instance, CREATESTRUCTW* params)
{
	RECT position{params->x, params->y, params->cx + params->x, params->cy + params->y};
	return instance->Create(CMFCTabCtrl::Style(params->style), position, CWnd::FromHandlePermanent(params->hwndParent), reinterpret_cast<UINT>(params->hMenu));
}

template<>
BOOL CreateClass(CMFCOutlookBarTabCtrl* instance, CREATESTRUCTW* params)
{
	RECT position{params->x, params->y, params->cx + params->x, params->cy + params->y};
	return instance->Create(position, CWnd::FromHandlePermanent(params->hwndParent), reinterpret_cast<UINT>(params->hMenu));
}


template<>
BOOL CreateClass(CMFCReBar* instance, CREATESTRUCTW* params)
{
	return instance->Create(CWnd::FromHandlePermanent(params->hwndParent), params->style, reinterpret_cast<UINT>(params->hMenu));
}

template<typename TClass>
LRESULT ProcessMessage(TClass* control, UINT message, WPARAM wparam, LPARAM lparam)
{
	return AfxCallWndProc(control, *control, message, wparam, lparam);
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

	return AfxCallWndProc(control, *control, message, wparam, lparam);
}

template<>
LRESULT ProcessMessage(CMFCPropertyGridCtrl* control, UINT message, WPARAM wparam, LPARAM lparam)
{

	if (message == LVM_GETHEADER) 
	//LVM_GETITEM LVM_DELETEITEM LVM_INSERTITEM
	//LVM_INSERTGROUP LVM_GETGROUPINFO
	//LVM_GETCOLUMN LVM_GETHEADER LVM_ENABLEGROUPVIEW LVM_GETGROUPCOUNT
		//LVM_GETGROUPINFO
		//LVM_GETGROUPINFOBYINDEX
		//LVM_REMOVEGROUP
		//LVM_GETITEMCOUNT
	{
		//TODO call SetItemData
	}

	return ::SendMessageW(*control, message, wparam, lparam);
}

using ClassMap = std::map<ATOM, HWND(*)(CREATESTRUCTW* params)>;

auto& GetClassMap()
{
	static ClassMap classes;

	return classes;
}

 extern "C" __declspec(dllexport) HWND __stdcall CreateMFCWindow(CREATESTRUCTW* params) noexcept
{
	 if (params == nullptr)
	 {
		return nullptr;
	 }

	 if (params->lpszClass == nullptr)
	 {
		return nullptr;
	 }

	 ATOM result = FindAtomW(params->lpszClass);

	 if (result == 0)
	 {
		return nullptr;
	 }

	 auto findIter = GetClassMap().find(result);

	 if (findIter == GetClassMap().end())
	 {
		return nullptr;
	 }

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return findIter->second(params);
}


 ATOM RegisterMFCClass(wchar_t* classname, HWND(*creator)(CREATESTRUCTW* params))
 {
	ATOM result = AddAtomW(classname);
	GetClassMap()[result] = creator;
	return result;
 }

BOOL UnregisterMFCClass(ATOM classname)
{
	GetClassMap().erase(classname);
	return DeleteAtom(classname) == 0 ? TRUE : FALSE;
}


BOOL UnregisterMFCClass(wchar_t* classname)
{
	 if (classname == nullptr)
	 {
		return FALSE;
	 }

	ATOM result = FindAtomW(classname);
	
	if (result == 0)
	{
		return FALSE;
	}

	return UnregisterMFCClass(result);
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
				AFX_MANAGE_STATE(AfxGetStaticModuleState());
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


WNDCLASSEXW GetSystemClass(wchar_t* className)
{
	WNDCLASSEXW info{sizeof(WNDCLASSEXW)};

	assert(::GetClassInfoExW(nullptr, className, &info));

	return info;
}

template<typename TClass>
struct class_wrapper
{
	static LRESULT CwndProc(HWND hWnd,
					  UINT uMsg,
					  WPARAM wParam,
					  LPARAM lParam,
					  UINT_PTR uIdSubclass,
					  DWORD_PTR dwRefData)
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		TClass* control = (TClass*)dwRefData;

		if (uMsg == WM_WINDOWPOSCHANGED)
		{
			auto* windowPos = (WINDOWPOS*)lParam;
			control->SetWindowPos(CWnd::FromHandle(windowPos->hwndInsertAfter), windowPos->x, windowPos->y, windowPos->cx, windowPos->cy, windowPos->flags);
		}

		if (uMsg == WM_NCDESTROY)
		{

			OutputDebugStringW(L"class_wrapper::WndProc WM_NCDESTROY\n");
			auto result = ProcessMessage(control, uMsg, wParam, lParam);
			RemoveWindowSubclass(*control, CwndProc, uIdSubclass);
			delete control;
			return result;
		}

		return ProcessMessage(control, uMsg, wParam, lParam);
	}

	static HWND CreateInstance(CREATESTRUCTW* params)
	{
		TClass* control = AllocateClass<TClass>();

		CWnd* parent = CWnd::FromHandlePermanent(params->hwndParent);

		if (parent == nullptr)
		{
			OutputDebugStringW(L"class_wrapper::WndProc Creating CWnd wrapper\n");

			parent = new CWnd();
			parent->Attach(params->hwndParent);

			SetWindowSubclass(params->hwndParent, parent_wrapper::CwndProc, UINT_PTR(parent), DWORD_PTR(parent));
			SetWindowSubclass(params->hwndParent, parent_wrapper::OwnerDrawProc, WM_DRAWITEM, 0);
			assert(CWnd::FromHandlePermanent(params->hwndParent));
		}

		if (CreateClass<TClass>(control, params))
		{
			SetWindowSubclass(*control, CwndProc, UINT_PTR(control), DWORD_PTR(control));
			return *control;
		}
		delete control;
		return nullptr;
	}

	inline static WNDCLASSEXW ParentClassInfo{};

	static LRESULT SuperProc(HWND hWnd,
					  UINT uMsg,
					  WPARAM wParam,
					  LPARAM lParam)
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());

		auto proc = DefWindowProcW;

		if (ParentClassInfo.lpfnWndProc != nullptr)
		{
			proc = ParentClassInfo.lpfnWndProc;			
		}

		if (uMsg == WM_NCCREATE)
		{
			if (proc(hWnd, uMsg, wParam, lParam))
			{
				TClass* control = AllocateClass<TClass>();

				CREATESTRUCTW* params = reinterpret_cast<CREATESTRUCTW*>(lParam);
				CWnd* parent = CWnd::FromHandlePermanent(params->hwndParent);

				if (parent == nullptr)
				{
					OutputDebugStringW(L"class_wrapper::WndProc Creating CWnd wrapper\n");

					parent = new CWnd();
					parent->Attach(params->hwndParent);

					SetWindowSubclass(params->hwndParent, parent_wrapper::CwndProc, UINT_PTR(parent), DWORD_PTR(parent));
					SetWindowSubclass(params->hwndParent, parent_wrapper::OwnerDrawProc, WM_DRAWITEM, 0);
					assert(CWnd::FromHandlePermanent(params->hwndParent));
				}

				control->SubclassWindow(hWnd);
				//control->SetStandardButtons();
				SetWindowSubclass(*control, CwndProc, UINT_PTR(control), DWORD_PTR(control));
			
				return TRUE;
			}
		}

		return proc(hWnd, uMsg, wParam, lParam);
	}
};


struct CMFCLibrary : public CWinApp
{
	BOOL InitInstance() override
	{
		OutputDebugStringW(L"CMFCLibrary::InitInstance enter\n");
		
		bool result = true;

		
		WNDCLASSEXW classInfo = class_wrapper<CVSListBox>::ParentClassInfo = GetSystemClass(WC_STATICW);

		classInfo.hInstance = AfxGetInstanceHandle();
		classInfo.lpszClassName = L"Mfc::CVSListBox";
		classInfo.lpfnWndProc = class_wrapper<CVSListBox>::SuperProc;
		
		assert(::RegisterClassEx(&classInfo));

		classInfo = class_wrapper<CDragListBox>::ParentClassInfo = GetSystemClass(WC_LISTBOXW);

		classInfo.hInstance = AfxGetInstanceHandle();
		classInfo.lpszClassName = L"Mfc::CDragListBox";
		classInfo.lpfnWndProc = class_wrapper<CDragListBox>::SuperProc;
		
		assert(::RegisterClassEx(&classInfo));

		classInfo = class_wrapper<CListBox>::ParentClassInfo = GetSystemClass(WC_LISTBOXW);

		classInfo.hInstance = AfxGetInstanceHandle();
		classInfo.lpszClassName = L"Mfc::CListBox";
		classInfo.lpfnWndProc = class_wrapper<CListBox>::SuperProc;
		
		assert(::RegisterClassEx(&classInfo));

		classInfo = class_wrapper<CCheckListBox>::ParentClassInfo = GetSystemClass(WC_LISTBOXW);

		classInfo.hInstance = AfxGetInstanceHandle();
		classInfo.lpszClassName = L"Mfc::CCheckListBox";
		classInfo.lpfnWndProc = class_wrapper<CCheckListBox>::SuperProc;
		
		assert(::RegisterClassEx(&classInfo));

		// common controls
		::RegisterMFCClass(L"Mfc::CButton", class_wrapper<CButton>::CreateInstance);
		::RegisterMFCClass(L"Mfc::CSplitButton", class_wrapper<CSplitButton>::CreateInstance);
		::RegisterMFCClass(L"Mfc::CComboBox", class_wrapper<CComboBox>::CreateInstance);
		::RegisterMFCClass(L"Mfc::CDateTimeCtrl", class_wrapper<CDateTimeCtrl>::CreateInstance);
		::RegisterMFCClass(L"Mfc::CEdit", class_wrapper<CEdit>::CreateInstance);
		::RegisterMFCClass(L"Mfc::CComboBoxEx", class_wrapper<CComboBoxEx>::CreateInstance);
		::RegisterMFCClass(L"Mfc::CListBox", class_wrapper<CListBox>::CreateInstance);
		::RegisterMFCClass(L"Mfc::CListCtrl", class_wrapper<CListCtrl>::CreateInstance);

		// extended controls
		// list boxes
		::RegisterMFCClass(L"Mfc::CDragListBox", class_wrapper<CDragListBox>::CreateInstance);
		//::RegisterMFCClass(L"Mfc::CVSListBox", class_wrapper<CVSListBox>::CreateInstance);
		::RegisterMFCClass(L"Mfc::CCheckListBox", class_wrapper<CCheckListBox>::CreateInstance);
		
		// buttons
		::RegisterMFCClass(L"Mfc::CMFCMenuButton", class_wrapper<CMFCMenuButton>::CreateInstance);
		::RegisterMFCClass(L"Mfc::CMFCButton", class_wrapper<CMFCButton>::CreateInstance);
		::RegisterMFCClass(L"Mfc::CMFCColorButton", class_wrapper<CMFCColorButton>::CreateInstance);
		::RegisterMFCClass(L"Mfc::CMFCColorPickerCtrl", class_wrapper<CMFCColorPickerCtrl>::CreateInstance);
		::RegisterMFCClass(L"Mfc::CMFCLinkCtrl", class_wrapper<CMFCLinkCtrl>::CreateInstance);

		// edits
		::RegisterMFCClass(L"Mfc::CMFCEditBrowseCtrl", class_wrapper<CMFCEditBrowseCtrl>::CreateInstance);
		::RegisterMFCClass(L"Mfc::CMFCMaskedEdit", class_wrapper<CMFCMaskedEdit>::CreateInstance);
		
		// combo boxes
		::RegisterMFCClass(L"Mfc::CMFCFontComboBox", class_wrapper<CMFCFontComboBox>::CreateInstance);

		// lists
		::RegisterMFCClass(L"Mfc::CMFCPropertyGridCtrl", class_wrapper<CMFCPropertyGridCtrl>::CreateInstance);
		::RegisterMFCClass(L"Mfc::CMFCListCtrl", class_wrapper<CMFCListCtrl>::CreateInstance);
		::RegisterMFCClass(L"Mfc::CMFCShellListCtrl", class_wrapper<CMFCShellListCtrl>::CreateInstance);
		
		//trees
		::RegisterMFCClass(L"Mfc::CMFCShellTreeCtrl", class_wrapper<CMFCShellTreeCtrl>::CreateInstance);

		// spinners
		::RegisterMFCClass(L"Mfc::CMFCSpinButtonCtrl", class_wrapper<CMFCSpinButtonCtrl>::CreateInstance);
		
		//headers
		::RegisterMFCClass(L"Mfc::CMFCHeaderCtrl", class_wrapper<CMFCHeaderCtrl>::CreateInstance);

		//rebars
		::RegisterMFCClass(L"Mfc::CMFCReBar", class_wrapper<CMFCReBar>::CreateInstance);

		//tabs
		::RegisterMFCClass(L"Mfc::CMFCTabCtrl", class_wrapper<CMFCTabCtrl>::CreateInstance);
		::RegisterMFCClass(L"Mfc::CMFCOutlookBarTabCtrl", class_wrapper<CMFCOutlookBarTabCtrl>::CreateInstance);

		// TODO to fix
		::RegisterMFCClass(L"Mfc::CMFCRibbonBar", class_wrapper<CMFCRibbonBar>::CreateInstance);

		return CWinApp::InitInstance();
	}

	int ExitInstance() override
	{
		OutputDebugStringW(L"CMFCLibrary::ExitInstance unregistering class\n");
		
		auto& classes = GetClassMap();

		for (auto& item : classes)
		{
			UnregisterMFCClass(item.first);
		}

		return CWinApp::ExitInstance();
	}
} library{};