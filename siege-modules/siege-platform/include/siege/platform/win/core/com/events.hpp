#ifndef WIN32_COM_EVENTS_HPP
#define WIN32_COM_EVENTS_HPP

#include "win32_com.hpp"
#include "win32_com_enumerable.hpp"
#include <ocidl.h>
#include <olectl.h>

namespace win32::com
{
    struct ConnectData : ::CONNECTDATA
    {
        std::unique_ptr<IDispatch, void(*)(IDispatch*)> sink;

        ConnectData(std::unique_ptr<IDispatch, void(*)(IDispatch*)> sink, DWORD cookie) : 
            ::CONNECTDATA{static_cast<IUnknown*>(sink.get()), cookie}, sink(std::move(sink))
        {
        
        }
    };

    struct ConnectionPoint : ComObject, IConnectionPoint
    {
        std::unique_ptr<IConnectionPointContainer, void(*)(IConnectionPointContainer*)> container; 
        std::vector<ConnectData> callbacks;

        ConnectionPoint(std::unique_ptr<IConnectionPointContainer, void(*)(IConnectionPointContainer*)> container) : container(std::move(container))
        {
            callbacks.reserve(4);
        }

        HRESULT __stdcall QueryInterface(const GUID& riid, void** ppvObj) noexcept override
        {
            return ComQuery<IUnknown, IConnectionPoint>(*this, riid, ppvObj)
                .or_else([&]() { return ComQuery<IConnectionPoint>(*this, riid, ppvObj); })
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

        HRESULT Advise(IUnknown *pUnkSink, DWORD *pdwCookie) noexcept override
        {
            if (!(pUnkSink || pdwCookie))
            {
                return E_INVALIDARG;
            }

            IDispatch* dispatch = nullptr;

            if (pUnkSink->QueryInterface<IDispatch>(&dispatch) == S_OK)
            {
                DWORD cookie = callbacks.size() + 1;

                if (!callbacks.empty())
                {
                    cookie = callbacks.rbegin()->dwCookie + 1;
                }

                auto& back = callbacks.emplace_back(as_unique(dispatch), cookie);
                *pdwCookie = back.dwCookie;               
                return S_OK;
            }

            return CONNECT_E_CANNOTCONNECT;
        }

        HRESULT Unadvise(DWORD dwCookie) override
        {
            auto result = std::find_if(callbacks.begin(), callbacks.end(), [&](auto& callback) {
                    return callback.dwCookie == dwCookie;
            });

            if (result == callbacks.end())
            {
                return E_POINTER;
            }

            callbacks.erase(result);

            return S_OK;
        }

        HRESULT GetConnectionInterface(IID* pIID) override
        {
            if (!pIID)
            {
                return E_POINTER;
            }

            *pIID = __uuidof(IDispatch);

            return S_OK;
        }

        HRESULT GetConnectionPointContainer(IConnectionPointContainer** ppCPC) override
        {
            if (!ppCPC)
            {
                return E_POINTER;
            }

            container->AddRef();
            *ppCPC = container.get();

            return S_OK;
        }

        HRESULT EnumConnections(IEnumConnections** ppEnum) override
        {
            if (!ppEnum)
            {
                return E_POINTER;
            }

            auto enumerator = make_unique_range_enumerator<CONNECTDATA>(callbacks.begin(), callbacks.begin(), callbacks.end());
            *ppEnum = static_cast<IEnumConnections*>(enumerator.release());

            return S_OK;
        }
    };

    struct ConnectionPointContainer : ComObject, IConnectionPointContainer
    {
        std::vector<std::unique_ptr<IConnectionPoint, void(*)(IConnectionPoint*)>> points;
    
        HRESULT __stdcall QueryInterface(const GUID& riid, void** ppvObj) noexcept override
        {
            return ComQuery<IUnknown, IConnectionPointContainer>(*this, riid, ppvObj)
                .or_else([&]() { return ComQuery<IConnectionPointContainer>(*this, riid, ppvObj); })
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

        HRESULT EnumConnectionPoints(IEnumConnectionPoints **ppEnum) noexcept override
        {
            if (!ppEnum)
            {
                return E_POINTER;
            }

            auto enumerator = make_unique_range_enumerator<decltype(points)::value_type>(points.begin(), points.begin(), points.end());
            *ppEnum = static_cast<IEnumConnectionPoints*>(enumerator.release());

            return S_OK;
        }

        HRESULT FindConnectionPoint(REFIID riid, IConnectionPoint **ppCP) noexcept override
        {
            if (!ppCP)
            {
                return E_POINTER;
            }

            auto result = std::find_if(points.begin(), points.end(), [&](auto& item) {
                IID temp;

                return item->GetConnectionInterface(&temp) == S_OK && IsEqualGUID(riid, temp);
            });

            if (result == points.end())
            {
                return CONNECT_E_NOCONNECTION;
            }

            return S_OK;
        }
    };
}

#endif