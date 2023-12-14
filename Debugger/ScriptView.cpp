// ScriptView.cpp : implementation file
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "ChildFrm.h"
#include "MainFrm.h"
#include "ScriptView.h"
#include "SafeThread.h"
#include "../Script/Compiler.h"

using namespace script;

#ifdef RICH_EDIT
#define CHtmlView   CRichEditView
#endif // RICH_EDIT


// CScriptView

IMPLEMENT_DYNCREATE(CScriptView, CHtmlView)

CScriptView::CScriptView() :
    m_currentLine(0)//,
    ////m_HTMLElemEventsNotify(this),
    //m_DebuggerEventsImpl(this),
    //m_tblContext("tblContext"),
    ////m_td("td"),
    ////m_p("p"),
    ////m_code("code"),
    ////m_codecell("codecell"),
    ///m_breakpoint("breakpoint"),
    ////m_debuggerState(DEBUGGER_STOP)/*,
    //m_highlightLine(0)//,
    ////m_threaId(0)*/ 
{
    ////m_htmlElemEvents.SetNotify(&m_HTMLElemEventsNotify);
    //CMainFrame *pMainFrame = (CMainFrame*)theApp.GetMainWnd();
    
    //m_ThreadScriptRunner.SetDebugerNotify(pMainFrame->GetDebuggerEvents());
    //Debugger::GetInstance().SetDebuggerNotify(&m_DebuggerEventsImpl);
}

CScriptView::~CScriptView()
{
    //Debugger::GetInstance().SetDebuggerNotify(0);
    //Debugger::GetInstance().Stop();
    //m_ThreadScriptRunner.Stop();
    //m_ThreadScriptRunner.SetDebugerNotify(0);
}

void CScriptView::DoDataExchange(CDataExchange* pDX)
{
	CHtmlView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CScriptView, CHtmlView)
    ON_WM_CREATE()
    /*ON_COMMAND(ID_DEBUG_RUN, &CScriptView::OnCmdDebugRun)
    ON_COMMAND(ID_DEBUG_STOP, &CScriptView::OnCmdDebugStop)
    ON_COMMAND(ID_DEBUG_PAUSE, &CScriptView::OnCmdDebugPause)
    ON_COMMAND(ID_DEBUG_STEPIN, &CScriptView::OnCmdDebugStepin)
    ON_COMMAND(ID_DEBUG_STEPOVER, &CScriptView::OnCmdDebugStepover)
    ON_COMMAND(ID_DEBUG_STEPOUT, &CScriptView::OnCmdDebugStepout)*/
    ON_WM_DESTROY()
#ifdef RICH_EDIT
    ON_WM_KEYUP()
    ON_WM_VSCROLL()
    ON_CONTROL_REFLECT(EN_VSCROLL, &CScriptView::OnEnVscroll)
#endif // RICH_EDIT

END_MESSAGE_MAP()


// CScriptView diagnostics

#ifdef _DEBUG
void CScriptView::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CScriptView::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}
#endif //_DEBUG


// CScriptView message handlers


int CScriptView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CHtmlView::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  Add your specialized creation code here
    return 0;
}

//BOOL CScriptView::CreateControlSite(COleControlContainer* pContainer, COleControlSite** ppSite, UINT nID, REFCLSID clsid) {
////    if (ppSite == NULL) {
////        ASSERT(FALSE);
////        return FALSE;
////    }
////
////    _CControlSite *pBrowserSite = new _CControlSite(pContainer, this);
////    if (!pBrowserSite)
////        return FALSE;
////
////    *ppSite = pBrowserSite;
////    return TRUE;
//    if (CHtmlView::CreateControlSite(pContainer, ppSite, nID, clsid)) {
//        //CComPtr<IOleObject> spObj;
//        //HRESULT hr = m_pBrowserApp->QueryInterface(IID_IOleObject, (void**)&spObj);
//        //if ((S_OK == hr) && (spObj))
//        //{
//        //    RECT r;
//        //    //activate ActiveX control for tab and other keys to work
//        //    hr = spObj->DoVerb(OLEIVERB_UIACTIVATE, NULL, NULL, 0, NULL, &r);
//        //    //hr = spOleObj->DoVerb(OLEIVERB_INPLACEACTIVATE,NULL,NULL,0,NULL,&r);
//        //    ::CComQIPtr<IOleInPlaceActiveObject> sp(spObj);
//        //    LONG data = SetWindowLongPtr(m_hWnd, GWL_USERDATA, (LONG)sp.Detach());
//
//        //    //DISPID_AMBIENT_DLCONTROL:
//
//        //    //pVarResult->vt = VT_I4;
//        //    //pVarResult->lVal = DLCTL_DLIMAGES;
//        //    //DISPID_IDISPATCHEX_GETMEMBERNAME
//        //    //PutProperty(LPCTSTR lpszProperty, const VARIANT& vtValue)
//        //    SetProperty(DISPID_AMBIENT_DLCONTROL, VT_I4, DLCTL_DLIMAGES);
//        //    //InvokeHelper(DISPID_AMBIENT_DLCONTROL, DISPATCH_PROPERTYPUT, VT_I4, )
//        //    //Invoke()
//        //}
//    }
//    return TRUE;
//};
//
//HRESULT CScriptView::OnShowContextMenu(DWORD dwID, LPPOINT ppt, LPUNKNOWN pcmdtReserved, LPDISPATCH pdispReserved) {
//    if (dwID == 0x2)
//        return E_NOTIMPL;
//    //else hide menu
//    return S_OK;
//}
//
//HRESULT CScriptView::OnTranslateAccelerator(LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID) {
//    if (lpMsg && lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_ESCAPE)
//    {
//        return S_OK;
//    }
//
//    return E_NOTIMPL;
//    //   if(msg && msg->message == WM_KEYDOWN && msg->wParam == VK_RETURN) 
//    //   {
//    //}
//}
//
//HRESULT CScriptView::OnGetHostInfo(DOCHOSTUIINFO *pInfo) {
//    pInfo->cbSize = sizeof(DOCHOSTUIINFO);
//    pInfo->dwFlags = DOCHOSTUIFLAG_DIALOG |
//        DOCHOSTUIFLAG_DISABLE_HELP_MENU |
//        DOCHOSTUIFLAG_NO3DBORDER |
//        DOCHOSTUIFLAG_FLAT_SCROLLBAR/* |
//        DOCHOSTUIFLAG_SCROLL_NO*/;
//    pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;
//
//    return S_OK;
//
//}

//HRESULT CScriptView::ShowMessage(HWND, LPOLESTR ws1, LPOLESTR ws2, DWORD, LPOLESTR ws3, DWORD, LRESULT *)
//{
//    // Don't show message boxes, produced
//    // by scripts on the page
//#ifdef _DEBUG
//    return E_NOTIMPL;//show if debug
//#else
//    return S_OK;
//#endif
//}
#ifndef RICH_EDIT



BOOL CScriptView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) {
    // TODO: Add your specialized code here and/or call the base class
    //HINSTANCE hInstance = theApp.m_hInstance;
    //HRSRC res = ::FindResource(hInstance, MAKEINTRESOURCE(IDR_HTML2), RT_HTML);
    ////DWORD dwSize = ::SizeofResource(hInstance, res);
    //m_TableRowTemplate = (char*)::LoadResource(hInstance, res);

    if (CHtmlView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext)) {
        LoadFromResource(IDR_HTML1);
        CComPtr<IOleObject> spObj;
        HRESULT hr = m_pBrowserApp->QueryInterface(IID_IOleObject, (void**)&spObj);
        if ((S_OK == hr) && (spObj))
        {
            RECT r;
            //activate ActiveX control for tab and other keys to work
            hr = spObj->DoVerb(OLEIVERB_UIACTIVATE, NULL, NULL, 0, NULL, &r);
            //hr = spOleObj->DoVerb(OLEIVERB_INPLACEACTIVATE,NULL,NULL,0,NULL,&r);
            ::CComQIPtr<IOleInPlaceActiveObject> sp(spObj);
            LONG data = SetWindowLongPtr(m_hWnd, GWL_USERDATA, (LONG)sp.Detach());

            //DISPID_AMBIENT_DLCONTROL:

            //pVarResult->vt = VT_I4;
            //pVarResult->lVal = DLCTL_DLIMAGES;
            //DISPID_IDISPATCHEX_GETMEMBERNAME
            //PutProperty(LPCTSTR lpszProperty, const VARIANT& vtValue)
            
            //SetProperty(DISPID_AMBIENT_DLCONTROL, VT_I4, DLCTL_DLIMAGES);
            
            //InvokeHelper(DISPID_AMBIENT_DLCONTROL, DISPATCH_PROPERTYPUT, VT_I4, )
            //Invoke()
        }
        return TRUE;
    }
    return FALSE;
}
#endif // !RICH_EDIT

