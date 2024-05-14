#include "targetver.h"

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcontrolbars.h>
#include <afxcmn.h>    

#include <map>
#include <string>
#include <array>
#include <string_view>
#include <filesystem>
#include <set>
#include <cassert>
#include <memory>



#pragma warning(disable: 4311)


HANDLE GetParentHeap(HWND parent)
{
	auto heap = ::GetPropW(parent, L"MFC::ControlHeap");

	if (heap == nullptr)
	{
		heap = HeapCreate(0, sizeof(CWnd) * 32, 0);
	
		if (!heap)
		{
			heap = ::GetProcessHeap();
		}

		assert(heap);
		assert(::SetPropW(parent, L"MFC::ControlHeap", heap));
	}

	return heap;
}

template<typename TClass>
TClass* AllocateClass(HWND parent)
{
	auto heap = GetParentHeap(parent);
	
	if (!heap)
	{
		return nullptr;
	}
	
	auto space = ::HeapAlloc(heap, HEAP_ZERO_MEMORY, sizeof(TClass));

	if (!space)
	{
		return nullptr;
	}

	auto* result = new (space) TClass();
	return result;
}

template<typename TClass>
void DeallocateClass(HWND parent, TClass* self)
{
	if (self == nullptr)
	{
		return;
	}
	
	self->~TClass();
	auto heap = GetParentHeap(parent);
	::HeapFree(heap, 0, self);
}

template<typename TClass>
LRESULT ProcessMessage(TClass* control, UINT message, WPARAM wparam, LPARAM lparam)
{
	return DefSubclassProc(*control, message, wparam, lparam);
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

	return DefSubclassProc(*control, message, wparam, lparam);
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

	return DefSubclassProc(*control, message, wparam, lparam);
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
		
		
		if (uMsg == WM_COMPAREITEM)
		{
			HWND child = GetDlgItem(hWnd, int(wParam));

			if (child && !CWnd::FromHandlePermanent(child))
			{
				goto DefaultProc;
			}

			auto result = ::SendMessageW(child, uMsg + WM_REFLECT_BASE, wParam, lParam);

			if (result == 1 || result == -1)
			{
				return result;
			}

			return ::SendMessageW(child, uMsg, wParam, lParam);				
		}

		if (uMsg == WM_MEASUREITEM || 
			uMsg == WM_DRAWITEM || 
			uMsg == WM_DELETEITEM)
		{
			HWND child = GetDlgItem(hWnd, int(wParam));

			if (child && !CWnd::FromHandlePermanent(child))
			{
				goto DefaultProc;
			}

			if (child != nullptr && ::SendMessageW(child, uMsg, wParam, lParam))
			{
				return TRUE;
			}				
		}
		else if (uMsg == WM_NOTIFY)
		{	
			auto params = (NMHDR*)lParam;

			if (!CWnd::FromHandlePermanent(params->hwndFrom))
			{
				goto DefaultProc;
			}

			SendMessageW(params->hwndFrom, uMsg, wParam, lParam);		
		}
		else if (uMsg == WM_COMMAND)
		{	
			if (!CWnd::FromHandlePermanent((HWND)lParam))
			{
				goto DefaultProc;
			}

			SendMessageW((HWND)lParam, uMsg, wParam, lParam);
		}
		else if (uMsg == WM_NCDESTROY)
		{	
			RemoveWindowSubclass(hWnd, OwnerDrawProc, WM_DRAWITEM);
		}

		DefaultProc:

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
			OutputDebugStringW(L"parent_wrapper::CwndProc WM_NCDESTROY for ");

			thread_local std::array<wchar_t, 256> temp{};
			temp[::GetWindowTextLengthW(hWnd)] = 0;

			::GetWindowTextW(hWnd, temp.data(), 256);

			OutputDebugStringW(temp.data());
			OutputDebugStringW(L"\n");

			CWnd* self = (CWnd*)dwRefData;
			self->Detach();
			delete self;

			auto heap = ::GetPropW(hWnd, L"MFC::ControlHeap");

			if (heap && heap != ::GetProcessHeap())
			{
				assert(::HeapDestroy(heap));
				assert(::RemovePropW(hWnd, L"MFC::ControlHeap"));
				OutputDebugStringW(L"parent_wrapper::CwndProc Destroying control heap\n");
			}
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
BOOL SubclassHandle(TClass* instance, HWND handle)
{
	return instance->SubclassWindow(handle);
}

template<>
BOOL SubclassHandle(CVSListBox* instance, HWND handle)
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
	static LRESULT ChildSubClassProc(HWND hWnd,
					  UINT uMsg,
					  WPARAM wParam,
					  LPARAM lParam,
					  UINT_PTR uIdSubclass,
					  DWORD_PTR dwRefData)
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		TClass* control = (TClass*)dwRefData;

		if (!control)
		{
			return 0;
		}

		if (uMsg == WM_WINDOWPOSCHANGED)
		{
			auto* windowPos = (WINDOWPOS*)lParam;
			control->SetWindowPos(CWnd::FromHandle(windowPos->hwndInsertAfter), windowPos->x, windowPos->y, windowPos->cx, windowPos->cy, windowPos->flags);
		}

		if (uMsg == WM_NCDESTROY)
		{
			OutputDebugStringW(L"class_wrapper::ChildSubClassProc WM_NCDESTROY\n");
			SetWindowSubclass(*control, ChildSubClassProc, uIdSubclass, 0);
			RemoveWindowSubclass(*control, ChildSubClassProc, uIdSubclass);
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
			TClass* control = AllocateClass<TClass>(params->hwndParent);

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

				if (!IsMenu(newId))
				{
					SetWindowLongPtrW(hWnd, GWL_ID, DWORD_PTR(newId));
				}
			
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
					SetWindowSubclass(params->hwndParent, parent_wrapper::OwnerDrawProc, 0, 0);
					assert(CWnd::FromHandlePermanent(params->hwndParent));
				}

				if (SubclassHandle(control, hWnd))
				{
					SetWindowSubclass(*control, ChildSubClassProc, DWORD_PTR(control), DWORD_PTR(control));
			
					OutputDebugStringW(L"class_wrapper::Successfully subclassed window\n");
					return TRUE;				
				}

				OutputDebugStringW(L"class_wrapper::Counld not subclass window\n");
			}
		Failed:
			{
				DeallocateClass<TClass>(params->hwndParent, control);
				return FALSE;
			}
			
		}
		else if (uMsg == WM_NCDESTROY)
		{
			OutputDebugStringW(L"SuperProc WM_NCDESTROY\n");
			auto* control = CWnd::FromHandlePermanent(hWnd);
			if (!control)
			{
				return 0;
			}

			control->UnsubclassWindow();
			control->~CWnd();

			return 0;
		}

		return proc(hWnd, uMsg, wParam, lParam);
	}
};


