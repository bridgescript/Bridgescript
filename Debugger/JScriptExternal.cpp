// JScriptExternal.cpp : Implementation of CJScriptExternal

#include "stdafx.h"
#include "HTMLElemEvents.h"
#include "JScriptExternal.h"

//BEGIN_OBJECT_MAP(ObjectMap)
//    OBJECT_ENTRY(CLSID_JScriptExternal, CJScriptExternal)
//END_OBJECT_MAP()
// CJScriptExternal

CJScriptExternal::CJScriptExternal() : m_pNotify(0), m_bridgeDispid(JSCRIPT_BRIDGE_ID::INVALID_DISP_ID)
{
    // Turn Expando case sensitive in case we have variables like root and Root   
    // and we've got a case-sensitive language like Java.   
   // m_fdexDefault = fdexNameCaseSensitive;
}

        //hr = spDispatchEx->GetNextDispID(fdexEnumAll, DISPID_STARTENUM, &dispid);
        //while (hr == NOERROR)
        //{
        //    BSTR ws;
        //    hr = spDispatchEx->GetMemberName(dispid, &ws);
        //    if (FAILED(hr))
        //    {
        //        break;
        //    }
        //    //retval = !wcscmp(bstrName, OLESTR("Bar"));
        //    SysFreeString(ws);
        //    hr = spDispatchEx->GetNextDispID(fdexEnumAll, dispid, &dispid);
        //}
bool CJScriptExternal::CreateFunctionBridge(IHTMLElemEventsNotify *pNotify, CComPtr<IDispatch> &spDispatch, LPWSTR fooName, IHTMLElemEventsNotify::BRIDGE_DISPID bridgeDispid, CComPtr<CJScriptExternal> &spfooImpl) {
#ifdef _DEBUG
    assert(pNotify);
    assert(fooName);
    assert(JSCRIPT_BRIDGE_ID::INVALID_DISP_ID > bridgeDispid);
#endif // _DEBUG

    if (!spDispatch || !*fooName) {
        return false;
    }

    CComObject<CJScriptExternal> *pObject = 0;
    HRESULT hr = CComObject<CJScriptExternal>::CreateInstance(&pObject);
    
    if (S_OK == hr) {
        spfooImpl = pObject;
        return spfooImpl->ConnectJScript(pNotify, spDispatch, fooName, bridgeDispid);
    }
    return false;
}

bool CJScriptExternal::ConnectJScript(IHTMLElemEventsNotify *pNotify, CComPtr<IDispatch> &spDispatch, LPWSTR fooName, IHTMLElemEventsNotify::BRIDGE_DISPID bridgeDispid) {
#ifdef _DEBUG
    assert(pNotify);
    assert(fooName && *fooName);
    assert(JSCRIPT_BRIDGE_ID::INVALID_DISP_ID > bridgeDispid);
#endif // _DEBUG

    if (!spDispatch) {
        return false;
    }

    CComQIPtr<IDispatchEx> spDispatchEx(spDispatch);
    DISPID putid = DISPID_PROPERTYPUT, dispid;
    DISPPARAMS dispparams;
    _variant_t var;

    m_pNotify      = pNotify;
    m_bridgeDispid = bridgeDispid;
    //m_bstrFooName   = fooName;

    HRESULT hr = spDispatchEx->GetDispID(fooName, 0, &dispid);

    var = this;
    dispparams.rgvarg = &var;
    dispparams.rgdispidNamedArgs = &putid;
    dispparams.cArgs = 1;
    dispparams.cNamedArgs = 1;

    hr = spDispatchEx->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUTREF, &dispparams, NULL, NULL, NULL);

    return (S_OK == hr);
}

STDMETHODIMP CJScriptExternal::InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS* pdp, VARIANT* pvarRes, EXCEPINFO* pei, IServiceProvider* pspCaller)
{
    if (pdp) {
        if (m_pNotify) {
            m_pNotify->OnJScriptFunctionCall(m_bridgeDispid, pdp, pvarRes);
            return S_OK;
        }
        return E_FAIL;
    }
    return E_POINTER;
}

/////////////////////////////////////////////////////////////////////////////   