//BOOL CScriptView::ReadScriptAsHTML() {
//    CString line;
//    CStdioFile file(m_ScriptFileName, CFile::modeRead | CFile::typeText);
//    
//    //m_pThreadScriptRunner = new THREAD_SCRIPT_RUNNER();
//    
//    //m_pRunner->Load();
//    //m_pRunner->Exec();
//    //m_rowCount = 0;
//    while (file.ReadString(line) && PushScriptLine(line)) {};
//    return true;
//}

void CScriptView::HighlightLine(HIGHLIGHT_TYPE type, ui32 fileId, ui32 line, ThreadId threadId, Scope *pScope) {
#ifdef RICH_EDIT
    m_currentLine = line;
    CChildFrame *pFrame =(CChildFrame*) GetParentFrame();
    CRichEditCtrl &ctl = GetRichEditCtrl();
   
    int scrollPos = GetScrollPos(SB_VERT);
    int mi = 0, ma = 0;
    GetScrollRange(SB_VERT, &mi, &ma);
    int vl = scrollPos / pFrame->GetImgSizeY() + ((scrollPos % pFrame->GetImgSizeY() != 0) ? 1 : 0);
    RECT r;
    ctl.GetClientRect(&r);
    //ctl.GetWindowRect(&wr);
    //ctl.GetRect(&rr);
    //ui32 vl = ctl.GetFirstVisibleLine();
    if ((line < vl) || (line >= (r.bottom / pFrame->GetImgSizeY() + vl))) { // scroll

    //if (curPos > line) {
      //  line = -(curPos - line);
    //}
    //else {
        i32 newLine = line - vl;
        //} // cur:3 line:10 -> 10-3 = 7 , -(3 - 10)
        int totalLines = ctl.GetLineCount();
        int maxscrollLine = totalLines - (r.bottom / pFrame->GetImgSizeY()) - 1;
        if (maxscrollLine < line) {
            newLine -= line - maxscrollLine;
        }
        else {
        }
        ctl.LineScroll(newLine, 0);
        //ScrollWindow(0, newLine * 15 + scrollPos % 15);
        //UpdateWindow();
        pFrame->SetHighlight(line, type);
        int newscrollPos = GetScrollPos(SB_VERT);
        pFrame->SetPos(newscrollPos);
    }
    else {
        //this->SelectLine()
        //SetScrollPos(SB_VERT, line * 15);
        //RECT r;
        //GetClientRect(&r);
        //InvalidateRect(&r);
        pFrame->SetHighlight(line, type);
    }

    pFrame->Invalidate();
   
    if (HIGHLIGHT_ON_BREAK == type) {
        ((CMainFrame*)theApp.m_pMainWnd)->m_wndProperties.UpdateStackList(threadId);
    }

    return;
#endif // RICH_EDIT
    //if (!m_spTable) {
    //    //m_breakHitData.insert(pair<ThreadId, BREAK_HIT_DATA>(threadId, BREAK_HIT_DATA(fileId, line, threadId, pScope)));
    //    m_breakHitData = BREAK_HIT_DATA(fileId, line, threadId, pScope);
    //    //m_highlightLine = line;
    //    m_highlightType = type;
    //    //m_threaId = threadId;
    //    return;
    //}

    //CComPtr<IHTMLElementCollection> spRows;
    //m_spTable->get_rows(&spRows);
    //if (!spRows) {
    //    return;
    //}
    //i32 len = 0;
    //spRows->get_length(&len);
    //if ((ui32)--len <= line) { // discard empty row
    //    return;
    //}
    //CComPtr<IDispatch> spDispatch;
    //spRows->item(_variant_t(m_currentLine), _variant_t(m_currentLine), &spDispatch);
    //CComQIPtr<IHTMLTableRow> spRow;
    //spRow = spDispatch;
    //if (!spRow) {
    //    return;
    //}
    //spRow->put_bgColor(_variant_t(""));
    //spDispatch.Release();
    //spRows->item(_variant_t(line), _variant_t(line), &spDispatch);
    //spRow = spDispatch;
    //if (!spRow) {
    //    return;
    //}
    //_variant_t color;
    //switch (type) {
    //    case HIGHLIGHT_STACK_CALL:
    //    case HIGHLIGHT_ON_BREAK:
    //        color = _bstr_t("rgb(187, 222, 251)");
    //        break;
    //    case HIGHLIGHT_COMPILE_ERROR:
    //        color = _bstr_t("rgb(255, 138, 128)");
    //        break;
    //    case HIGHLIGHT_RUNTIME_ERROR:
    //        color = _bstr_t("rgb(255, 224, 130)");
    //        break;
    //    case HIGHLIGHT_BREAKPOINT:
    //        color = _bstr_t("rgb(200, 252, 200)");
    //        break;
    //    case HIGHLIGHT_FUNC_DEF:
    //        color = _bstr_t("rgb(255, 204, 255)");
    //        break;
    //    default:
    //        assert(0);
    //        break;
    //}
    //spRow->put_bgColor(color);// _variant_t("rgb(250, 210, 180)"));
    //CComQIPtr<IHTMLElement> spElement(spRow);
    //CComQIPtr<IHTMLElement2> spElement2(spElement);
    //CComPtr<IHTMLRect> spRect;// , spRectDoc;
    //spElement2->getBoundingClientRect(&spRect);
    ////CComQIPtr<IHTMLDocument3> spDoc3(m_spHtmlDocument2);
    //CComPtr<IHTMLWindow2> spWindow2;
    //m_spHtmlDocument2->get_parentWindow(&spWindow2);
    //CComQIPtr<IHTMLWindow7> spWindow7(spWindow2);
    ////CComPtr<IHTMLElement> spDocElement;
    ////spDoc3->get_documentElement(&spDocElement);
    ////CComQIPtr<IHTMLElement2> spDocElement2(spDocElement);
    ////spDocElement2->getBoundingClientRect(&spRectDoc);
    //if (spRect && /*spDocElement2 && */spWindow7) {
    //    long top = 0, bottom = 0, /*docTop = 0, docBottom = 0,*/ height = 0;
    //    spWindow7->get_innerHeight(&height);
    //    spRect->get_top(&top);
    //    spRect->get_bottom(&bottom);
    //    //spRectDoc->get_top(&docTop);
    //    //spRectDoc->get_bottom(&docBottom);

    //    if ((top >= 0) && (bottom <= height)) {
    //    //if ((top >= docTop) && (bottom <= docBottom)) {
    //        m_currentLine = line;
    //        if (HIGHLIGHT_ON_BREAK == type) {
    //            ((CMainFrame*)theApp.m_pMainWnd)->m_wndProperties.UpdateStackList(threadId);
    //        }
    //        return; // noscroll
    //    }
    //}
    //spElement->scrollIntoView(_variant_t(true));
    //m_currentLine = line;
    //if (HIGHLIGHT_ON_BREAK == type) {
    //    ((CMainFrame*)theApp.m_pMainWnd)->m_wndProperties.UpdateStackList(threadId);
    //}
}

