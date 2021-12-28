#pragma once
#include "HTMLElemEvents.h"
#include "../Script/Basic.h"
#include "../Script/Debugger.h"
#include "../Script/Script.h"
//#include "smart_ptr.h"
#include "SafeThread.h"
#include "MainFrm.h"
#include "JScriptExternal.h"

using namespace script;
using namespace mb_thread_lib;

#ifdef _WIN32_WCE
#error "CHtmlView is not supported for Windows CE."
#endif 

#ifdef RICH_EDIT
#define CHtmlView   CRichEditView
//class CRichEditDocTestCntrItem;
#endif // RICH_EDIT
typedef enum Highlight_Type {
    HIGHLIGHT_ON_BREAK = 0,
    HIGHLIGHT_COMPILE_ERROR,
    HIGHLIGHT_RUNTIME_ERROR,
    HIGHLIGHT_STACK_CALL,
    HIGHLIGHT_BREAKPOINT,
    HIGHLIGHT_FUNC_DEF,
    HIGHLIGHT_OFF,
} HIGHLIGHT_TYPE;
// CScriptView html view

class CScriptView : public CHtmlView
{
    //class _CControlSite : public CBrowserControlSite, public IDispatch
    //{
    //public:
    //    _CControlSite(COleControlContainer* pCtrlCont, CHtmlView *pHandler)
    //        :CBrowserControlSite(pCtrlCont, pHandler)
    //    {
    //    };
    //    ~_CControlSite() {
    //    }
    //    LPUNKNOWN GetInterfaceHook(const void *iid)
    //    {
    //        if (IsEqualIID((REFIID)(*(IID*)iid), IID_IDispatch))
    //            return (IDispatch*) this;
    //        return CBrowserControlSite::GetInterfaceHook(iid);
    //    }
    //    //IUnknown
    //    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject)
    //    {
    //        return CBrowserControlSite::ExternalQueryInterface(&riid, ppvObject);
    //    }
    //    STDMETHOD_(ULONG, AddRef)()
    //    {
    //        return CBrowserControlSite::AddRef();//ExternalAddRef();
    //    }

    //    STDMETHOD_(ULONG, Release)()
    //    {
    //        return CBrowserControlSite::Release();//ExternalRelease();
    //    }
    //    //IDispatch
    //    STDMETHOD(GetTypeInfoCount)(
    //        /* [out] */ UINT *pctinfo)
    //    {
    //        return E_NOTIMPL;
    //    };

    //    STDMETHOD(GetTypeInfo)(
    //        /* [in] */ UINT iTInfo,
    //        /* [in] */ LCID lcid,
    //        /* [out] */ ITypeInfo **ppTInfo)
    //    {
    //        return E_NOTIMPL;
    //    };

    //    STDMETHOD(GetIDsOfNames)(
    //        /* [in] */ REFIID riid,
    //        /* [size_is][in] */ LPOLESTR *rgszNames,
    //        /* [in] */ UINT cNames,
    //        /* [in] */ LCID lcid,
    //        /* [size_is][out] */ DISPID *rgDispId)
    //    {
    //        return E_NOTIMPL;
    //    };

    //    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
    //        DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
    //    {
    //        switch (dispIdMember)
    //        {
    //            case DISPID_AMBIENT_DLCONTROL:
    //                pVarResult->vt = VT_I4;
    //                pVarResult->lVal = DLCTL_DLIMAGES;
    //                return S_OK;
    //            default:
    //                break;
    //        }
    //        return E_NOTIMPL;
    //    };

    //};


	DECLARE_DYNCREATE(CScriptView)

protected:
	CScriptView();           // protected constructor used by dynamic creation
	virtual ~CScriptView();
#ifdef RICH_EDIT
    virtual void OnInitialUpdate(); // called first time after construct
    void ParseText();
#endif // RICH_EDIT

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
#ifdef RICH_EDIT
    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnEnVscroll();
#endif // RICH_EDIT

    //virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
    void SetScriptFileName(LPCTSTR filePath) { m_ScriptFileName = filePath; };
    //BOOL ReadScriptAsHTML();
    void HighlightLine(HIGHLIGHT_TYPE type, ui32 fileId, ui32 line, ThreadId threadId, Scope *pScope);
    void ClearHighlight(bool invalidateBreakpoints);
    void SetBreakPointMarker(ui32 line, BREAKPOINT_MARKER marker);
    void SetAllBreakpointsMarker(bool invalidate);
#ifdef RICH_EDIT
    void LoadBreakpointMarkers();
#endif // RICH_EDIT

    void EnableEditing(bool enable);
    //virtual BOOL CreateControlSite(COleControlContainer* pContainer, COleControlSite** ppSite, UINT nID, REFCLSID clsid);
#ifndef RICH_EDIT
    virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext);
