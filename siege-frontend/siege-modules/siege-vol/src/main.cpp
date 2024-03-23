#include <win32_controls.hpp>
#include <win32_builders.hpp>
#include <bit>
#include <filesystem>
#include <cassert>
#include <atomic>


//CreateStreamOverRandomAccessStream 
//CreateStreamOnHGlobal 
//SHCreateMemStream 
//CreateILockBytesOnHGlobal 
//StgCreateDocfileOnILockBytes
//CreateFile2 
//CreateFile2FromAppW 
//OpenFileMappingFromApp 
//MapViewOfFile3FromApp
//MapViewOfFileFromApp
//CreateFileMappingFromApp
//MFCreateCollection 
//MFCreateMFByteStreamOnStreamEx 
//IWICStream 
//CLSID_WICImagingFactory
//CreateXmlReader
//CreateXmlWriter

struct OleVariant
{
    VARIANT variant;

    OleVariant() noexcept
    {
        VariantInit(&variant);
    }

    ~OleVariant() noexcept
    {
        VariantClear(&variant);
    }

};

struct IEnumerable : IDispatch
{
    std::expected<std::unique_ptr<IEnumVARIANT, void(*)(IEnumVARIANT*)>, HRESULT> NewEnum()
    {
        DISPPARAMS dp = {nullptr, nullptr, 0, 0};
        VARIANT result;
        auto hresult = Invoke(DISPID_NEWENUM, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET | DISPATCH_METHOD, &dp, &result, nullptr, nullptr);
        
        auto temp = std::unique_ptr<IEnumVARIANT, void(*)(IEnumVARIANT*)>(nullptr, [](auto* self) {
                if (self)
                {
                    self->Release();
                }
            });

        if (hresult == S_OK && (result.vt == VT_DISPATCH || result.vt == VT_UNKNOWN))
        {
            IEnumVARIANT* self = nullptr;
            result.punkVal->QueryInterface(IID_IEnumVARIANT, (void**)&self);

            temp.reset(self);
            return temp;
        }

        return std::unexpected(hresult);

    }
};

struct IReadOnlyCollection : IEnumerable
{
    std::expected<std::uint32_t, HRESULT> Count()
    {
        //GetIDsOfNames
        //Invoke

        return std::unexpected(0);
    }

    std::expected<VARIANT, HRESULT> Item(std::uint32_t )
    {
        //DISPID_VALUE
        return std::unexpected(0);
    }
};


struct ICollection : IReadOnlyCollection
{
    std::expected<VARIANT, HRESULT> Add(VARIANT)
    {
        //GetIDsOfNames
        //Invoke
        return std::unexpected(0);
    }

    std::expected<VARIANT, HRESULT> Remove(std::uint32_t)
    {
        //GetIDsOfNames
        //Invoke

        return std::unexpected(0);
    }
};


struct VectorCollection : IDispatch
{
    std::atomic_int refCount = 0;
    std::vector<OleVariant> items;
    ATOM countAtom = 0;
    ATOM addAtom = 0;
    ATOM removeAtom = 0;

    HRESULT __stdcall QueryInterface(const GUID& riid, void** ppvObj) noexcept override
    {
        if (IsEqualGUID(riid, IID_IUnknown) || IsEqualGUID(riid, IID_IDispatch))
        {
            AddRef();
            *ppvObj = this;
        }

        return E_NOINTERFACE;
    }

    [[maybe_unused]] ULONG __stdcall AddRef() noexcept override
    {
        countAtom  = AddAtomW(L"Count");
        addAtom = AddAtomW(L"Add");
        removeAtom = AddAtomW(L"Remove");
        return ++refCount;
    }

    [[maybe_unused]] ULONG __stdcall Release() noexcept override
    {
        if (refCount == 0)
        {
            return 0;
        }

        --refCount;

        DeleteAtom(countAtom);
        DeleteAtom(addAtom);
        DeleteAtom(removeAtom);

        if (refCount == 0)
        {
            items.clear();
        }

        return refCount;
    }