void CScriptView::ClearHighlight(bool invalidateBreakpoints) {
#ifdef RICH_EDIT
    CChildFrame *pFrame = (CChildFrame*)GetParentFrame();
    pFrame->SetHighlight(0, HIGHLIGHT_OFF);
    pFrame->SetPos(GetScrollPos(SB_VERT));
    pFrame->Invalidate();
#endif // RICH_EDIT

    SetAllBreakpointsMarker(invalidateBreakpoints);
#ifdef RICH_EDIT
    return;
#endif // RICH_EDIT
/*

    if (!m_spTable) {
        return;
    }
    CComPtr<IHTMLElementCollection> spRows;
    m_spTable->get_rows(&spRows);
    if (!spRows) {
        return;
    }
    CComPtr<IDispatch> spDispatch;
    spRows->item(_variant_t(m_currentLine), _variant_t(m_currentLine), &spDispatch);
    CComQIPtr<IHTMLTableRow> spRow;
    spRow = spDispatch;
    if (!spRow) {
        return;
    }
    spRow->put_bgColor(_variant_t(""));*/
}
//
//BOOL CScriptView::PushScriptLine(CString& scriptLine) {
//    HRESULT hr;
//    if (!m_spHtmlDocument2) {
//#ifndef RICH_EDIT
//        m_spHtmlDocument2 = GetHtmlDocument();
//#endif // !RICH_EDIT
//        CComQIPtr<IHTMLElementCollection> spScripts;
//        CComPtr<IDispatch> spDispatch;
//        hr = m_spHtmlDocument2->get_Script(&spDispatch);
//        if (S_OK == hr) {
//            CJScriptExternal::CreateFunctionBridge(&m_HTMLElemEventsNotify, spDispatch, L"App_InsertRow", JSCRIPT_BRIDGE_ID::APP_INSERT_ROW, m_spInsertRowImpl);
//            CJScriptExternal::CreateFunctionBridge(&m_HTMLElemEventsNotify, spDispatch, L"App_DeleteRow", JSCRIPT_BRIDGE_ID::APP_DELETE_ROW, m_spDeleteRowImpl);
//            //CJScriptExternal::CreateBridge(&m_HTMLElemEventsNotify, spDispatch, L"App_ScriptEdited", JSCRIPT_BRIDGE_ID::APP_SCRIPT_EDITED, m_spScriptEditedImpl);
//            //CJScriptExternal::CreateBridge(&m_HTMLElemEventsNotify, spDispatch, L"App_EnableEditing", JSCRIPT_BRIDGE_ID::APP_ENABLE_EDITING, m_spEnableEditingImpl);
//        }
//    }
//    if (m_spHtmlDocument2) {
//        if (!m_spTable) {
//            CComQIPtr<IHTMLDocument3> spHtmlDocument3(m_spHtmlDocument2);
//            CComPtr<IHTMLElement> spElement;
//            _bstr_t m_tblContext("tblContext");;
//            hr = spHtmlDocument3->getElementById(m_tblContext, &spElement);
//            if (FAILED(hr)) {
//                return false;
//            }
//            m_spTable = spElement;
//        }
//        if (!m_spEventHolder) {
//            CComQIPtr<IHTMLDocument3> spHtmlDocument3(m_spHtmlDocument2);
//            _bstr_t eventHolder("eventHolder");
//            hr = spHtmlDocument3->getElementById(eventHolder, &m_spEventHolder);
//            if (FAILED(hr)) {
//                return false;
//            }
//        }
//        return CreateRow(scriptLine);
//    }
//    return false;
//}

//bool CScriptView::InsertRow(LPCWSTR wsText, ui32 rowPos) {
//    if (!m_spTable) {
//        return false;
//    }
//    HRESULT hr;
//
//    CComPtr<IHTMLElementCollection> spRows;
//    hr = m_spTable->get_rows(&spRows);
//    if (FAILED(hr)) {
//        return false;
//    }
//    i32 rowCount;
//    hr = spRows->get_length(&rowCount);
//    if (FAILED(hr)) {
//        return false;
//    }
//    --rowCount; // decrement by empty row
//    if ((ui32)rowCount < rowPos) {
//        rowPos = rowCount; // append
//    }
//
//    CComPtr<IDispatch> spDispatch;
//    CComPtr<IHTMLElement> spRow;
//
//    //_bstr_t attr("idx");
//    //_variant_t val(m_rowCount);
//    hr = m_spTable->insertRow(rowPos, &spDispatch);
//    if (FAILED(hr)) {
//        return false;
//    }
//    //if (rowPos == m_rowCount) {
//    //    ++m_rowCount;
//    //}
//    spRow = spDispatch;
//    //spRow->setAttribute(attr, val);
//    CComPtr<IHTMLElement> spCell;
//
//    hr = m_spHtmlDocument2->createElement(m_td, &spCell);
//    if (FAILED(hr)) {
//        return false;
//    }
//    CComQIPtr<IHTMLDOMNode> spNode(spRow), spChild(spCell);
//    hr = spNode->appendChild(spChild, 0);
//    if (FAILED(hr)) {
//        return false;
//    }
//
//    CComPtr<IHTMLElement> spP;
//
//    hr = m_spHtmlDocument2->createElement(m_p, &spP);
//    if (FAILED(hr)) {
//        return false;
//    }
//
//    hr = spP->put_className(_bstr_t("nobreakpoint")/*m_breakpoint*/);
//    if (FAILED(hr)) {
//        return false;
//    }
//    _variant_t var;
//    hr = m_spEventHolder->get_onmouseover(&var);
//    hr = spP->put_onmouseover(var);
//    var.Clear();
//    hr = m_spEventHolder->get_onmouseout(&var);
//    hr = spP->put_onmouseout(var);
//    var.Clear();
//    //hr = m_spEventHolder->get_ondblclick(&var);
//    //hr = spP->put_ondblclick(var);
//    //var.Clear();
//    hr = m_spEventHolder->get_onclick(&var);
//    _variant_t vAttr;
//    hr = m_spEventHolder->getAttribute(_bstr_t("onclick"), 0, &vAttr);
//    //hr = spP->put_onclick(var);
//    hr = m_htmlElemEvents.DispEventAdvise(spP);
//    if (FAILED(hr)) {
//        return false;
//    }
//    spNode = spChild;
//    spChild = spP;
//    hr = spNode->appendChild(spChild, 0);
//    if (FAILED(hr)) {
//        return false;
//    }
//    spCell.Release();
//    hr = m_spHtmlDocument2->createElement(m_td, &spCell);
//    if (FAILED(hr)) {
//        return false;
//    }
//    hr = spCell->put_className(m_codecell);
//    if (FAILED(hr)) {
//        return false;
//    }
//    hr = spCell->put_onclick(var);//"ActivateEdit(this)"
//
//    spNode = spRow;
//    spChild = spCell;
//    hr = spNode->appendChild(spChild, 0);
//    if (FAILED(hr)) {
//        return false;
//    }
//    spP.Release();
//    hr = m_spHtmlDocument2->createElement(m_p, &spP);
//    if (FAILED(hr)) {
//        return false;
//    }
//    hr = spP->put_className(m_code);
//    if (FAILED(hr)) {
//        return false;
//    }
//    spNode = spCell;
//    spChild = spP;
//    hr = spNode->appendChild(spChild, 0);
//    if (FAILED(hr)) {
//        return false;
//    }
//
//    _bstr_t bstr(wsText);
//    if (*wsText == 0) {
//        bstr = " ";
//    }
//
//    hr = spP->put_innerText(bstr);
//    if (FAILED(hr)) {
//        return false;
//    }
//
//    if (rowPos < (ui32)rowCount) { // rowPos < rowCount, it's insert, otherwise append
//        RefreshBreakpointMarkers(spRows, rowPos, true);
//    }
//    return true;
//}
//
//bool CScriptView::DeleteRow(i32 rowPos) {
//    if (m_spTable) {
//        HRESULT hr;
//        CComPtr<IHTMLElementCollection> spRows;
//        hr = m_spTable->get_rows(&spRows);
//        if (FAILED(hr)) {
//            return false;
//        }
//        i32 rowCount;
//        hr = spRows->get_length(&rowCount);
//        if (FAILED(hr)) {
//            return false;
//        }
//        --rowCount; // decrement by empty row
//        if (rowCount <= rowPos) {
//            return false;
//        }
//        _variant_t var(rowPos);
//        CComPtr<IDispatch>  spDispatch;
//        
//        hr = spRows->item(var, var, &spDispatch);
//        if (FAILED(hr)) {
//            return false;
//        }
//
//        CComQIPtr<IHTMLTableRow> spRow(spDispatch);
//        CComPtr<IHTMLElementCollection> spCells;
//        hr = spRow->get_cells(&spCells);
//        if (FAILED(hr)) {
//            return false;
//        }
//        var = 0;
//        spDispatch = 0;
//        hr = spCells->item(var, var, &spDispatch);
//        if (FAILED(hr)) {
//            return false;
//        }
//
//        CComQIPtr<IHTMLElement> spCell(spDispatch);
//        spDispatch = 0;
//        hr = spCell->get_children(&spDispatch);
//        if (FAILED(hr)) {
//            return false;
//        }
//        CComQIPtr<IHTMLElementCollection> spChildren(spDispatch);
//        var = 0;
//        spDispatch = 0;
//        hr = spChildren->item(var, var, &spDispatch);
//        if (FAILED(hr)) {
//            return false;
//        }
//        CComQIPtr<IHTMLElement> spP(spDispatch);
//        hr = m_htmlElemEvents.DispEventUnadvise(spP);
//        if (FAILED(hr)) {
//            return false;
//        }
//
//        RefreshBreakpointMarkers(spRows, (ui32)rowPos, false);
//        hr = m_spTable->deleteRow(rowPos);
//        if (S_OK == hr) {
//            return true;
//        }
//    }
//    return false;
//}
//
//void CScriptView::RefreshBreakpointMarkers(CComPtr<IHTMLElementCollection> &spRows, ui32 startRow, bool newRow) {
//    CMainFrame* pMainFrame = (CMainFrame*)theApp.GetMainWnd();
//    _bstr_t fileName, path;
//    GetPathAndName(fileName, path);
//    _variant_t vOldRow, vNewRow(startRow);
//    ui32 i = startRow;
//    if (!newRow) {
//        vOldRow = ++i;
//        pMainFrame->m_wndProperties.DeleteBreakpoint(path, fileName, _bstr_t(vOldRow));
//    }
//    i32 len = 0;
//    spRows->get_length(&len);
//    --len;// discard empty row
//    if (!len) {
//        return; // empty
//    }
//    for (; i < (ui32)len; ++i) {
//        vNewRow = i;
//        CComPtr<IDispatch> spDispatch;
//        HRESULT hr = spRows->item(vNewRow, vNewRow, &spDispatch);
//        if (FAILED(hr)) {
//            continue;
//        }
//        CComQIPtr<IHTMLTableRow> spRow(spDispatch);
//        CComPtr<IHTMLElementCollection> spCells;
//        hr = spRow->get_cells(&spCells);
//        if (FAILED(hr)) {
//            continue;
//        }
//        _variant_t var = 0;
//        spDispatch = 0;
//        hr = spCells->item(var, var, &spDispatch);
//        if (FAILED(hr)) {
//            continue;
//        }
//
//        CComQIPtr<IHTMLElement> spCell(spDispatch);
//        spDispatch = 0;
//        hr = spCell->get_children(&spDispatch);
//        if (FAILED(hr)) {
//            continue;
//        }
//        CComQIPtr<IHTMLElementCollection> spChildren(spDispatch);
//        var = 0;
//        spDispatch = 0;
//        hr = spChildren->item(var, var, &spDispatch);
//        if (FAILED(hr)) {
//            continue;
//        }
//        CComQIPtr<IHTMLElement> spP(spDispatch);
//        _bstr_t className;
//        spP->get_className(&className.GetBSTR());
//
//        if (_wcsicmp(className, L"invalidbreakpoint") == 0) {
//            vOldRow = (newRow ? i - 1 : i) + START_POSITION_OFFSET;
//            if (newRow) {
//                vOldRow = i - 1 + START_POSITION_OFFSET;
//                vNewRow.lVal += START_POSITION_OFFSET;
//            }
//            else {
//                vOldRow = i + START_POSITION_OFFSET;
//            }
//            pMainFrame->m_wndProperties.UpdateBreakpointLine(path, fileName, _bstr_t(vOldRow), _bstr_t(vNewRow));
//        }
//    }
//}
//
//BOOL CScriptView::CreateRow(CString &scriptLine) {
//    return InsertRow((LPCWSTR)scriptLine, 0xffffffff);
//    /*
//    HRESULT hr;
//    
//    CComPtr<IDispatch> spDispatch;
//    CComPtr<IHTMLElement> spRow;
//
//    //_bstr_t attr("idx");
//    //_variant_t val(m_rowCount);
//
//    hr = m_spTable->insertRow(m_rowCount++, &spDispatch);
//    if (FAILED(hr)) {
//        return false;
//    }
//    spRow = spDispatch;
//    //spRow->setAttribute(attr, val);
//    CComPtr<IHTMLElement> spCell;
//
//    hr = m_spHtmlDocument2->createElement(m_td, &spCell);
//    if (FAILED(hr)) {
//        return false;
//    }
//    CComQIPtr<IHTMLDOMNode> spNode(spRow), spChild(spCell);
//    hr = spNode->appendChild(spChild, 0);
//    if (FAILED(hr)) {
//        return false;
//    }
//
//    CComPtr<IHTMLElement> spP;
//
//    hr = m_spHtmlDocument2->createElement(m_p, &spP);
//    if (FAILED(hr)) {
//        return false;
//    }
//
//    hr = spP->put_className(_bstr_t("nobreakpoint"));
//    if (FAILED(hr)) {
//        return false;
//    }
//    _variant_t var;
//    hr = m_spEventHolder->get_onmouseover(&var);
//    hr = spP->put_onmouseover(var);
//    var.Clear();
//    hr = m_spEventHolder->get_onmouseout(&var);
//    hr = spP->put_onmouseout(var);
//    var.Clear();
//    hr = m_spEventHolder->get_onclick(&var);
//    _variant_t vAttr;
//    hr = m_spEventHolder->getAttribute(_bstr_t("onclick"), 0, &vAttr);
//    //hr = spP->put_onclick(var);
//    hr = m_htmlElemEvents.DispEventAdvise(spP);
//    if (FAILED(hr)) {
//        return false;
//    }
//    spNode = spChild;
//    spChild = spP;
//    hr = spNode->appendChild(spChild, 0);
//    if (FAILED(hr)) {
//        return false;
//    }
//    spCell.Release();
//    hr = m_spHtmlDocument2->createElement(m_td, &spCell);
//    if (FAILED(hr)) {
//        return false;
//    }
//    hr = spCell->put_className(m_codecell);
//    if (FAILED(hr)) {
//        return false;
//    }
//    hr = spCell->put_onclick(var);//"ActivateEdit(this)"
//
//    spNode = spRow;
//    spChild = spCell;
//    hr = spNode->appendChild(spChild, 0);
//    if (FAILED(hr)) {
//        return false;
//    }
//    spP.Release();
//    hr = m_spHtmlDocument2->createElement(m_p, &spP);
//    if (FAILED(hr)) {
//        return false;
//    }
//    hr = spP->put_className(m_code); 
//    if (FAILED(hr)) {
//        return false;
//    }
//    spNode = spCell;
//    spChild = spP;
//    hr = spNode->appendChild(spChild, 0);
//    if (FAILED(hr)) {
//        return false;
//    }
//
//    _bstr_t bstr(scriptLine);
//    hr = spP->put_innerText(bstr);
//    if (FAILED(hr)) {
//        return false;
//    }
//    return true;*/
//}
//
//void CScriptView::SelectLine() {
//
//}

