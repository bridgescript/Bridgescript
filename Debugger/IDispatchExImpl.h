#pragma once 
//#ifndef INC_DISPEXIMPL 
//#define INC_DISPEXIMPL 

#ifndef HR 
#define HR(_ex) { HRESULT _hr = _ex; if( FAILED(_hr) ) return _hr; } 
#endif 

// TODO: Figure out how to make this work with plain IDispatchEx? 
// Otherwise, you have to define a new interface derived from IDispatchEx, 
// even if you don't want anything desides IDispatchEx... 
// TODO: Thread safety. 
class ATL_NO_VTABLE IDispatchExImpl : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchEx
{
public:

    IDispatchExImpl() { }

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(IDispatchExImpl)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IDispatchEx)
    END_COM_MAP()

    // IDispatch 
public:
    STDMETHODIMP GetTypeInfoCount(UINT* pctinfo) {
        if (!pctinfo) return E_POINTER;
        *pctinfo = 0;
        return S_OK;
    }

    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo) {
        ATLTRACENOTIMPL(__T("IDispatchEx::GetTypeInfo"));
    }

    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid) {
        // No support for named arguments 
        if (cNames != 1) {
            return DISP_E_NONAMEDARGS;
        }
        return this->GetDispID(CComBSTR(rgszNames[0]), fdexNameCaseInsensitive, rgdispid);
    }

    STDMETHODIMP Invoke(DISPID id, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdp, VARIANT* pvarRes, EXCEPINFO* pei, UINT* puArgErr) {
        // Fake the arg number 
        if (puArgErr) {
            *puArgErr = (UINT)-1;
        }
        // Forward to IDispatchEx 
        return this->InvokeEx(id, lcid, wFlags, pdp, pvarRes, pei, 0);
    }

    // IDispatchEx 
public:
    STDMETHODIMP GetDispID(BSTR bstrName, DWORD grfdex, DISPID* pid) {
        ATLTRACENOTIMPL(__T("IDispatchEx::GetDispID"));
    }

    STDMETHODIMP InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS* pdp, VARIANT* pvarRes, EXCEPINFO* pei, IServiceProvider* pspCaller) {
        ATLTRACENOTIMPL(__T("IDispatchEx::InvokeEx"));
    }

    STDMETHODIMP DeleteMemberByName(BSTR bstr, DWORD grfdex) {
        ATLTRACENOTIMPL(__T("IDispatchEx::DeleteMemberByName"));
    }

    STDMETHODIMP DeleteMemberByDispID(DISPID id) {
        ATLTRACENOTIMPL(__T("IDispatchEx::DeleteMemberByDispID"));
    }

    STDMETHODIMP GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD* pgrfdex) {
        ATLTRACENOTIMPL(__T("IDispatchEx::GetMemberProperties"));
    }

    STDMETHODIMP GetMemberName(DISPID id, BSTR* pbstrName) {
        ATLTRACENOTIMPL(__T("IDispatchEx::GetMemberName"));
    }

    STDMETHODIMP GetNextDispID(DWORD grfdex, DISPID id, DISPID* pid) {
        ATLTRACENOTIMPL(__T("IDispatchEx::GetNextDispID"));
    }

    STDMETHODIMP GetNameSpaceParent(IUnknown** ppunk) {
        if (!ppunk) {
            return E_POINTER;
        }
        *ppunk = 0;
        ATLTRACENOTIMPL(__T("IDispatchEx::GetNameSpaceParent"));
    }
};

//#endif  // INC_DISPEXIMPL 