    HRESULT __stdcall GetIDsOfNames(const GUID& riid, wchar_t **rgszNames, UINT cNames, LCID  lcid, DISPID  *rgDispId) noexcept override
    {
        assert(IsEqualGUID(riid, IID_NULL));

        if (cNames >= 0)
        {
            assert(rgszNames);

            auto atom = FindAtomW(rgszNames[0]);

            if (atom)
            {
                *rgDispId = atom;
                return S_OK;
            }
    
            std::wstring_view temp = rgszNames[0];

            constexpr static auto NewEnum = std::wstring_view(L"_NewEnum");

            if (CompareStringW(lcid, NORM_IGNORECASE, NewEnum.data(), NewEnum.size(), temp.data(), temp.size()) == CSTR_EQUAL)
            {    
                *rgDispId = DISPID_NEWENUM;
                return S_OK;
            }

            constexpr static auto Item = std::wstring_view(L"Item");
            
            if (CompareStringW(lcid, NORM_IGNORECASE, Item.data(), Item.size(), temp.data(), temp.size()) == CSTR_EQUAL)
            {
                *rgDispId = DISPID_VALUE;
                return S_OK;
            }

            *rgDispId = DISPID_UNKNOWN;
        }

        return DISP_E_UNKNOWNNAME;
    }

    HRESULT __stdcall GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) noexcept override
    {
        assert(ppTInfo);
        *ppTInfo = nullptr;
        return S_OK;
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

        OleVariant temp;

        if (dispIdMember == addAtom && wFlags & DISPATCH_METHOD)
        {
            if (pDispParams->cArgs != 1)
            {
                return DISP_E_BADPARAMCOUNT;
            }

            auto changed = VariantChangeType(&temp.variant, pDispParams->rgvarg, 0, VT_UNKNOWN);

            if (changed != S_OK)
            {
                *puArgErr = 0;
                return changed;
            }

            return Add(temp.variant, *pVarResult);
        }

        if (dispIdMember == removeAtom && wFlags & DISPATCH_METHOD)
        {
            if (pDispParams->cArgs != 1)
            {
                return DISP_E_BADPARAMCOUNT;
            }

            auto changed = VariantChangeType(&temp.variant, pDispParams->rgvarg, 0, VT_UI4);

            if (changed != S_OK)
            {
                *puArgErr = 0;
                return changed;
            }

            return Remove(temp.variant, *pVarResult);
        }

        if (dispIdMember == countAtom && (wFlags & DISPATCH_METHOD || wFlags & DISPATCH_PROPERTYGET))
        {
            return Count(*pVarResult);
        }

        if (dispIdMember == DISPID_VALUE && wFlags & DISPATCH_METHOD)
        {
            if (pDispParams->cArgs != 1)
            {
                return DISP_E_BADPARAMCOUNT;
            }

            auto changed = VariantChangeType(&temp.variant, pDispParams->rgvarg, 0, VT_UI4);

            if (changed != S_OK)
            {
                *puArgErr = 0;
                return changed;
            }

            return Item(temp.variant, *pVarResult);
        }

        if (dispIdMember == DISPID_NEWENUM && (wFlags & DISPATCH_METHOD || wFlags & DISPATCH_PROPERTYGET))
        {
            return NewEnum(*pVarResult);
        }

        return DISP_E_MEMBERNOTFOUND;
    }

    HRESULT Add(VARIANTARG& value, VARIANT& result) noexcept
    {
        auto& newItem = items.emplace_back();
        result.vt = VT_EMPTY;
        return VariantCopy(&newItem.variant, &value);
    }

    HRESULT Remove(VARIANTARG& value, VARIANT& result) noexcept
    {
        auto begin = items.begin() + result.uintVal;
        items.erase(begin);
        result.vt = VT_EMPTY;
        return S_OK;
    }

    HRESULT Count(VARIANT& result) noexcept
    {
        result.vt = VT_I4;
        result.intVal = int(items.size());
        return S_OK;
    }

    HRESULT Item(VARIANTARG& value, VARIANT& result) noexcept
    {
        std::size_t index = std::size_t(value.uintVal); 

        if (items.size() > index)
        {
            return VariantCopy(&result, &items[index].variant);
        }

        return S_OK;
    }

    HRESULT NewEnum(VARIANT& result) noexcept
    {
        return S_OK;
    }
};


struct volume_window
{
    constexpr static std::u8string_view formats = 
            u8".vol .rmf .mis .rmf .map .rbx .tbv .zip .vl2 .pk3 .iso .mds .cue .nrg .7z .tgz .rar .cab .z .cln .atd";

    win32::hwnd_t self;

    volume_window(win32::hwnd_t self, const CREATESTRUCTW&) : self(self)
	{
	}