void CScriptView::GetPathAndName(_bstr_t &fileName, _bstr_t &path) {
    int sep = m_ScriptFileName.ReverseFind(L'\\');
    path = (LPCWSTR)m_ScriptFileName.Left(sep);
    fileName = m_ScriptFileName.Right(m_ScriptFileName.GetLength() - sep - 1);
}

#ifdef RICH_EDIT
void CScriptView::LoadBreakpointMarkers() {
    CMainFrame* pMainFrame = (CMainFrame*)theApp.GetMainWnd();
    _bstr_t fileName, path;

    GetPathAndName(fileName, path);

    //if (invalidate) {
    //    pMainFrame->m_wndProperties.InvalidateAllBreakpoints();
    //}
    //else {
    //    pMainFrame->m_wndProperties.MarkAllValidBreakpoints(path, fileName);
    //}

    //if (invalidate || !theApp.GetDebugger()) {
        vector<BREAKPOINT_INFO> breakpoints;

        pMainFrame->m_wndProperties.GetBreakpoints(path, fileName, breakpoints);
//#ifdef RICH_EDIT
        CChildFrame* pFrame = (CChildFrame*)GetParentFrame();
//#endif // RICH_EDIT
        for (ui32 i = 0; i < breakpoints.size(); ++i) {
            BREAKPOINT_INFO &breakpointInfo = breakpoints[i];
            ui32 line = breakpointInfo.line - START_POSITION_OFFSET;
            //if (!invalidate) { // collect initiali line numbers, so after editting and not saveing the lines can be restored in properties
            //    m_InitialBreakpoints.insert(breakpointInfo.line);
            //}
//#ifdef RICH_EDIT
            pFrame->SetBreakpointMarker(line, breakpointInfo.marker);
//#else // RICH_EDIT
            //SetBreakPointMarker(line, breakpointInfo.marker);
//#endif // RICH_EDIT

        }
    //}
}
#endif // RICH_EDIT

