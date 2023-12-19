
// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "ScriptView.h"

#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
    ON_WM_NCPAINT()
    ON_WM_PAINT()
    ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// TODO: add member initialization code here
    m_highlightType = HIGHLIGHT_OFF;
    m_highlightLine = 0;
    m_rectBorder.left = 25;
    m_imgSizeY = 15;
	m_imgSizeX = 20;
    m_pos = 0;
}

CChildFrame::~CChildFrame()
{
}


BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs
	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWndEx::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWndEx::Dump(dc);
}
#endif //_DEBUG

// CChildFrame message handlers


BOOL CChildFrame::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CMDIFrameWnd* pParentWnd, CCreateContext* pContext)
{
    // TODO: Add your specialized code here and/or call the base class

    if (CMDIChildWndEx::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, pContext)) {
        CBitmap bmp;
        BITMAP bmpObj;

        UINT nFlags = ILC_MASK;

        nFlags |= /*(theApp.m_bHiColorIcons) ? */ILC_COLOR24/* : ILC_COLOR4*/;

        UINT uiBmpId = /*theApp.m_bHiColorIcons ? IDB_BREAKPOINTS : */IDB_MARKERS;

        bmp.LoadBitmap(uiBmpId);
        bmp.GetBitmap(&bmpObj);
        m_imageBpList.Create(m_imgSizeX, m_imgSizeY, nFlags, 0, 0);
        m_imageBpList.Add(&bmp, RGB(255, 0, 0));
        return TRUE;
    }
    return FALSE;
}


void CChildFrame::OnNcPaint()
{
    // TODO: Add your message handler code here
    // Do not call CMDIChildWndEx::OnNcPaint() for painting messages
    CMDIChildWndEx::OnNcPaint();
}

#ifdef RICH_EDIT
int CChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;
    return 0;
}

void CChildFrame::OnLButtonUp(UINT nFlags, CPoint point)
{
    // TODO: Add your message handler code here and/or call default
    CMDIChildWndEx::OnLButtonUp(nFlags, point);
    CScriptView *pWnd = (CScriptView *)GetDlgItem(AFX_IDW_PANE_FIRST);
    _bstr_t fileName, path;
    pWnd->GetPathAndName(fileName, path);
    if (fileName.length() == 0) {
        this->MessageBox(_T("Must save this file!"), _T("Error!"), MB_ICONERROR | MB_OK);
        return;
    }

    SCROLLBARINFO sbi = { 0 };
    sbi.cbSize = sizeof(sbi);
    if (pWnd->GetScrollBarInfo(OBJID_VSCROLL, &sbi)) {
        int pos = pWnd->GetScrollPos(SB_VERT);
        m_pos = pos;
        m_bp = point;
        m_bp.y += m_pos;
        int line = m_bp.y / m_imgSizeY;
        BREAKPOINT br;
        br.m_marker = BREAKPOINT_INVALID;
        br.m_y = m_bp.y;
        pair<map<int, BREAKPOINT>::iterator, bool> res = m_breakpoints.insert(pair<int, BREAKPOINT>(line, br));
        if (!res.second) {
            m_breakpoints.erase(res.first); // remove breakpoint marker
            br.m_marker = BREAKPOINT_NONE;
        }
        {// set breakpoint type and delegate it
            CMainFrame* pMainFrame = (CMainFrame*)theApp.GetMainWnd();
            
            variant_t val(START_POSITION_OFFSET + line);
            if (br.m_marker == BREAKPOINT_INVALID) {
                // InsertBreakPoint will update the marker if the script is running
                br.m_marker = pMainFrame->m_wndProperties.InsertBreakpoint(path, fileName, _bstr_t(val));
                res.first->second.m_marker = br.m_marker;
            }
            else {
                pMainFrame->m_wndProperties.DeleteBreakpoint(path, fileName, _bstr_t(val));
            }
        }
        Invalidate();
    }
}

void CChildFrame::SetBreakpointMarker(int line, BREAKPOINT_MARKER marker) {
    BREAKPOINT br;
    br.m_marker = marker; 
    br.m_y = line * m_imgSizeY;
    if (marker == BREAKPOINT_NONE) {
        m_breakpoints.erase(line);
    }
    else {
        pair<map<int, BREAKPOINT>::iterator, bool> res = m_breakpoints.insert(pair<int, BREAKPOINT>(line, br));
        if (!res.second) {
            res.first->second.m_marker = marker;
        }
    }
    Invalidate();
}

void CChildFrame::SetHighlight(int line, HIGHLIGHT_TYPE type) {
    m_highlightLine = line;
    m_highlightType = type;
}

void CChildFrame::OnPaint()
{
    CPaintDC dc(this);
    RECT rect = { 0 };

    GetClientRect(&rect);
    --rect.top; 
    ++rect.bottom;
    CBrush* pOrigBrush = (CBrush*)dc.SelectObject(&m_brushBkg);

    dc.Rectangle(&rect);
    CScriptView *pWnd = (CScriptView *)GetDlgItem(AFX_IDW_PANE_FIRST);

    SIZE sz;
    sz.cy = m_imgSizeY;
	sz.cx = m_imgSizeX;

    POINT po;
    int img = -1; // ?

    po.x = IMG_X_OFFSET;// 3;
    for (map<int, BREAKPOINT>::iterator it = m_breakpoints.begin(); it != m_breakpoints.end(); ++it) {
        po.y = (it->first * m_imgSizeY) - m_pos + IMG_Y_OFFSET;
        if (po.y + m_imgSizeY <= 0) continue;
        if (po.y >= rect.bottom) break;
        
        img = (it->second.m_marker == BREAKPOINT_INVALID) ? 0 : 1;
        m_imageBpList.DrawIndirect(&dc, img, po, sz, CPoint(0,1));
    }
    img = -1;
    if (m_highlightType != HIGHLIGHT_OFF) {
        switch (m_highlightType) {
            case HIGHLIGHT_STACK_CALL:
            case HIGHLIGHT_ON_BREAK:
                img = 2;
                break;
            case HIGHLIGHT_COMPILE_ERROR:
                img = 3;
                break;
            case HIGHLIGHT_RUNTIME_ERROR:
                img = 4;
                break;
            case HIGHLIGHT_BREAKPOINT:
                img = 5;
                break;
            case HIGHLIGHT_FUNC_DEF:
                img = 6;
                break;
            default:
                break;
        }
        if (img != -1) {
            po.y = (m_highlightLine * m_imgSizeY) - m_pos + IMG_Y_OFFSET;
            m_imageBpList.DrawIndirect(&dc, img, po, sz, CPoint(0, 0));
        }
    }
    dc.SelectObject(pOrigBrush);
}

void CChildFrame::SetPos(int pos) {
    m_pos = pos;
}
void CChildFrame::Invalidate() {
    RECT r;
    GetClientRect(&r);
    r.right = m_rectBorder.left;
    InvalidateRect(&r);
}
#endif // RICH_EDIT
