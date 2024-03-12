#include "targetver.h"

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcontrolbars.h>
#include <afxcmn.h>    

#include <map>
#include <string>
#include <array>
#include <string_view>
#include <cassert>



#pragma warning(disable: 4311)

template<typename TClass>
TClass* AllocateClass()
{
	return new TClass();
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


WNDCLASSEXW GetSystemClass(const wchar_t* className)
{
	WNDCLASSEXW info{sizeof(WNDCLASSEXW)};

	assert(::GetClassInfoExW(GetModuleHandleW(L"comctrl32.dll"), className, &info));

	info.lpszClassName = className;
	return info;
}

WNDCLASSEXW GetSystemClass(const CRuntimeClass& info)
{
	static std::map<const CRuntimeClass*, WNDCLASSEXW> knownClasses = {
		{&CStatic::classCStatic, GetSystemClass(WC_STATICW)},
		{&CButton::classCButton, GetSystemClass(WC_BUTTONW)},
		{&CListBox::classCListBox, GetSystemClass(WC_LISTBOXW)},
		{&CComboBox::classCComboBox, GetSystemClass(WC_COMBOBOXW)},
		{&CEdit::classCEdit, GetSystemClass(WC_EDITW)},
		{&CScrollBar::classCScrollBar, GetSystemClass(WC_SCROLLBARW)},
		{&CListCtrl::classCListCtrl, GetSystemClass(WC_LISTVIEWW)},
		{&CTreeCtrl::classCTreeCtrl, GetSystemClass(WC_TREEVIEWW)},
		{&CTabCtrl::classCTabCtrl, GetSystemClass(WC_TABCONTROLW)},
		{&CHeaderCtrl::classCHeaderCtrl, GetSystemClass(WC_HEADERW)},
		{&CComboBoxEx::classCComboBoxEx, GetSystemClass(WC_COMBOBOXEXW)},
		{&CSpinButtonCtrl::classCSpinButtonCtrl, GetSystemClass(UPDOWN_CLASSW)},
		{&CReBar::classCReBar, GetSystemClass(REBARCLASSNAMEW)},
		{&CProgressCtrl::classCProgressCtrl, GetSystemClass(PROGRESS_CLASSW)},
		{&CToolTipCtrl::classCToolTipCtrl, GetSystemClass(TOOLTIPS_CLASSW)},
		{&CMonthCalCtrl::classCMonthCalCtrl, GetSystemClass(MONTHCAL_CLASSW)},
		{&CDateTimeCtrl::classCDateTimeCtrl, GetSystemClass(DATETIMEPICK_CLASSW)},
		{&CToolBarCtrl::classCToolBarCtrl, GetSystemClass(TOOLBARCLASSNAMEW)},
		{&CStatusBarCtrl::classCStatusBarCtrl, GetSystemClass(STATUSCLASSNAMEW)},
		{&CSliderCtrl::classCSliderCtrl, GetSystemClass(TRACKBAR_CLASS)},
	};

	if (info.IsDerivedFrom(RUNTIME_CLASS(CComboBoxEx)))
	{
		return knownClasses.at(RUNTIME_CLASS(CComboBoxEx));
	}

	for (auto& other : knownClasses)
	{
		if (info.IsDerivedFrom(other.first))
		{
			return other.second;
		}
	}

	WNDCLASSEXW fallback{sizeof(WNDCLASSEXW)};
	
	if (info.IsDerivedFrom(&CDialog::classCDialog))
	{
		fallback.lpfnWndProc = DefDlgProcW;
		fallback.cbWndExtra = DLGWINDOWEXTRA;
	}
	else
	{
		fallback.lpfnWndProc = DefWindowProcW;
		fallback.cbWndExtra = int(sizeof(void*));
	}

	fallback.lpszClassName = L"";

	return fallback;
}

template<typename TClass>
BOOL InitFromHandle(TClass* instance, HWND handle)
{
	return instance->SubclassWindow(handle);
}

template<>
BOOL InitFromHandle(CVSListBox* instance, HWND handle)
{
	if (instance->SubclassWindow(handle))
	{
		instance->SetStandardButtons();
	}

	return FALSE;
}

template<typename TClass>
BOOL PreCreateWindow(TClass* instance, CREATESTRUCT& cs)
{
	return static_cast<CWnd*>(instance)->PreCreateWindow(cs);
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
			OutputDebugStringW(L"class_wrapper::WndProc WM_WINDOWPOSCHANGED\n");
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
			CREATESTRUCTW* params = reinterpret_cast<CREATESTRUCTW*>(lParam);
			TClass* control = AllocateClass<TClass>();

			if (!PreCreateWindow(control, *params))
			{
				goto Failed;
			}

			auto newStyle = params->style;
			auto newExStyle = params->dwExStyle;
			auto newId = params->hMenu;


			if (proc(hWnd, uMsg, wParam, lParam))
			{
				OutputDebugStringW(L"class_wrapper::Allocating class ");

				SetWindowLongPtrW(hWnd, GWL_STYLE, newStyle);
				SetWindowLongPtrW(hWnd, GWL_EXSTYLE, newExStyle);
				SetWindowLongPtrW(hWnd, GWL_ID, DWORD_PTR(newId));
				
				OutputDebugStringA(control->GetRuntimeClass()->m_lpszClassName);
				OutputDebugStringW(L"(");
				OutputDebugStringW(ParentClassInfo.lpszClassName);
				OutputDebugStringW(L")");
				OutputDebugStringW(L"\n");

				
				CWnd* parent = CWnd::FromHandlePermanent(params->hwndParent);

				if (parent == nullptr)
				{
					OutputDebugStringW(L"class_wrapper::WndProc Creating parent CWnd wrapper\n");

					parent = new CWnd();
					parent->Attach(params->hwndParent);

					SetWindowSubclass(params->hwndParent, parent_wrapper::CwndProc, UINT_PTR(parent), DWORD_PTR(parent));
					SetWindowSubclass(params->hwndParent, parent_wrapper::OwnerDrawProc, WM_DRAWITEM, 0);
					assert(CWnd::FromHandlePermanent(params->hwndParent));
				}

				if (InitFromHandle(control, hWnd))
				{
					SetWindowSubclass(*control, CwndProc, UINT_PTR(control), DWORD_PTR(control));
			
					OutputDebugStringW(L"class_wrapper::Successfully subclassed window\n");
					return TRUE;				
				}

				OutputDebugStringW(L"class_wrapper::Counld not subclass window\n");
			}
		Failed:
			{
				delete control;
				return FALSE;
			}
			
		}

		return proc(hWnd, uMsg, wParam, lParam);
	}
};