void CScriptView::SetAllBreakpointsMarker(bool invalidate) {
    CMainFrame* pMainFrame = (CMainFrame*)theApp.GetMainWnd();
    _bstr_t fileName, path;

    GetPathAndName(fileName, path);

    if (invalidate) {
        pMainFrame->m_wndProperties.InvalidateAllBreakpoints();
    }
    else {
        pMainFrame->m_wndProperties.MarkAllValidBreakpoints(path, fileName);
    }

    if (invalidate || !theApp.GetDebugger()) {
        vector<BREAKPOINT_INFO> breakpoints;

        pMainFrame->m_wndProperties.GetBreakpoints(path, fileName, breakpoints);
#ifdef RICH_EDIT
        CChildFrame* pFrame = (CChildFrame*)GetParentFrame();
#endif // RICH_EDIT
        for (ui32 i = 0; i < breakpoints.size(); ++i) {
            BREAKPOINT_INFO &breakpointInfo = breakpoints[i];
            ui32 line = breakpointInfo.line - START_POSITION_OFFSET;
            //if (!invalidate) { // collect initiali line numbers, so after editting and not saveing the lines can be restored in properties
            //    m_InitialBreakpoints.insert(breakpointInfo.line);
            //}
#ifdef RICH_EDIT
            pFrame->SetBreakpointMarker(line, breakpointInfo.marker);
#else // RICH_EDIT
            SetBreakPointMarker(line, breakpointInfo.marker);
#endif // RICH_EDIT

        }
    }
}

void CScriptView::EnableEditing(bool enable) {
    CRichEditCtrl &ctl = GetRichEditCtrl();
    ctl.SetReadOnly(!enable);
//    if (m_spHtmlDocument2) {
//        if (!enable) { // dismiss inputHolder
//            CComQIPtr<IHTMLDocument3> spHtmlDocument3(m_spHtmlDocument2);
//            CComPtr<IHTMLElement> spElement;
//            _bstr_t bstrInputHolder("inputHolder");
//            HRESULT hr = spHtmlDocument3->getElementById(bstrInputHolder, &spElement);
//            //CComQIPtr<IHTMLElement2> spElement2(spElement);
//            CComQIPtr<IDispatchEx> spDispEx(spElement);
//            if (spDispEx) {
//                //DISPID_THIS
//                //hr = spElement2->blur();
//                CComPtr<IHTMLElement> spParent;
//                spElement->get_parentElement(&spParent);
//                if (spParent) {
//                    //putid = DISPID_THIS;
//                    _variant_t var(spParent);
//                    //var.vt = VT_DISPATCH;
//                    //var.pdispVal = pdispObj;
//                    DISPPARAMS dispparams;
//                    DISPID dispid;
//                    hr = spDispEx->GetDispID(L"onblur", 0, &dispid);
//                    dispparams.rgvarg = &var;
//                    dispparams.rgdispidNamedArgs = 0;// &putid;
//                    dispparams.cArgs = 1;
//                    dispparams.cNamedArgs = 0;// 1;
//                    //spElement->Invoke(0, LOCALE_USER_DEFAULT,)
//                    hr = spDispEx->InvokeEx(dispid, LOCALE_USER_DEFAULT,
//                        DISPATCH_METHOD, &dispparams,
//                        NULL, NULL, NULL);
//                }
//            }
//        }
//        CComPtr<IDispatch> spDispatch;
//        m_spHtmlDocument2->get_Script(&spDispatch);
//        CComQIPtr<IDispatchEx> spDispatchEx(spDispatch);
//        if (spDispatchEx) {
//            DISPID putid = DISPID_PROPERTYPUT, dispid;
//            DISPPARAMS dispparams;
//            _variant_t var(enable);
//
//            //m_pNotify = pNotify;
//            //m_bridgeDispid = bridgeDispid;
//            //m_bstrFooName   = fooName;
//
//            //HRESULT hr = spDispatchEx->GetDispID(fooName, 0, &dispid);
//
//            //DISPID dispid;
//            //DISPPARAMS dispparams = { 0 };
//
//            HRESULT hr = spDispatchEx->GetDispID(_bstr_t("App_EnableEditing"), 0, &dispid);
//            if (S_OK == hr) {
//
//                dispparams.rgvarg = &var;
//                dispparams.rgdispidNamedArgs = &putid;
//                dispparams.cArgs = 1;
//                dispparams.cNamedArgs = 1;
//
//                //hr = spDispatchEx->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUTREF, &dispparams, NULL, NULL, NULL);
//
//                hr = spDispatchEx->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &dispparams, NULL, NULL, NULL);
//            }
//        }
//    }
}

void CScriptView::OnDocumentComplete(LPCTSTR lpszURL)
{
#ifndef RICH_EDIT
    CHtmlView::OnDocumentComplete(lpszURL);

    ReadScriptAsHTML();
    
    EnableEditing(theApp.GetDebugger() == 0);

    SetAllBreakpointsMarker(false);
#else // !RICH_EDIT
    LoadBreakpointMarkers();
#endif // !RICH_EDIT


    if (m_breakHitData.threadId) {
        HighlightLine(m_highlightType, m_breakHitData.fileId, m_breakHitData.line, m_breakHitData.threadId, m_breakHitData.pScope);
        memset(&m_breakHitData, 0, sizeof(m_breakHitData));
    }
}
//
//void CScriptView::OnClick(CComPtr<IHTMLEventObj> spHTMLEvent) {
//    CComPtr<IHTMLElement> spElement;
//    HRESULT hr = spHTMLEvent->get_srcElement(&spElement);
//    if (!spElement) {
//        return;
//    }
//    _bstr_t className;
//    spElement->get_className(&className.GetBSTR());
//    CComPtr<IHTMLElement> spParent;
//    hr = spElement->get_parentElement(&spParent);
//    if (!spParent) {
//        return;
//    }
//    spElement = spParent;
//    spParent.Release();
//    hr = spElement->get_parentElement(&spParent);
//    if (!spParent) {
//        return;
//    }
//    //_bstr_t attr("idx");
//    _bstr_t attr("rowIndex");
//    _variant_t val;
//    hr = spParent->getAttribute(attr, 0, &val);
//    if (FAILED(hr)) {
//        return;
//    }
//
//    BREAKPOINT_MARKER marker = (className == _bstr_t("nobreakpoint")) ? BREAKPOINT_INVALID : BREAKPOINT_NONE;
//
//    SetBreakPointMarker(val, marker);
//
//    CMainFrame* pMainFrame = (CMainFrame*)theApp.GetMainWnd();
//    _bstr_t fileName, path;
//    GetPathAndName(fileName, path);
//    val.ulVal += START_POSITION_OFFSET;
//    if (marker == BREAKPOINT_INVALID) {
//        // InsertBreakPoint will update the marker if the script is running
//        pMainFrame->m_wndProperties.InsertBreakpoint(path, fileName, _bstr_t(val));
//    }
//    else {
//        pMainFrame->m_wndProperties.DeleteBreakpoint(path, fileName, _bstr_t(val));
//    }
//}
//
//void  CScriptView::OnMouseOver(CComPtr<IHTMLEventObj> spHTMLEvent) {
//
//}
//
//void  CScriptView::OnMouseOut(CComPtr<IHTMLEventObj> spHTMLEvent) {
//
//}
//
//void CScriptView::OnJScriptFunctionCall(IHTMLElemEventsNotify::BRIDGE_DISPID bridgeDispid, DISPPARAMS* pdp, VARIANT* pvarRes) {
//
//    switch (bridgeDispid) {
//        case APP_INSERT_ROW:
//            if (pdp->cArgs == 2) {
//                pdp->rgvarg[0].bstrVal;
//                InsertRow(pdp->rgvarg[0].bstrVal, pdp->rgvarg[1].lVal);
//            }
//            break;
//        case APP_DELETE_ROW:
//            if (pdp->cArgs == 1) {
//                DeleteRow(pdp->rgvarg[0].lVal);
//            }
//            break;
//        case APP_SCRIPT_EDITED:
//            break;
//        case APP_ENABLE_EDITING:
//            break;
//        default:
//            break;
//    }
//}

