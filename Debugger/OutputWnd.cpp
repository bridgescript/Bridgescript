
#include "stdafx.h"

#include "OutputWnd.h"
#include "Resource.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputBar

void COutputWnd::Output::Print(PRINT_TYPE type, string &text) {
    switch (type) {
        case BUILD_OUT:
            m_pOutputWnd->m_wndOutputBuild.AddString(_bstr_t(text.c_str()));
            m_pOutputWnd->SetActiveTab(0);
            break;
        case DEBUG_OUT:
            m_pOutputWnd->m_wndOutputDebug.AddString(_bstr_t(text.c_str()));
            m_pOutputWnd->SetActiveTab(1);
            break;
    }
}

void COutputWnd::Output::Clear() {
    int count = m_pOutputWnd->m_wndOutputBuild.GetCount();
    for (int i = count; i >= 0; --i) {
        m_pOutputWnd->m_wndOutputBuild.DeleteString(i);
    }
    count = m_pOutputWnd->m_wndOutputDebug.GetCount();
    for (int i = count; i >= 0; --i) {
        m_pOutputWnd->m_wndOutputDebug.DeleteString(i);
    }
}

COutputWnd::COutputWnd() : 
    m_Output(this) {
}

COutputWnd::~COutputWnd()
{
}

BEGIN_MESSAGE_MAP(COutputWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
    //ON_WM_LBUTTONDBLCLK()
    ON_MESSAGE(WM_ON_SCRIPT_RUNTIME_ERROR, &COutputWnd::OnOnScriptRuntimeError)
END_MESSAGE_MAP()

int COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create tabs window:
	if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_FLAT, rectDummy, this, 1))
	{
		TRACE0("Failed to create output tab window\n");
		return -1;      // fail to create
	}

	// Create output panes:
	const DWORD dwStyle = LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;

	if (!m_wndOutputBuild.Create(dwStyle, rectDummy, &m_wndTabs, 2) ||
		!m_wndOutputDebug.Create(dwStyle, rectDummy, &m_wndTabs, 3) ||
		!m_wndOutputFind.Create(dwStyle, rectDummy, &m_wndTabs, 4))
	{
		TRACE0("Failed to create output windows\n");
		return -1;      // fail to create
	}

	UpdateFonts();

	CString strTabName;
	BOOL bNameValid;

	// Attach list windows to tab:
	bNameValid = strTabName.LoadString(IDS_BUILD_TAB);
	ASSERT(bNameValid);
	m_wndTabs.AddTab(&m_wndOutputBuild, strTabName, (UINT)0);
	bNameValid = strTabName.LoadString(IDS_DEBUG_TAB);
	ASSERT(bNameValid);
	m_wndTabs.AddTab(&m_wndOutputDebug, strTabName, (UINT)1);
	bNameValid = strTabName.LoadString(IDS_FIND_TAB);
	ASSERT(bNameValid);
	m_wndTabs.AddTab(&m_wndOutputFind, strTabName, (UINT)2);

	// Fill output tabs with some dummy text (nothing magic here)
	FillBuildWindow();
	FillDebugWindow();
	FillFindWindow();

	return 0;
}

void COutputWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// Tab control should cover the whole client area:
	m_wndTabs.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
    AdjustHorzScroll(m_wndOutputBuild);
    AdjustHorzScroll(m_wndOutputDebug);
    AdjustHorzScroll(m_wndOutputFind);
}

void COutputWnd::AdjustHorzScroll(CListBox& wndListBox)
{
	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);

	int cxExtentMax = 0;

	for (int i = 0; i < wndListBox.GetCount(); i ++)
	{
		CString strItem;
		wndListBox.GetText(i, strItem);

		cxExtentMax = max(cxExtentMax, (int)dc.GetTextExtent(strItem).cx);
	}

	wndListBox.SetHorizontalExtent(cxExtentMax);
	dc.SelectObject(pOldFont);
}

void COutputWnd::FillBuildWindow()
{
}

void COutputWnd::FillDebugWindow()
{
}