    auto on_create(const win32::create_message& data)
    {
        auto parent_size = win32::GetClientRect(self);

        auto root = win32::CreateWindowExW(win32::window_params<>{
            .parent = self,
            .class_name = win32::rebar::class_Name,
            .style{win32::window_style(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
                                        WS_CLIPCHILDREN | CCS_TOP | 
                                        CCS_VERT | RBS_AUTOSIZE | RBS_FIXEDORDER | RBS_VERTICALGRIPPER) }
        });

        assert(root);

        auto rebar = win32::CreateWindowExW(win32::window_params<>{
            .parent = *root,
            .class_name = win32::rebar::class_Name,
            .style{win32::window_style(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
                WS_CLIPCHILDREN | RBS_VARHEIGHT | RBS_BANDBORDERS)}
        });

        auto toolbar = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | CCS_NOPARENTALIGN | CCS_NORESIZE | TBSTYLE_LIST,   
						}, *rebar, win32::tool_bar::class_name, L"Toolbar");

        assert(toolbar);

        win32::tool_bar::SetExtendedStyle(*toolbar, win32::tool_bar::mixed_buttons | win32::tool_bar::draw_drop_down_arrows);
     
        std::array<TBBUTTON, 2> buttons{{
              TBBUTTON{.iBitmap = I_IMAGENONE, .idCommand = 0, .fsState = TBSTATE_ENABLED, 
                                .fsStyle = BTNS_BUTTON | BTNS_SHOWTEXT, .iString = INT_PTR(L"Open")},
              TBBUTTON{.iBitmap = I_IMAGENONE, .idCommand = 1, .fsState = TBSTATE_ENABLED, 
                                .fsStyle = BTNS_DROPDOWN | BTNS_SHOWTEXT, .iString = INT_PTR(L"Extract")},
         }};

         if (!win32::tool_bar::AddButtons(*toolbar, buttons))
         {
            DebugBreak();       
         }

         auto button_size = win32::tool_bar::GetButtonSize(*toolbar);

        assert(rebar);
        win32::rebar::InsertBand(*rebar, -1, REBARBANDINFOW{
            .fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS,
            .lpText = const_cast<wchar_t*>(L""),
            .hwndChild = *toolbar,
            .cxMinChild = UINT(button_size.cx * 2),
            .cyMinChild = UINT(button_size.cy),
            .cx = UINT(parent_size->right / 3 * 2),
            .cyChild = UINT(button_size.cy)
            });

        win32::rebar::InsertBand(*rebar, -1, REBARBANDINFOW{
            .fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS,
            .lpText = const_cast<wchar_t*>(L"Search"),
            .hwndChild = [&]{
              auto search = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_BORDER | WS_EX_STATICEDGE | WS_CHILD
						}, *rebar, win32::edit::class_name, L"");

                assert(search);
                win32::edit::SetCueBanner(*search, false, L"Enter search text here");

                SendMessageW(*search, CCM_SETWINDOWTHEME , 0, reinterpret_cast<win32::lparam_t>(L"SearchBoxEdit"));
                return *search;
            }(),
            .cx = UINT(parent_size->right / 8)
            });


        auto table = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_VISIBLE | WS_CHILD | LVS_REPORT,
				//		.x = 0,       
				//		.y = short(win32::GetClientRect(*rebar)->bottom + 2),
				//		.cx = short(parent_size->right),  
				//		.cy = short(parent_size->bottom - 2 - win32::rebar::GetBarHeight(*rebar))       
						}, *root, win32::list_view::class_name, L"Volume");

        // TODO: make table columns have a split button
         win32::list_view::InsertColumn(*table, -1, LVCOLUMNW {
                .cx = LVSCW_AUTOSIZE,
                .pszText = const_cast<wchar_t*>(L"Filename"),
                .cxMin = parent_size->right / 10
        });

        win32::list_view::InsertColumn(*table, -1, LVCOLUMNW {
                .cx = LVSCW_AUTOSIZE,
                .pszText = const_cast<wchar_t*>(L"Path"),
                .cxMin = parent_size->right / 10,
        });

        win32::list_view::InsertColumn(*table, -1, LVCOLUMNW {
                .cx = LVSCW_AUTOSIZE,
                .pszText = const_cast<wchar_t*>(L"Size (in bytes)"),
                .cxMin = parent_size->right / 10,
        });

        win32::list_view::InsertColumn(*table, -1, LVCOLUMNW {
              .cx = LVSCW_AUTOSIZE,
              .pszText = const_cast<wchar_t*>(L"Compression Method"),
              .cxMin = parent_size->right / 10
        });


        auto min_height = parent_size->bottom / 10;
        auto min_width = parent_size->right / 2;

       // auto rebar_rect = win32::GetClientRect(*toolbar);

        win32::rebar::InsertBand(*root, -1, REBARBANDINFOW {
            .fStyle = RBBS_NOGRIPPER | RBBS_HIDETITLE | RBBS_TOPALIGN,
            .hwndChild = *rebar,  
            .cxMinChild = UINT(button_size.cy), // min height
            .cyMinChild = UINT(button_size.cx), // min width
            .cx = UINT(button_size.cy), // default height
   //         .cyChild = UINT(parent_size->right), // default width
            .cyChild = UINT(parent_size->right),
       //     .cxIdeal = UINT(parent_size->right - 20),
         //  .cx = UINT(parent_size->right - 100),
            });

        win32::rebar::InsertBand(*root, -1, REBARBANDINFOW {
            .fStyle = RBBS_NOGRIPPER | RBBS_HIDETITLE | RBBS_TOPALIGN,
            .hwndChild = *table,
            .cxMinChild = UINT(min_height), // min height
            .cyMinChild = UINT(parent_size->right), // min width
            .cx = UINT(parent_size->bottom - button_size.cy), // default height
            .cyChild = UINT(parent_size->right),
    //        .cxIdeal = UINT(parent_size->right - 20)
       //     .cx = UINT(parent_size->right - 100),
            });


    //    win32::rebar::MaximizeBand(*root, 0, parent_size->right - 100);
 //       win32::rebar::MaximizeBand(*root, 1, parent_size->right - 100);

        return 0;
    }

    auto on_notify(win32::toolbar_notify_message notification)
    {
        auto [sender, id, code] = notification;
        auto mapped_result = win32::MapWindowPoints(sender, HWND_DESKTOP, *win32::tool_bar::GetRect(sender, id));

        // TODO: create and add menu items
        win32::TrackPopupMenuEx(LoadMenuIndirectW(nullptr), 0, POINT{mapped_result->second.left, mapped_result->second.bottom}, sender, TPMPARAMS {
            .rcExclude = mapped_result->second
        });

        return 0;
    }


   auto on_size(win32::size_message sized)
	{
		win32::ForEachDirectChildWindow(self, [&](auto child) {            
            for (auto i = 0; i < win32::rebar::GetBandCount(child); ++i)
            {
                auto info = win32::rebar::GetBandChildSize(child, i);

                if (info)
                {
                    info->cyMinChild = sized.client_size.cx;
                    info->cyChild = sized.client_size.cx;
                    win32::rebar::SetBandInfo(child, i, std::move(*info));
                }
            }
		});

		return std::nullopt;
	}


    static bool is_bitmap(std::istream& raw_data)
    {
        return false;
    }
};