void CScriptView::SetBreakPointMarker(ui32 line, BREAKPOINT_MARKER marker) {
#ifdef RICH_EDIT
    CChildFrame *pFrame = (CChildFrame*) GetParentFrame();
    pFrame->SetBreakpointMarker(line, marker);
    return;
#endif // RICH_EDIT

    //if (!m_spTable) {
    //    return;
    //}
    //CComPtr<IHTMLElementCollection> spRows;
    //m_spTable->get_rows(&spRows);
    //if (!spRows) {
    //    return;
    //}
    //i32 len = 0;
    //spRows->get_length(&len);
    //if ((ui32)--len <= line) { // discard empty row
    //    return;
    //}
    //CComPtr<IDispatch> spDispatch;
    //CComQIPtr<IHTMLTableRow> spRow;
    //spRows->item(_variant_t(line), _variant_t(line), &spDispatch);
    //spRow = spDispatch;
    //if (!spRow) {
    //    return;
    //}
    //CComPtr<IHTMLElementCollection> spCells;
    //CComQIPtr<IHTMLElement> spElement(spCells);
    //spRow->get_cells(&spCells);
    //if (!spCells) {
    //    return;
    //}
    //spDispatch.Release();
    //spCells->item(_variant_t(0), _variant_t(0), &spDispatch);
    //if (!spDispatch) {
    //    return;
    //}
    //CComQIPtr<IHTMLDOMNode> spDomNode(spDispatch);
    //CComPtr<IHTMLDOMNode> spDomChild;
    //spDomNode->get_firstChild(&spDomChild);
    //if (!spDomChild) {
    //    return;
    //}
    //spElement.Release();
    //spElement = spDomChild;
    //if (!spElement) {
    //    return;
    //}

    //CComPtr<IHTMLStyle> spStyle;
    //spElement->get_style(&spStyle);
    //
    //_bstr_t bstrLine = _variant_t(line + START_POSITION_OFFSET);
    //_bstr_t fileName, path;
    //GetPathAndName(fileName, path);
    //CMainFrame* pMainFrame = (CMainFrame*)theApp.GetMainWnd();
    //switch (marker) {
    //    case BREAKPOINT_NONE:
    //        spElement->put_className(_bstr_t("nobreakpoint"));
    //        spStyle->setAttribute(_bstr_t("background-image"), _variant_t("none"));
    //        break;
    //    case BREAKPOINT_INVALID:
    //        spElement->put_className(_bstr_t("invalidbreakpoint"));
    //        spStyle->setAttribute(_bstr_t("background-position-x"), _variant_t("0px"));
    //        spStyle->setAttribute(_bstr_t("background-image"), _variant_t("url(2/319)"));
    //        break;
    //    case BREAKPOINT_VALID:
    //        spElement->put_className(_bstr_t("breakpoint"));
    //        spStyle->setAttribute(_bstr_t("background-position-x"), _variant_t("20px"));
    //        spStyle->setAttribute(_bstr_t("background-image"), _variant_t("url(2/319)"));
    //        break;
    //    default:
    //        break;
    //}
}

BOOL CScriptView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
    return CHtmlView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

bool CScriptView::IsModified() {
#ifdef RICH_EDIT
    return GetRichEditCtrl().GetModify() == TRUE;
#else // RICH_EDIT
    if (m_modified) {
        return true;
    }
    _variant_t varValue(false);
    GetHtmlVarValue("App_ScriptEdited", varValue);
    return m_modified = (bool)varValue;
#endif // RICH_EDIT
}

void CScriptView::OnDestroy() {
    CHtmlView::OnDestroy();
   // m_keywords.GetSize();
    POSITION pos;
    CString key;
    KeyWordData *p;
    for (pos = m_keywords.GetStartPosition(); pos != NULL;)
    {
        m_keywords.GetNextAssoc(pos, key, (void *&)p);
        delete p;
    }
    
}

//bool CScriptView::Save(CArchive& ar) {
//    HRESULT hr;
//    if (!m_spTable) {
//        return false;
//    }
//
//    CComPtr<IHTMLElementCollection> spRows;
//    hr = m_spTable->get_rows(&spRows);
//
//    if (hr != S_OK) {
//        return false;
//    }
//    long count;
//    hr = spRows->get_length(&count);
//    if (hr != S_OK) {
//        return false;
//    }
//    --count;
//    for (long i = 0; i < count; ++i) {
//        _variant_t var(i);
//        CComPtr<IDispatch> spDisp;
//        spRows->item(var, var, &spDisp);
//        CComQIPtr<IHTMLTableRow> spRow(spDisp);
//
//        if (!spRow) continue;
//
//        CComPtr<IHTMLElementCollection> spCells;
//        spRow->get_cells(&spCells);
//
//        if (!spCells) continue;
//
//        spDisp.Release();
//        var = 1;
//        spCells->item(var, var, &spDisp);
//
//        CComQIPtr<IHTMLElement> spElem(spDisp);
//
//        if (spElem != NULL) {
//            _bstr_t bstr;
//            spElem->get_innerText(&bstr.GetBSTR());
//            if (bstr.length()) {
//                ar.Write((LPSTR)bstr, bstr.length());
//            }
//            if (i != count - 1) {
//                ar.Write("\r\n", 2);
//            }
//        }
//    }
//
//    CMainFrame* pMainFrame = (CMainFrame*)theApp.GetMainWnd();
//    _bstr_t fileName, path;
//
//    GetPathAndName(fileName, path);
//
//    //pMainFrame->m_wndProperties.RestoreBreakpopints(path, fileName, m_InitialBreakpoints);
//
//    _variant_t varValue(false);
//    if (SetHtmlVarValue("App_ScriptEdited", varValue)) {
//       m_modified = false;
//    }
//    return true;
//}
//
//bool CScriptView::SetHtmlVarValue(_bstr_t varName, _variant_t &varValue) {
//    if (m_spHtmlDocument2) {
//        CComPtr<IDispatch> spDispatch;
//        m_spHtmlDocument2->get_Script(&spDispatch);
//        CComQIPtr<IDispatchEx> spDispatchEx(spDispatch);
//
//        if (spDispatchEx) {
//            DISPID putid = DISPID_PROPERTYPUT, dispid;
//            DISPPARAMS dispparams;
//
//            HRESULT hr = spDispatchEx->GetDispID(varName, 0, &dispid);
//            if (S_OK == hr) {
//                dispparams.rgvarg = &varValue;
//                dispparams.rgdispidNamedArgs = &putid;
//                dispparams.cArgs = 1;
//                dispparams.cNamedArgs = 1;
//
//                hr = spDispatchEx->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &dispparams, NULL, NULL, NULL);
//                return hr == S_OK;
//            }
//        }
//    }
//    return false;
//}
//
//bool CScriptView::GetHtmlVarValue(_bstr_t varName, _variant_t &varValue) {
//    if (m_spHtmlDocument2) {
//        CComPtr<IDispatch> spDispatch;
//        m_spHtmlDocument2->get_Script(&spDispatch);
//        CComQIPtr<IDispatchEx> spDispatchEx(spDispatch);
//        if (spDispatchEx) {
//            DISPID dispid;
//            DISPPARAMS dispparams = { 0 };
//
//            HRESULT hr = spDispatchEx->GetDispID(varName, 0, &dispid);
//            if (S_OK == hr) {
//                hr = spDispatchEx->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET | DISPATCH_METHOD, &dispparams, &varValue, NULL, NULL);
//                return hr == S_OK;
//            }
//        }
//    }
//    return false;
//}
#ifdef RICH_EDIT
void CScriptView::OnInitialUpdate()
{
    CRichEditView::OnInitialUpdate();
    //UINT ui = GetRichEditCtrl().GetWordWrapMode();
    //GetRichEditCtrl().SetWordWrapMode(WBF_WORDWRAP);

    //GetRichEditCtrl().SetTargetDevice(*GetRichEditCtrl().GetDC(), INT_MAX);
    //GetRichEditCtrl().ShowScrollBar(SB_HORZ, 0);

    //SetWordWrapMode()
    m_nWordWrap = WrapNone;
    WrapChanged();

    // Set the printing margins (720 twips = 1/2 inch)
    SetMargins(CRect(720, 720, 720, 720));

    const COLORREF rgbRed = 0x000000FF;
    const COLORREF rgbGreen = 0x00009900;
    const COLORREF rgbBlue = 0x00CD0000;
    const COLORREF rgbBlack = 0x00010101;
    const COLORREF rgbWhite = 0x00FFFFFF;
    const COLORREF rgbPurple = 0x00990099;
    const COLORREF rgbDarkCyan = 0x00BEC100;
    const COLORREF rgbBrown = 0x000000CC;
    const COLORREF rgbGray = 0x00909090;

    m_keywords.SetAt(L"i8", new KeyWordData(rgbDarkCyan));
    m_keywords.SetAt(L"ui8", new KeyWordData(rgbDarkCyan));
    m_keywords.SetAt(L"i16", new KeyWordData(rgbDarkCyan));
    m_keywords.SetAt(L"ui16", new KeyWordData(rgbDarkCyan));
    m_keywords.SetAt(L"i32", new KeyWordData(rgbDarkCyan));
    m_keywords.SetAt(L"ui32", new KeyWordData(rgbDarkCyan));
    m_keywords.SetAt(L"i64", new KeyWordData(rgbDarkCyan));
    m_keywords.SetAt(L"ui64", new KeyWordData(rgbDarkCyan));
    m_keywords.SetAt(L"double", new KeyWordData(rgbDarkCyan));
    m_keywords.SetAt(L"float", new KeyWordData(rgbDarkCyan));
    m_keywords.SetAt(L"bool", new KeyWordData(rgbDarkCyan));
    m_keywords.SetAt(L"string", new KeyWordData(rgbDarkCyan));

    m_keywords.SetAt(L"struct", new KeyWordData(rgbGreen));
    m_keywords.SetAt(L"lib", new KeyWordData(rgbGreen));
    m_keywords.SetAt(L"array", new KeyWordData(rgbGreen));

    m_keywords.SetAt(L"include", new KeyWordData(rgbBlue/*rgbGray*/));

    m_keywords.SetAt(L"error", new KeyWordData(rgbBrown));

    m_keywords.SetAt(L"function", new KeyWordData(rgbBlue));
    m_keywords.SetAt(L"for", new KeyWordData(rgbBlue));
    m_keywords.SetAt(L"while", new KeyWordData(rgbBlue));
    m_keywords.SetAt(L"switch", new KeyWordData(rgbBlue));
    m_keywords.SetAt(L"case", new KeyWordData(rgbBlue));
    m_keywords.SetAt(L"default", new KeyWordData(rgbBlue));
    m_keywords.SetAt(L"break", new KeyWordData(rgbBlue));
    m_keywords.SetAt(L"if", new KeyWordData(rgbBlue));
    m_keywords.SetAt(L"else", new KeyWordData(rgbBlue));
    m_keywords.SetAt(L"continue", new KeyWordData(rgbBlue));
    m_keywords.SetAt(L"return", new KeyWordData(rgbBlue));

    m_keywords.SetAt(L"sizeof", new KeyWordData(rgbPurple));
    m_keywords.SetAt(L"arr2str", new KeyWordData(rgbPurple));
    m_keywords.SetAt(L"warr2str", new KeyWordData(rgbPurple));
    m_keywords.SetAt(L"str2arr", new KeyWordData(rgbPurple));
    m_keywords.SetAt(L"str2warr", new KeyWordData(rgbPurple));
    m_keywords.SetAt(L"cast", new KeyWordData(rgbPurple));
    m_keywords.SetAt(L"lock", new KeyWordData(rgbPurple));
    m_keywords.SetAt(L"unlock", new KeyWordData(rgbPurple));
    CFont Font;
    Font.CreatePointFont(100, L"Consolas");
    this->SetFont(&Font);
    ParseText();
}