template <typename TClass>
CRuntimeClass* GetRuntimeClass()
{
	if constexpr (std::is_same_v<CMFCRibbonBar, TClass>)
	{
		return RUNTIME_CLASS(CMFCRibbonBar);
	}
	else if constexpr (std::is_same_v<CMFCShellTreeCtrl, TClass>)
	{
		return RUNTIME_CLASS(CMFCShellTreeCtrl);
	}
	else if constexpr (std::is_same_v<CMFCShellListCtrl, TClass>)
	{
		return RUNTIME_CLASS(CMFCShellListCtrl);
	}
	else if constexpr (std::is_same_v<CMFCPropertyGridCtrl, TClass>)
	{
		return RUNTIME_CLASS(CMFCPropertyGridCtrl);
	}
	else if constexpr (std::is_same_v<CVSListBox, TClass>)
	{
		return RUNTIME_CLASS(CVSListBox);
	}
	else if constexpr (std::is_same_v<CMFCOutlookBarTabCtrl, TClass>)
	{
		return RUNTIME_CLASS(CMFCOutlookBarTabCtrl);
	}
	else
	{
		TClass instance;
		return instance.GetRuntimeClass();	
	}
}


template<typename TClass>
auto RegisterMFCClass(std::string className = std::string{})
{		
		CRuntimeClass* metaInfo = nullptr;

		if (className.empty())
		{
			metaInfo = GetRuntimeClass<TClass>();
			className = std::string(metaInfo->m_lpszClassName);
		}
		else
		{
			metaInfo = CRuntimeClass::FromName(className.c_str());
		}

		if (metaInfo == nullptr)
		{
			metaInfo = GetRuntimeClass<TClass>();
		}

		auto temp = L"MFC::" + std::wstring(className.begin(), className.end());
		WNDCLASSEXW classInfo = class_wrapper<TClass>::ParentClassInfo = GetSystemClass(*metaInfo);

		classInfo.hInstance = AfxGetInstanceHandle();
		
		OutputDebugStringW((L"Registering " + temp + L"(" + classInfo.lpszClassName + L")" +  L"\n").c_str());
		classInfo.lpszClassName = temp.c_str();
		classInfo.lpfnWndProc = class_wrapper<TClass>::SuperProc;
		
		auto result = ::RegisterClassEx(&classInfo);
		assert(result);

		return result;
}


