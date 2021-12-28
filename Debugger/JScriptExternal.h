// JScriptExternal.h : Declaration of the CJScriptExternal

#pragma once

#include "resource.h"       // main symbols 
#include "IDispatchExImpl.h" 

///////////////////////////////////////////////////////////////////////////// 
// CMyExpandoObject 
//

typedef enum {
    APP_INSERT_ROW = 0,
    APP_DELETE_ROW,
    APP_SCRIPT_EDITED,
    APP_ENABLE_EDITING,
    INVALID_DISP_ID,
} JSCRIPT_BRIDGE_ID;

class ATL_NO_VTABLE CJScriptExternal : public IDispatchExImpl
{
public:
    CJScriptExternal();
    static bool CreateFunctionBridge(IHTMLElemEventsNotify *pNotify, CComPtr<IDispatch> &spDispatch, LPWSTR fooName, IHTMLElemEventsNotify::BRIDGE_DISPID bridgeDispid, CComPtr<CJScriptExternal> &spfooImpl);

public:
    STDMETHODIMP InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS* pdp, VARIANT* pvarRes, EXCEPINFO* pei, IServiceProvider* pspCaller);
private:
    bool ConnectJScript(IHTMLElemEventsNotify *pNotify, CComPtr<IDispatch> &spDispatch, LPWSTR fooName, IHTMLElemEventsNotify::BRIDGE_DISPID bridgeDispid);

    IHTMLElemEventsNotify               *m_pNotify;
    IHTMLElemEventsNotify::BRIDGE_DISPID m_bridgeDispid;
    //_bstr_t                 m_bstrFooName;
};