void CScriptView::ParseText() {
    CString ext = PathFindExtension(m_ScriptFileName);
    ext.MakeLower();
    if (ext != TEXT(EXT_TEXT_SCRIPT_STRING)) return;

    CRichEditCtrl& ctl = GetRichEditCtrl();
    int nLineLength, nLineIndex, nLineCount = ctl.GetLineCount();
    CString strText, strLine;
    int pos = 0;
    CHARFORMAT cfm;
    ctl.GetSelectionCharFormat(cfm);
    cfm.cbSize = sizeof(cfm);
	bool bLongComment = false;
    // Dump every line of text of the rich edit control.
    for (int k = 0; k < nLineCount; k++)
    {
        nLineIndex = ctl.LineIndex(k);
        nLineLength = ctl.LineLength(nLineIndex);
        if (!nLineLength) {
            ++pos;
            continue;
        }
        LPTSTR p = strText.GetBufferSetLength(nLineLength + 1);
        ctl.GetLine(k, p, nLineLength + 1);
        strText.SetAt(nLineLength, _T('\0')); // null terminate
        strText.ReleaseBuffer(nLineLength + 1);
		int LongCommentBeginPos = 0;
        CString str;
        for (int i = 0; i <= nLineLength; ++i) {
            TCHAR ch = strText[i];
			if (!bLongComment) {
				if (isalnum(ch)) {
					str.AppendChar(ch);
				}
				else if (str.GetLength()) {
					KeyWordData* pkw = 0;
					if (m_keywords.Lookup(str, (void*&)pkw)) {
						//return pkw;
						//long b = i - str.GetLength() + start, e = i + start;
						ctl.SetSel(pos + i - str.GetLength(), pos + i);
						cfm.crTextColor = pkw->color;
						cfm.dwMask = CFM_COLOR;// | CFM_BOLD;
						cfm.dwEffects = 0;//CFE_BOLD | CFE_AUTOCOLOR;
						ctl.SetSelectionCharFormat(cfm);
						//cfm.dwMask = CFM_BOLD;
						//cfm.dwEffects = CFE_BOLD;
						//ctl.SetSelectionCharFormat(cfm);
					}
					str = "";
				}

				if ((ch == _T('/')) && (i + 1 <= nLineLength) && (strText[i + 1] == _T('/'))) // comment out rest of the lilne
				{
					const COLORREF rgbGray = 0x00909090;
					//KeyWordData pkw(rgbGray);
					ctl.SetSel(pos + i, pos + nLineLength);
					cfm.crTextColor = rgbGray;
					cfm.dwMask = CFM_ITALIC | CFM_COLOR;// | CFM_BOLD;
					cfm.dwEffects = CFE_ITALIC;//CFE_BOLD | CFE_AUTOCOLOR;
					ctl.SetSelectionCharFormat(cfm);
					break;
				}

				if ((ch == _T('/')) && (i + 1 <= nLineLength) && (strText[i + 1] == _T('*'))) // comment out until end of comment marker - "*/"
				{
					bLongComment = true;
					LongCommentBeginPos = i;
				}
			}
			else {
				if ((ch == _T('*')) && (i + 1 <= nLineLength) && (strText[i + 1] == _T('/'))) // comment out until end of comment marker - "*/"
				{
					bLongComment = false;
					str.AppendChar(ch);
					str.AppendChar(strText[i + 1]);
					const COLORREF rgbGray = 0x00909090;
					ctl.SetSel(pos + LongCommentBeginPos, pos + i + 2);
					cfm.crTextColor = rgbGray;
					cfm.dwMask = CFM_ITALIC | CFM_COLOR;// | CFM_BOLD;
					cfm.dwEffects = CFE_ITALIC;//CFE_BOLD | CFE_AUTOCOLOR;
					ctl.SetSelectionCharFormat(cfm);
				}
				else {
					str.AppendChar(ch);
					if (i == nLineLength) {
						const COLORREF rgbGray = 0x00909090;
						//KeyWordData pkw(rgbGray);
						ctl.SetSel(pos + LongCommentBeginPos, pos + nLineLength);
						cfm.crTextColor = rgbGray;
						cfm.dwMask = CFM_ITALIC | CFM_COLOR;// | CFM_BOLD;
						cfm.dwEffects = CFE_ITALIC;//CFE_BOLD | CFE_AUTOCOLOR;
						ctl.SetSelectionCharFormat(cfm);
					}
				}
			}
        }
        pos += nLineLength + 1;

        //TRACE(_T("line %d: '%s'\r\n"), i, strText);
    }
    ctl.SetSel(0, 0);

    ctl.SetModify(FALSE);

    //long start, end;
    //ctl.GetSel(start, end);
    //int len = ctl.SendMessage(EM_LINELENGTH, start);
    //if (len == 0) return;

    //CHARFORMAT cfm;
    //ctl.GetSelectionCharFormat(cfm);
    //cfm.cbSize = sizeof(cfm);
    //cfm.crTextColor = 0;
    //
    //ctl.SetSel(0, len + 1);
    //cfm.dwMask = CFM_COLOR;// | CFM_BOLD;
    //cfm.dwEffects = 0;//CFE_BOLD | CFE_AUTOCOLOR;
    //ctl.SetSelectionCharFormat(cfm);
    //cfm.dwMask = CFM_BOLD;
    //cfm.dwEffects = 0;
    //ctl.SetSelectionCharFormat(cfm);


    //int curLine = ctl.LineFromChar(start);
    //LPTSTR pBuf = new TCHAR[len + 1];
    //ctl.GetLine(curLine, pBuf);
    //CString str;
    //for (int i = 0; i <= len; ++i) {
    //	TCHAR ch = pBuf[i];
    //	if (isalnum(ch)) {
    //		str.AppendChar(ch);
    //	}
    //	else {
    //		if (str.GetLength()) {
    //               KeyWordData* pkw = 0;
    //               if (m_keywords.Lookup(str, (void*&)pkw)) {
    //                   //return pkw;
    //				long b = i - str.GetLength() + start, e = i + start;
    //				ctl.SetSel(b, e);
    //				cfm.crTextColor = pkw->color;
    //				cfm.dwMask = CFM_COLOR;// | CFM_BOLD;
    //				cfm.dwEffects = 0;//CFE_BOLD | CFE_AUTOCOLOR;
    //				ctl.SetSelectionCharFormat(cfm);
    //				cfm.dwMask = CFM_BOLD;
    //				cfm.dwEffects = CFE_BOLD;
    //				ctl.SetSelectionCharFormat(cfm);
    //               }
    //               str = "";
    //           }
    //	}
    //}
    //ctl.SetSel(start, end);
}
void CScriptView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    long start, end, pos;
    CRichEditCtrl &ctl = GetRichEditCtrl();

    ctl.GetSel(start, end);
    if (start != end) {
        CRichEditView::OnKeyUp(nChar, nRepCnt, nFlags);
        return;
    }
    int origStart = start, origEnd = end;
    const int maxKeywordLen = 8 + 3;
    //longest keword is 8 chars, so get up to 8 chars right and left
    //pos = end;
    //int right = maxKeywordLen;

    end = end + maxKeywordLen;//((len - end) >= maxKeywordLen) ? (end + maxKeywordLen) : len;
    start = (start >= maxKeywordLen) ? (start - maxKeywordLen) : 0;
    //ctl.SetSel(0, end);
    CString str;
    ctl.GetTextRange(start, end, str);
    end = start + str.GetLength();
    if (start == end) return;
    CString out;
    long offset0 = 0, offset1 = 0;
    //check if char at this position is not alpha or digit
    pos = origStart - start;
    KeyWordData* pkw;
    pkw = IsKeyWord(str, pos, offset0, offset1);
    MarkKeyWord(pkw, start + offset0, start + offset1, origEnd);
    if (pos) {
        --pos;
        pkw = IsKeyWord(str, pos, offset0, offset1);
        MarkKeyWord(pkw, start + offset0, start + offset1, origEnd);
        if (!isalnum(str[pos])) {
            if (pos) {
                --pos;
                pkw = IsKeyWord(str, pos, offset0, offset1);
                MarkKeyWord(pkw, start + offset0, start + offset1, origEnd);
            }
        }
    }
}