struct CMFCLibrary : public CWinApp
{
	BOOL InitInstance() override
	{
		OutputDebugStringW(L"CMFCLibrary::InitInstance enter\n");
	
		bool result = true;

		// common controls
		RegisterMFCClass<CButton>();
		RegisterMFCClass<CSplitButton>();
		RegisterMFCClass<CComboBox>();
		RegisterMFCClass<CDateTimeCtrl>();
		RegisterMFCClass<CEdit>();
		RegisterMFCClass<CComboBoxEx>();
		RegisterMFCClass<CListBox>();
		RegisterMFCClass<CListCtrl>();
		RegisterMFCClass<CStatic>();
		RegisterMFCClass<CHeaderCtrl>();
		RegisterMFCClass<CToolBarCtrl>();
		RegisterMFCClass<CTreeCtrl>();
		RegisterMFCClass<CTabCtrl>();
		RegisterMFCClass<CStatusBarCtrl>();
		RegisterMFCClass<CNetAddressCtrl>();
		RegisterMFCClass<CIPAddressCtrl>();
		RegisterMFCClass<CPagerCtrl>();
		

		//extended controls
		//list boxes
		RegisterMFCClass<CDragListBox>();
		RegisterMFCClass<CVSListBox>();
		RegisterMFCClass<CCheckListBox>();
	

		//buttons
		RegisterMFCClass<CBitmapButton>();
		RegisterMFCClass<CMFCMenuButton>();
		RegisterMFCClass<CMFCButton>();
		RegisterMFCClass<CMFCColorButton>();
		RegisterMFCClass<CMFCColorPickerCtrl>("CMFCColorPickerCtrl");
		RegisterMFCClass<CMFCLinkCtrl>();

		////edits
		RegisterMFCClass<CMFCEditBrowseCtrl>();
		RegisterMFCClass<CMFCMaskedEdit>();
		////// combo boxes
		RegisterMFCClass<CMFCFontComboBox>("CMFCFontComboBox");

		////// lists
		RegisterMFCClass<CMFCPropertyGridCtrl>();
		RegisterMFCClass<CMFCListCtrl>();
		RegisterMFCClass<CMFCShellListCtrl>();

		//////trees
		RegisterMFCClass<CMFCShellTreeCtrl>();

		////// spinners
		RegisterMFCClass<CMFCSpinButtonCtrl>();
		////
		//////headers
		RegisterMFCClass<CMFCHeaderCtrl>();
		//
		//////rebars
		RegisterMFCClass<CMFCReBar>();		

		//////tabs
		RegisterMFCClass<CMFCTabCtrl>();
		RegisterMFCClass<CMFCOutlookBarTabCtrl>();

		//dialogs
		RegisterMFCClass<CMFCColorDialog>();

		//tooltips
		RegisterMFCClass<CMFCToolTipCtrl>();

		// status bars
		RegisterMFCClass<CMFCStatusBar>();


		////// TODO to fix
		RegisterMFCClass<CMFCPopupMenuBar>();
		RegisterMFCClass<CPaneDivider>();
		RegisterMFCClass<CSplitterWndEx>();
		RegisterMFCClass<CSplitterWnd>();
		RegisterMFCClass<CMFCCaptionBar>();
		RegisterMFCClass<CMFCAutoHideBar>();
		RegisterMFCClass<CMFCRibbonBar>();
		RegisterMFCClass<CDockablePane>();

		return CWinApp::InitInstance();
	}

	int ExitInstance() override
	{
		OutputDebugStringW(L"CMFCLibrary::ExitInstance unregistering class\n");
		
		// TODO remove all the registered classes

		return CWinApp::ExitInstance();
	}
} library{};