//
//BOOL test(IDispatchEx *pdexScript)
//{
//    HRESULT hr;
//    VARIANT var;
//    DISPID dispid, putid;
//    BOOL retval = FALSE;
//    BSTR bstrName = NULL;
//    IDispatch   *pdispObj = NULL, *pdispCat = NULL;
//    IDispatchEx *pdexObj = NULL;
//    DISPPARAMS dispparams, dispparamsNoArgs = { NULL, NULL, 0, 0 };
//
//    // Get dispatch pointer for "cat"  
//    bstrName = SysAllocString(OLESTR("cat"));
//    if (bstrName == NULL) goto LDone;
//    hr = pdexScript->GetDispID(bstrName, 0, &dispid);
//    if (FAILED(hr)) goto LDone;
//    SysFreeString(bstrName);
//    bstrName = NULL;
//    hr = pdexScript->InvokeEx(dispid, LOCALE_USER_DEFAULT,
//        DISPATCH_PROPERTYGET, &dispparamsNoArgs,
//        &var, NULL, NULL);
//    if (FAILED(hr)) goto LDone;
//    pdispCat = var.pdispVal;
//
//    // Create object by calling "Object" constructor  
//    bstrName = SysAllocString(OLESTR("Object"));
//    if (NULL == bstrName) goto LDone;
//    hr = pdexScript->GetDispID(bstrName, 0, &dispid);
//    if (FAILED(hr)) goto LDone;
//    SysFreeString(bstrName);
//    bstrName = NULL;
//    hr = pdexScript->InvokeEx(dispid, LOCALE_USER_DEFAULT,
//        DISPATCH_CONSTRUCT, &dispparamsNoArgs,
//        &var, NULL, NULL);
//    if (FAILED(hr)) goto LDone;
//    pdispObj = var.pdispVal;
//    hr = pdispObj->QueryInterface(IID_IDispatchEx, (void **)&pdexObj);
//    if (FAILED(hr)) goto LDone;
//
//    // Create new element in object  
//    bstrName = SysAllocString(OLESTR("Elem"));
//    if (NULL == bstrName) goto LDone;
//    hr = pdexObj->GetDispID(bstrName, fdexNameEnsure, &dispid);
//    if (FAILED(hr)) goto LDone;
//    SysFreeString(bstrName);
//    bstrName = NULL;
//
//    // Assign "cat" dispatch pointer to element  
//    putid = DISPID_PROPERTYPUT;
//    var.vt = VT_DISPATCH;
//    var.pdispVal = pdispCat;
//    dispparams.rgvarg = &var;
//    dispparams.rgdispidNamedArgs = &putid;
//    dispparams.cArgs = 1;
//    dispparams.cNamedArgs = 1;
//    hr = pdexObj->InvokeEx(dispid, LOCALE_USER_DEFAULT,
//        DISPATCH_PROPERTYPUTREF, &dispparams,
//        NULL, NULL, NULL);
//    if (FAILED(hr)) goto LDone;
//
//    // Invoke method with "this" pointer  
//    putid = DISPID_THIS;
//    var.vt = VT_DISPATCH;
//    var.pdispVal = pdispObj;
//    dispparams.rgvarg = &var;
//    dispparams.rgdispidNamedArgs = &putid;
//    dispparams.cArgs = 1;
//    dispparams.cNamedArgs = 1;
//    hr = pdexObj->InvokeEx(dispid, LOCALE_USER_DEFAULT,
//        DISPATCH_METHOD, &dispparams,
//        NULL, NULL, NULL);
//    if (FAILED(hr)) goto LDone;
//
//    // Confirm that new element "Bar" is in object  
//    hr = pdexObj->GetNextDispID(fdexEnumAll, DISPID_STARTENUM, &dispid);
//    while (hr == NOERROR)
//    {
//        hr = pdexObj->GetMemberName(dispid, &bstrName);
//        if (FAILED(hr)) goto LDone;
//        retval = !wcscmp(bstrName, OLESTR("Bar"));
//        SysFreeString(bstrName);
//        bstrName = NULL;
//        if (retval) goto LDone;
//        hr = pdexObj->GetNextDispID(fdexEnumAll, dispid, &dispid);
//    }
//LDone:
//    SysFreeString(bstrName);
//    if (pdispCat != NULL)
//        pdispCat->Release();
//    if (pdispObj != NULL)
//        pdispObj->Release();
//    if (pdexObj != NULL)
//        pdexObj->Release();
//
//    return retval;
//}

//
//<html>
//<body>
//<script type = "text/javascript">
//function cat()
//{
//    // Create new element and assign the value 10  
//    this.Bar = 10;
//}
//
//function test()
//{
//    // Construct new object  
//    Obj = new Object();
//
//    // Create new element and assign function pointer  
//    Obj.Elem = cat;
//
//    // Call Elem method ("this" == Obj)  
//    Obj.Elem();
//
//    // Obj.Bar now exists  
//}
//test();
//< / script>
//< / body>
//< / html>