static const char* const _SBcode(UINT n) {
    switch (n) {
        case SB_LINEUP:
            return "SB_LINEUP";
        case SB_LINEDOWN:
            return "SB_LINEDOWN";
        case SB_PAGEUP:
            return "SB_PAGEUP";
        case SB_PAGEDOWN:
            return "SB_PAGEDOWN";
        case SB_THUMBPOSITION:
            return "SB_THUMBPOSITION";
        case SB_THUMBTRACK:
            return "SB_THUMBTRACK";
        case SB_TOP:
            return "SB_TOP";
        case SB_BOTTOM:
            return "SB_BOTTOM";
        case SB_ENDSCROLL:
            return "SB_ENDSCROLL";
        default:
            return "Unknown";
    }
}
void CScriptView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // TODO: Add your message handler code here and/or call default
    //int pos = GetScrollPos(SB_VERT);
    CRichEditView::OnVScroll(nSBCode, nPos, pScrollBar);
    
    //SCROLLINFO si = { 0 };
    //si.cbSize = sizeof(si);
    //GetScrollInfo(SB_VERT, &si);
    //
    /*
    nSBCode
    Specifies a scroll-bar code that indicates the user's scrolling request. This parameter can be one of the following:

    SB_BOTTOM Scroll to bottom.

    SB_ENDSCROLL End scroll.

    SB_LINEDOWN Scroll one line down.

    SB_LINEUP Scroll one line up.

    SB_PAGEDOWN Scroll one page down.

    SB_PAGEUP Scroll one page up.

    SB_THUMBPOSITION Scroll to the absolute position. The current position is provided in nPos.

    SB_THUMBTRACK Drag scroll box to specified position. The current position is provided in nPos.

    SB_TOP Scroll to top.

    nPos
    Contains the current scroll-box position if the scroll-bar code is SB_THUMBPOSITION or SB_THUMBTRACK;
    otherwise not used. Depending on the initial scroll range, nPos may be negative and should be cast
    to an int if necessary.
    #define SB_LINEUP           0
    #define SB_LINELEFT         0
    #define SB_LINEDOWN         1
    #define SB_LINERIGHT        1
    #define SB_PAGEUP           2
    #define SB_PAGELEFT         2
    #define SB_PAGEDOWN         3
    #define SB_PAGERIGHT        3
    #define SB_THUMBPOSITION    4
    #define SB_THUMBTRACK       5
    #define SB_TOP              6
    #define SB_LEFT             6
    #define SB_BOTTOM           7
    #define SB_RIGHT            7
    #define SB_ENDSCROLL        8
    *///SB_BOTTOM

    //const char*const _ch = _SBcode(nSBCode);
    switch (nSBCode) {
        case SB_THUMBTRACK: 
            break;
        case SB_LINEUP:
        case SB_LINEDOWN:
        case SB_PAGEUP:
        case SB_PAGEDOWN:
        case SB_TOP:
        case SB_BOTTOM:
            nPos = GetScrollPos(SB_VERT);
            break;
        default:
            return;
    }
    CChildFrame* pFrame = (CChildFrame*)GetParentFrame();
    pFrame->SetPos(nPos); 
    pFrame->Invalidate();
    //pFrame->HandleVScroll(nPos);//RedrawWindow(&r, 0, RDW_NOCHILDREN | RDW_ERASE | RDW_INTERNALPAINT | RDW_INVALIDATE | RDW_UPDATENOW);

    //pos = pos;
}


void CScriptView::OnEnVscroll()
{
    // TODO: Add your control notification handler code here
    int pos = GetScrollPos(SB_VERT);
    CChildFrame* pFrame = (CChildFrame*)GetParentFrame();
    pFrame->SetPos(pos);
    pFrame->Invalidate();
    //pFrame->HandleVScroll(pos);
    //pFrame->SetPos(pos);
    //RECT r;
    //pFrame->GetClientRect(&r);
    //r.right = 20;
    //pFrame->InvalidateRect(&r);//RedrawWindow(&r, 0, RDW_NOCHILDREN | RDW_ERASE | RDW_INTERNALPAINT | RDW_INVALIDATE | RDW_UPDATENOW);
    //pos = pos;
}

CScriptView::KeyWordData* CScriptView::IsKeyWord(CString& s, long pos, long& begin, long& end) {
    begin = pos, end = pos + 1;
    if (s.GetLength() < 2/* smallest is i8*/) return 0;
    if (isalpha(s[pos]) || isdigit(s[pos])) { // extract keyword

        for (int i = pos; i < s.GetLength(); ++i) {
            if (isalpha(s[i]) || isdigit(s[i])) {
                end = i + 1;
                continue;
            }
            break;
        }
        for (int i = pos - 1; i >= 0; --i) {
            if (isalpha(s[i]) || isdigit(s[i])) {
                begin = i;
                continue;
            }
            break;
        }

        CString out = s.Mid(begin, end - begin);

        KeyWordData* pkw = 0;
        if (m_keywords.Lookup(out, (void*&)pkw)) {
            //--end;
            return pkw;
        }
    }
    else {

    }
    return 0;
}

void CScriptView::MarkKeyWord(KeyWordData* pkw, long start, long end, long origEnd) {
    CRichEditCtrl& ctl = GetRichEditCtrl();
    ctl.SetSel(start, end);
    if (pkw) {
        CHARFORMAT cfm;
        //ctl.GetSelectionCharFormat(cfm);
        cfm.cbSize = sizeof(cfm);
        cfm.crTextColor = pkw->color;

        cfm.dwMask = CFM_COLOR;// | CFM_BOLD;
        cfm.dwEffects = 0;//CFE_BOLD | CFE_AUTOCOLOR;
        ctl.SetSelectionCharFormat(cfm);

        //cfm.dwMask = CFM_BOLD;
        //cfm.dwEffects = CFE_BOLD;
        //ctl.SetSelectionCharFormat(cfm);
    }
    else {
        CHARFORMAT cfm;
        //ctl.GetSelectionCharFormat(cfm);
        cfm.cbSize = sizeof(cfm);
        cfm.crTextColor = 0;//pkw->color;
        cfm.dwMask = CFM_COLOR;// | CFM_BOLD;
        cfm.dwEffects = 0;//CFE_BOLD | CFE_AUTOCOLOR;
                          //ctl.SetSel(start, end + 1);
        ctl.SetSelectionCharFormat(cfm);
        //cfm.dwMask = CFM_BOLD;
        //cfm.dwEffects = 0;
        //ctl.SetSelectionCharFormat(cfm);
    }
    ctl.SetSel(origEnd, origEnd);
}

#endif // RICH_EDIT