#endif // !RICH_EDIT

    //virtual HRESULT OnGetHostInfo(DOCHOSTUIINFO *pInfo);
    //virtual HRESULT OnShowContextMenu(DWORD dwID, LPPOINT ppt, LPUNKNOWN pcmdtReserved, LPDISPATCH pdispReserved);
    //virtual HRESULT OnTranslateAccelerator(LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID);
    //STDMETHOD(ShowContextMenu)(DWORD dwID, POINT * ppt, IUnknown * pcmdtReserved, IDispatch *pdispReserved);
    //STDMETHOD(TranslateAccelerator)(LPMSG msg, const GUID __RPC_FAR * guid, DWORD dw);
    //STDMETHOD(GetHostInfo)(DOCHOSTUIINFO __RPC_FAR *pInfo);
    //STDMETHOD(ShowMessage)(HWND, LPOLESTR ws1, LPOLESTR ws2, DWORD, LPOLESTR ws3, DWORD, LRESULT *);
    //bool Save(CArchive& ar);
    bool IsModified();
protected:
    //BOOL PushScriptLine(CString& scriptLine);
    //BOOL CreateRow(CString &scriptLine);
    //void SelectLine();
    //bool InsertRow(LPCWSTR wsText, ui32 rowPos);
    //bool DeleteRow(i32 rowPos);
    //void RefreshBreakpointMarkers(CComPtr<IHTMLElementCollection> &spRows, ui32 startRow, bool newRow);
    //bool SetHtmlVarValue(_bstr_t varName, _variant_t &varValue);
    //bool GetHtmlVarValue(_bstr_t varName, _variant_t &varValue);
public:
    virtual void OnDocumentComplete(LPCTSTR lpszURL);
    //void OnClick(CComPtr<IHTMLEventObj> spHTMLEvent);
    //void OnMouseOver(CComPtr<IHTMLEventObj> spHTMLEvent);
    //void OnMouseOut(CComPtr<IHTMLEventObj> spHTMLEvent);
    //void OnJScriptFunctionCall(IHTMLElemEventsNotify::BRIDGE_DISPID bridgeDispid, DISPPARAMS* pdp, VARIANT* pvarRes);
    void GetPathAndName(_bstr_t &fileName, _bstr_t &path);
private:
    //class HTMLElemEventsNotify: public IHTMLElemEventsNotify {
    //public:
    //    HTMLElemEventsNotify(CScriptView *pScriptView) : m_pScriptView(pScriptView) {};
    //    virtual ~HTMLElemEventsNotify() {};
    //    virtual void OnClick(CComPtr<IHTMLEventObj> spHTMLEvent) { m_pScriptView->OnClick(spHTMLEvent); };
    //    virtual void OnMouseOver(CComPtr<IHTMLEventObj> spHTMLEvent) { m_pScriptView->OnMouseOver(spHTMLEvent); };
    //    virtual void OnMouseOut(CComPtr<IHTMLEventObj> spHTMLEvent) { m_pScriptView->OnMouseOut(spHTMLEvent); };
    //    virtual void OnJScriptFunctionCall(BRIDGE_DISPID bridgeDispid, DISPPARAMS* pdp, VARIANT* pvarRes) { m_pScriptView->OnJScriptFunctionCall(bridgeDispid, pdp, pvarRes); };

    //private:
    //    CScriptView *m_pScriptView;
    //} m_HTMLElemEventsNotify;
#ifdef RICH_EDIT

    struct KeyWordData {
        COLORREF color;
        KeyWordData(COLORREF c) : color(c) {}
    };
    KeyWordData* IsKeyWord(CString& s, long pos, long &begin, long &end);
    void MarkKeyWord(KeyWordData* pkw, long start, long end, long origend);
    CMapStringToPtr m_keywords;

#endif // RICH_EDIT

    //unordered_set<ui32>         m_InitialBreakpoints;
    //CComPtr<CJScriptExternal>   m_spInsertRowImpl,
    //                            m_spDeleteRowImpl/*,
    //                            m_spScriptEditedImpl,
    //                            m_spEnableEditingImpl*/;
    //CHTMLElemEvents             m_htmlElemEvents;
    CString                    // m_TableRowTemplate,
                                m_ScriptFileName;
    //CComQIPtr<IHTMLDocument2>   m_spHtmlDocument2;
    //CComPtr<IHTMLElement>       m_spEventHolder;
    //CComQIPtr<IHTMLTable>       m_spTable;
    //const _bstr_t               m_td,
    //                            m_p,
    //                            m_code,
    //                            m_codecell,
    //                            m_breakpoint;
    int                         //m_rowCount,
                                m_currentLine;
    BREAK_HIT_DATA              m_breakHitData;
    HIGHLIGHT_TYPE              m_highlightType;
    //DEBUGGER_STATES             m_debuggerState;
    //bool                        m_modified;
    
public:
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
    afx_msg void OnDestroy();
};

#ifdef CHtmlView
#undef CHtmlView
#endif // CDocument