struct vol_module
{
    HINSTANCE module_instance;
    std::uint32_t is_supported_id;

    vol_module(win32::hwnd_t self, const CREATESTRUCTW& args)
    {
        module_instance = args.hInstance;
        win32::RegisterClassExW<volume_window>(WNDCLASSEXW{
            .hInstance = module_instance
            });

        SetPropW(self, win32::type_name<volume_window>().c_str(), std::bit_cast<void*>(volume_window::formats.data()));

        is_supported_id = RegisterWindowMessageW(L"is_supported_message");
    }

    ~vol_module()
    {
       win32::UnregisterClassW<volume_window>(module_instance);
    }
};

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpvReserved )  // reserved
{

    if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
    {
        if (lpvReserved != nullptr)
        {
            return TRUE; // do not do cleanup if process termination scenario
        }

        static win32::hwnd_t info_instance = nullptr;

        static std::wstring module_file_name(255, '\0');
        GetModuleFileNameW(hinstDLL, module_file_name.data(), module_file_name.size());

       std::filesystem::path module_path(module_file_name.data());


       if (fdwReason == DLL_PROCESS_ATTACH)
       {
           win32::RegisterStaticClassExW<vol_module>(WNDCLASSEXW{
                  .hInstance = hinstDLL,
                  .lpszClassName = module_path.stem().c_str()
           });

          info_instance = *win32::CreateWindowExW(CREATESTRUCTW{
                .hInstance = hinstDLL,
                .hwndParent = HWND_MESSAGE,
                .lpszClass = module_path.stem().c_str()
            });
        }
        else if (fdwReason == DLL_PROCESS_DETACH)
        {
            DestroyWindow(info_instance);
            UnregisterClassW(module_path.stem().c_str(), hinstDLL);
        }
    }

    return TRUE;
}