void COutputWnd::FillFindWindow()
{
	m_wndOutputFind.AddString(_T("Find output is being displayed here."));
	m_wndOutputFind.AddString(_T("The output is being displayed in rows of a list view"));
	m_wndOutputFind.AddString(_T("but you can change the way it is displayed as you wish..."));
}

void COutputWnd::UpdateFonts()
{
	m_wndOutputBuild.SetFont(&afxGlobalData.fontRegular);
	m_wndOutputDebug.SetFont(&afxGlobalData.fontRegular);
	m_wndOutputFind.SetFont(&afxGlobalData.fontRegular);
}

/////////////////////////////////////////////////////////////////////////////
// COutputList1

COutputList::COutputList()
{
}

COutputList::~COutputList()
{
}

BEGIN_MESSAGE_MAP(COutputList, CListBox)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
	ON_WM_WINDOWPOSCHANGING()
    ON_CONTROL_REFLECT(LBN_DBLCLK, &COutputList::OnLbnDblclk)
    ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// COutputList message handlers

void COutputList::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_OUTPUT_POPUP);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}

	SetFocus();
}

void COutputList::OnEditCopy()
{
	MessageBox(_T("Copy output"));
}

void COutputList::OnEditClear()
{
	MessageBox(_T("Clear output"));
}

void COutputList::OnViewOutput()
{
	CDockablePane* pParentBar = DYNAMIC_DOWNCAST(CDockablePane, GetOwner());
	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetTopLevelFrame());

	if (pMainFrame != NULL && pParentBar != NULL)
	{
		pMainFrame->SetFocus();
		pMainFrame->ShowPane(pParentBar, FALSE, FALSE, FALSE);
		pMainFrame->RecalcLayout();

	}
}


void COutputList::OnLbnDblclk()
{
    // TODO: Add your control notification handler code here
    int sel = GetCurSel();
    if (sel == -1) return;
    CString s;
    GetText(sel, s);
    HIGHLIGHT_TYPE type = HIGHLIGHT_COMPILE_ERROR;
    if (0 == s.Find(_T("RUNTIME ERROR:"))) {
        type = HIGHLIGHT_RUNTIME_ERROR;
    }
    int pathStart = s.Find(_T("\""), 0);
    if (pathStart != -1) {
        ++pathStart;
        int pathEnd = s.Find(_T("\""), pathStart);
        if (pathEnd != -1) {
            pathEnd;
            int lineStart = s.Find(_T("line: "), pathEnd);
            if (lineStart != -1) {
                lineStart += 6;
                int lineEnd = s.Find(_T(","), lineStart);
                if (lineEnd != -1) {
                    CString sub = s.Right(s.GetLength() - pathStart);
                    CString path = sub.Left(pathEnd - pathStart);
                    sub = s.Right(s.GetLength() - lineStart);
                    CString ln = sub.Left(lineEnd - lineStart);
					int beginPos = sub.Find(_T(":"));
					CString ps = sub.Right(sub.GetLength() - beginPos - 2);

                    WPARAM fileId = 0;
					ui32 line = atoi(_bstr_t(ln));
					ui32 pos = atoi(_bstr_t(ps));
                    CMainFrame* pMainFrame = DYNAMIC_DOWNCAST(CMainFrame, GetTopLevelFrame());
                    //pMainFrame->PostMessageW(WM_ON_SCRIPT_COMPILE_ERROR, fileId, line);
                    if (path.GetLength()) {
                        if (CScriptView *pCScriptView = pMainFrame->GetScriptView(path)) {
                            pCScriptView->HighlightLine(type, 0, line - 1, INVALID_THREAD_ID, nullptr, pos - 1);
                        }
                    }
                }
            }
        }
    }
}

void COutputList::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    // TODO: Add your message handler code here and/or call default
    OnLbnDblclk();
    CListBox::OnLButtonDblClk(nFlags, point);
}


afx_msg LRESULT COutputWnd::OnOnScriptRuntimeError(WPARAM wParam, LPARAM lParam) {
    m_wndTabs.SetActiveTab(wParam);
    return 0;
}

void COutputList::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{

    // TODO:  Add your code to draw the specified item
    CListBox::DrawItem(lpDrawItemStruct);
}
