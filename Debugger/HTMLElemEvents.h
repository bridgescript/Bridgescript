//#ifndef HTMLELEMEVENTS_H
//#define HTMLELEMEVENTS_H
#pragma once

#include <atlbase.h>
#include <atlcom.h>
#include <exdisp.h>
#include <ExDispid.h>      // DISPIDs for WebBrowser Events
#include <MsHtmdid.h> // DISPIDs of html elements
#include <comdef.h>
#include <unordered_map>
#include "../Script/Basic.h"
class IHTMLElemEventsNotify
{
public:
    typedef ui32 BRIDGE_DISPID;
    virtual ~IHTMLElemEventsNotify() {};
    virtual void OnClick(CComPtr<IHTMLEventObj> spHTMLEvent) = 0;
    virtual void OnMouseOver(CComPtr<IHTMLEventObj> spHTMLEvent) = 0;
    virtual void OnMouseOut(CComPtr<IHTMLEventObj> spHTMLEvent) = 0;
    virtual void OnJScriptFunctionCall(BRIDGE_DISPID bridgeDispid, DISPPARAMS* pdp, VARIANT* pvarRes) = 0;
};

class CHTMLElemEventsBase 
{
protected:
    static _ATL_FUNC_INFO OnEventStruct;
};

class CHTMLElemEvents : public CHTMLElemEventsBase, public ATL::IDispEventSimpleImpl<1, CHTMLElemEvents, &DIID_HTMLElementEvents2>
{
    //LONG m_lRef;
    IHTMLElemEventsNotify* m_pHTMLElemEventsNotify;
    //typedef std::pair<DWORD, CComPtr<IHTMLElement> > ADVISE_INFO;
    //typedef std::vector<ADVISE_INFO> ADVISED_ITFS;
    typedef std::unordered_map<IUnknown*, DWORD>   EVENT_COOKIES;
    //ADVISED_ITFS m_dwEventCookies;
    EVENT_COOKIES m_dwEventCookies;

public:
    CHTMLElemEvents();
    ~CHTMLElemEvents();

    /*virtual ULONG STDMETHODCALLTYPE AddRef()
    {
        return ::InterlockedIncrement(&m_lRef);
    }
    virtual ULONG STDMETHODCALLTYPE Release()
    {
        ULONG ulRef;
        if((ulRef = ::InterlockedDecrement(&m_lRef)) == 0L)
        {
            delete this;
        }
        return ulRef;
    }*/
    HRESULT DispEventAdvise(CComPtr<IHTMLElement> &spHTMLElement) {
        DWORD dwEventCookie = 0;
        CComPtr<IUnknown> spUnk;
        
        spHTMLElement->QueryInterface(__uuidof(IUnknown), (void**)&spUnk);

        HRESULT hr = AtlAdvise(spUnk.p, (IUnknown*)this, DIID_HTMLElementEvents2, &dwEventCookie);
        if (S_OK == hr) {
            m_dwEventCookies.insert(EVENT_COOKIES::value_type(spUnk.p, dwEventCookie));
        }
        return hr;
    }

    void DispEventUnadviseAll() {
        HRESULT hr = E_UNEXPECTED;
        EVENT_COOKIES::iterator it = m_dwEventCookies.begin();
        for (; it != m_dwEventCookies.end(); ++it) {
            hr = AtlUnadvise(it->first, DIID_HTMLElementEvents2, it->second);
        }
        m_dwEventCookies.clear();
    }
    
    HRESULT DispEventUnadvise(CComPtr<IHTMLElement> &spHTMLElement) {
        HRESULT hr = E_UNEXPECTED;
        CComPtr<IUnknown> spUnk;

        spHTMLElement->QueryInterface(__uuidof(IUnknown), (void**)&spUnk);

        EVENT_COOKIES::iterator it = m_dwEventCookies.find(spUnk.p);
        if (it != m_dwEventCookies.end()) {
            hr = AtlUnadvise(it->first, DIID_HTMLElementEvents2, it->second);
            if (S_OK == hr) {
                m_dwEventCookies.erase(it);
            }
        }
        return hr;
    }

    void SetNotify(IHTMLElemEventsNotify* pHTMLElemEventsNotify)
    {
        m_pHTMLElemEventsNotify = pHTMLElemEventsNotify;
    }
    BEGIN_SINK_MAP(CHTMLElemEvents)
        SINK_ENTRY_INFO(1, DIID_HTMLElementEvents2, (DISPID)DISPID_HTMLELEMENTEVENTS2_ONCLICK, __OnClick, &OnEventStruct)
        SINK_ENTRY_INFO(1, DIID_HTMLElementEvents2, (DISPID)DISPID_HTMLELEMENTEVENTS2_ONMOUSEOVER, __OnMouseOver, &OnEventStruct)
        SINK_ENTRY_INFO(1, DIID_HTMLElementEvents2, (DISPID)DISPID_HTMLELEMENTEVENTS2_ONMOUSEOUT, __OnMouseOut, &OnEventStruct)
    END_SINK_MAP()

    void __stdcall __OnClick(/*IHTMLEventObj*/IDispatch* pEventObj)
    {
        CHTMLElemEvents* pThis = static_cast<CHTMLElemEvents*>(this);
        CComQIPtr<IHTMLEventObj> spHTMLEvent(pEventObj);
        pThis->m_pHTMLElemEventsNotify->OnClick(spHTMLEvent);
    }
    void __stdcall __OnMouseOver(/*IHTMLEventObj*/IDispatch* pEventObj)
    {
        CHTMLElemEvents* pThis = static_cast<CHTMLElemEvents*>(this);
        CComQIPtr<IHTMLEventObj> spHTMLEvent(pEventObj);
        pThis->m_pHTMLElemEventsNotify->OnMouseOver(spHTMLEvent);
    }
    void __stdcall __OnMouseOut(/*IHTMLEventObj*/IDispatch* pEventObj)
    {
        CHTMLElemEvents* pThis = static_cast<CHTMLElemEvents*>(this);
        CComQIPtr<IHTMLEventObj> spHTMLEvent(pEventObj);
        pThis->m_pHTMLElemEventsNotify->OnMouseOut(spHTMLEvent);
    }

private:

};
__declspec(selectany) _ATL_FUNC_INFO CHTMLElemEventsBase::OnEventStruct 
    = {CC_STDCALL, VT_EMPTY, 1, { VT_DISPATCH } };


//#endif // HTMLELEMEVENTS_H