template <typename TClass>
CRuntimeClass* GetRuntimeClass()
{
	if constexpr (std::is_same_v<CMFCPropertyGridCtrl, TClass>)
	{
		return RUNTIME_CLASS(CMFCPropertyGridCtrl);
	}
	else if constexpr (std::is_same_v<CVSListBox, TClass>)
	{
		return RUNTIME_CLASS(CVSListBox);
	}
	else
	{
		TClass instance;
		return instance.GetRuntimeClass();	
	}
}

std::set<ATOM>& GetRegisteredClasses()
{
	static std::set<ATOM> registeredClasses;

	return registeredClasses;
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

		GetRegisteredClasses().emplace(result);

		return result;
}


struct CMFCLibrary : public CWinAppEx
{
	BOOL InitInstance() override
	{
		OutputDebugStringW(L"CMFCLibrary::InitInstance enter\n");
	
		bool result = true;

		RegisterMFCClass<CVSListBox>();
		RegisterMFCClass<CCheckListBox>();
		RegisterMFCClass<CMFCButton>();
		RegisterMFCClass<CMFCColorButton>();
		RegisterMFCClass<CMFCLinkCtrl>();
		RegisterMFCClass<CMFCPropertyGridCtrl>();
		RegisterMFCClass<CMFCListCtrl>();
		RegisterMFCClass<CMFCHeaderCtrl>();
		RegisterMFCClass<CMFCSpinButtonCtrl>();		
		RegisterMFCClass<CMFCTabCtrl>();
		RegisterMFCClass<CMFCToolTipCtrl>();

		return CWinApp::InitInstance();
	}

	int ExitInstance() override
	{
		OutputDebugStringW(L"CMFCLibrary::ExitInstance unregistering classes\n");
		
		for (auto atom : GetRegisteredClasses())
		{
			assert(::UnregisterClassW(MAKEINTATOM(atom), AfxGetInstanceHandle()));
		}

		return CWinApp::ExitInstance();
	}
} library{};