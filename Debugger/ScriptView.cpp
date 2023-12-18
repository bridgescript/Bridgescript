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
    m_currentLine(0) 
{
}

CScriptView::~CScriptView()
{
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

void CScriptView::HighlightLine(HIGHLIGHT_TYPE type, ui32 fileId, ui32 line, ThreadId threadId, Scope *pScope, ui32 pos) {
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

    if ((line < vl) || (line >= (r.bottom / pFrame->GetImgSizeY() + vl))) { // scroll
        i32 newLine = line - vl;
        int totalLines = ctl.GetLineCount();
        int maxscrollLine = totalLines - (r.bottom / pFrame->GetImgSizeY()) - 1;
        if (maxscrollLine < line) {
            newLine -= line - maxscrollLine;
        }
        else {
        }
        ctl.LineScroll(newLine, 0);
        pFrame->SetHighlight(line, type);
        int newscrollPos = GetScrollPos(SB_VERT);
        pFrame->SetPos(newscrollPos);
    }
    else {
        pFrame->SetHighlight(line, type);
    }

	if (HIGHLIGHT_COMPILE_ERROR == type) {
		CChildFrame* pFrame = (CChildFrame*)GetParentFrame();
		int scrollPos = GetScrollPos(SB_VERT); // get scroll position

		CPoint cp = ctl.GetCaretPos(); // for debugging only

		cp.x = 0;
		cp.y = (line * pFrame->GetImgSizeY()) - scrollPos + IMG_Y_OFFSET; // set Y offset

		int idx = ctl.CharFromPos(cp); // get character index at Point
		
		// get word triggered this error
		int nLineLength = ctl.LineLength(ctl.LineIndex(line)) + 1;
		CString strText;
		ctl.GetLine(line, strText.GetBufferSetLength(nLineLength), nLineLength);
		//CString word;
		int i = pos;
		for (; i < strText.GetLength(); ++i) {
			if (!isalnum(strText[i])) {
				break;
			}
		}

		ctl.SetSel(idx + pos, idx + pos + (pos == i ? 1 : i - pos)); // select the word

		ctl.SetModify(FALSE);
	}

    pFrame->Invalidate();
   
    if (HIGHLIGHT_ON_BREAK == type) {
        ((CMainFrame*)theApp.m_pMainWnd)->m_wndProperties.UpdateStackList(threadId);
    }

#endif // RICH_EDIT
}

void CScriptView::ClearHighlight(bool invalidateBreakpoints) {
#ifdef RICH_EDIT
    CChildFrame *pFrame = (CChildFrame*)GetParentFrame();
    pFrame->SetHighlight(0, HIGHLIGHT_OFF);
    pFrame->SetPos(GetScrollPos(SB_VERT));
    pFrame->Invalidate();
#endif // RICH_EDIT

    SetAllBreakpointsMarker(invalidateBreakpoints);
}

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

	vector<BREAKPOINT_INFO> breakpoints;

	pMainFrame->m_wndProperties.GetBreakpoints(path, fileName, breakpoints);
	CChildFrame* pFrame = (CChildFrame*)GetParentFrame();
	for (ui32 i = 0; i < breakpoints.size(); ++i) {
		BREAKPOINT_INFO& breakpointInfo = breakpoints[i];
		ui32 line = breakpointInfo.line - START_POSITION_OFFSET;
		pFrame->SetBreakpointMarker(line, breakpointInfo.marker);
	}
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

void CScriptView::SetBreakPointMarker(ui32 line, BREAKPOINT_MARKER marker) {
#ifdef RICH_EDIT
    CChildFrame *pFrame = (CChildFrame*) GetParentFrame();
    pFrame->SetBreakpointMarker(line, marker);
    return;
#endif // RICH_EDIT
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
    POSITION pos;
    CString key;
    KeyWordData *p;
    for (pos = m_keywords.GetStartPosition(); pos != NULL;)
    {
        m_keywords.GetNextAssoc(pos, key, (void *&)p);
        delete p;
    }
    
}

#ifdef RICH_EDIT
void CScriptView::OnInitialUpdate()
{
    CRichEditView::OnInitialUpdate();
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
	auto SetTextGrayFn = [&cfm, &ctl](long Start, long End)
	{
		const COLORREF rgbGray = 0x00909090;
		ctl.SetSel(Start, End);
		cfm.crTextColor = rgbGray;
		cfm.dwMask = CFM_ITALIC | CFM_COLOR;
		cfm.dwEffects = CFE_ITALIC;
		ctl.SetSelectionCharFormat(cfm);
	};
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
						ctl.SetSel(pos + i - str.GetLength(), pos + i);
						cfm.crTextColor = pkw->color;
						cfm.dwMask = CFM_COLOR;// | CFM_BOLD;
						cfm.dwEffects = 0;//CFE_BOLD | CFE_AUTOCOLOR;
						ctl.SetSelectionCharFormat(cfm);
					}
					str = "";
				}

				if ((ch == _T('/')) && (i + 1 <= nLineLength) && (strText[i + 1] == _T('/'))) // comment out rest of the lilne
				{
					SetTextGrayFn(pos + i, pos + nLineLength);
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
					SetTextGrayFn(pos + LongCommentBeginPos, pos + i + 2);
				}
				else {
					str.AppendChar(ch);
					if (i == nLineLength) {
						SetTextGrayFn(pos + LongCommentBeginPos, pos + nLineLength);
					}
				}
			}
        }
        pos += nLineLength + 1;

        //TRACE(_T("line %d: '%s'\r\n"), i, strText);
    }
    ctl.SetSel(0, 0);

    ctl.SetModify(FALSE);

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
}


void CScriptView::OnEnVscroll()
{
    // TODO: Add your control notification handler code here
    int pos = GetScrollPos(SB_VERT);
    CChildFrame* pFrame = (CChildFrame*)GetParentFrame();
    pFrame->SetPos(pos);
    pFrame->Invalidate();
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

        cfm.cbSize = sizeof(cfm);
        cfm.crTextColor = pkw->color;

        cfm.dwMask = CFM_COLOR;// | CFM_BOLD;
        cfm.dwEffects = 0;//CFE_BOLD | CFE_AUTOCOLOR;
        ctl.SetSelectionCharFormat(cfm);
    }
    else {
        CHARFORMAT cfm;

        cfm.cbSize = sizeof(cfm);
        cfm.crTextColor = 0;//pkw->color;
        cfm.dwMask = CFM_COLOR;// | CFM_BOLD;
        cfm.dwEffects = 0;//CFE_BOLD | CFE_AUTOCOLOR;
        ctl.SetSelectionCharFormat(cfm);
    }
    ctl.SetSel(origEnd, origEnd);
}

#endif // RICH_EDIT
