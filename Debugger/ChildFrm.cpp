
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
    //m_bp.SetPoint(0, 0);
    //CPngImage img;
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
        //RECT rect;
        //GetWindowRect(&rect);
        //rect.top -= 20;
        //rect.bottom = rect.top + 40;
        //rect.right = rect.left + 200;
        //CRect r = { 40, -20, 300, 10 };
        //m_editCheckBox.Create(CString("Edit script"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, r, this, 0);
        //m_editCheckBox.EnableWindow();
        //m_editCheckBox
        CBitmap bmp;
        //if (!bmp.LoadBitmap(uiBmpId))
        //{
          //  TRACE(_T("Can't load bitmap: %x\n"), uiBmpId);
            //ASSERT(FALSE);
            //return -1;
       // }

        BITMAP bmpObj;
        //bmp.GetBitmap(&bmpObj);

        UINT nFlags = ILC_MASK;

        nFlags |= /*(theApp.m_bHiColorIcons) ? */ILC_COLOR24/* : ILC_COLOR4*/;

        //m_imageList.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
        //m_imageList.Add(&bmp, RGB(255, 0, 0));

        //m_wndGlobalVarList.SetImageList(&m_imageList, LVSIL_SMALL);
        //m_wndThreadList.SetImageList(&m_imageList, LVSIL_SMALL);
        //m_wndWatchList.SetImageList(&m_imageList, LVSIL_SMALL);

        UINT uiBmpId = /*theApp.m_bHiColorIcons ? IDB_BREAKPOINTS : */IDB_BITMAP5;
        //bmp.DeleteObject();
        bmp.LoadBitmap(uiBmpId);
        bmp.GetBitmap(&bmpObj);
        //m_imgSizeY = 15;//bmpObj.bmHeight;
        m_imageBpList.Create(m_imgSizeX, m_imgSizeY, nFlags, 0, 0);
        m_imageBpList.Add(&bmp, RGB(255, 0, 0));
       // m_brushBkg.CreateSolidBrush(0x00f0f0f0); // background color
        return TRUE;
    }
    return FALSE;
}


void CChildFrame::OnNcPaint()
{
    // TODO: Add your message handler code here
    // Do not call CMDIChildWndEx::OnNcPaint() for painting messages
    CMDIChildWndEx::OnNcPaint();
    //RECT rect;
    //GetWindowRect(&rect);
    //rect.top -= 20;
    //rect.bottom = rect.top + 30;
    //rect.right = rect.left + 200;
    ////m_editCheckBox.Create(CString("Edit script"), WS_VISIBLE | WS_CHILD, rect, this, 0);
    ////m_editCheckBox.EnableWindow();
    //m_editCheckBox.Invalidate();
}

#ifdef RICH_EDIT
int CChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    //EnableD2DSupport();

    //CHwndRenderTarget* pRenderTarget = GetRenderTarget();
    //ASSERT_VALID(pRenderTarget);

    //_LoadDemoImage(pRenderTarget);
   // m_pBP_enabled = new CD2DBitmap(pRenderTarget, // parent render target
   //     IDB_PNG_BP_ENABLED,                              // resource ID
   //     _T("PNG"));                               // resource type

    //HRESULT hr = m_pBP_enabled->Create(pRenderTarget);
    //if (FAILED(hr))
    //    return -1;

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
        //SCROLLINFO si = { 0 };
        //si.cbSize = sizeof(si);
        //int pos = pWnd->GetScrollInfo(SB_VERT, &si);//pScrollBar->GetScrollPos();
        int pos = pWnd->GetScrollPos(SB_VERT);//->GetScrollInfo(&si);
        //int mi = 0, ma = 0;
        //pWnd->GetScrollRange(SB_VERT, &mi, &ma);
        m_pos = pos;
        m_bp = point;
        m_bp.y += m_pos;
        //latest!////m_bp.y = (m_bp.y / m_imgSizeY) * m_imgSizeY;// +2;
        //BREAKPOINT_MARKER marker = BREAKPOINT_INVALID;
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
            
            variant_t val(START_POSITION_OFFSET + line);// m_bp.y / IMG_SIZE);
            //val.ulVal += START_POSITION_OFFSET;
            if (br.m_marker == BREAKPOINT_INVALID) {
                // InsertBreakPoint will update the marker if the script is running
                br.m_marker = pMainFrame->m_wndProperties.InsertBreakpoint(path, fileName, _bstr_t(val));
                //if (marker == BREAKPOINT_VALID) {
                    res.first->second.m_marker = br.m_marker;
                //}
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
    int nPos = line * m_imgSizeY;
    //RECT r;
    //GetClientRect(&r);
    //r.right = m_rectBorder.left;
    //InvalidateRect(&r);
    //SetScrollPos(SB_VERT, nPos);
    
    //Invalidate();
    //HandleVScroll(nPos);
    //Invalidate();
    //SetScrollInfo()
    // scroll document...
}
//LRESULT CChildFrame::OnAfxDraw2D(WPARAM wParam, LPARAM lParam)
//{
    //CHwndRenderTarget* pRenderTarget = reinterpret_cast<CHwndRenderTarget*>(lParam);

    //CD2DSizeF sizeTarget = pRenderTarget->GetSize();

    //pRenderTarget->FillRectangle(CD2DRectF(0, 0, sizeTarget.width = 20, sizeTarget.height), m_pB);

    //if (m_bp.y != 0) {
    //    //CHwndRenderTarget* pRenderTarget = GetRenderTarget();
    //    //CD2DSizeF sizeTarget = pRenderTarget->GetSize();
    //    //pRenderTarget->FillRectangle(CD2DRectF(0, 0, sizeTarget.width, sizeTarget.height), m_pBrushBackground);

    //    int row = (m_bp.y) / 20;
    //    float pos = row * 20;
    //    CD2DSizeF sizeBitmap = m_pBP_enabled->GetSize();
    //    for (int i = 0; i < 10; ++i) {
    //        pos = i * 20;
    //        pRenderTarget->DrawBitmap(m_pBP_enabled, D2D1::RectF(0, pos, 14, pos + 14));//sizeBitmap.width, sizeBitmap.height));

    //    }
    //}
    //CScriptView* pWnd = (CScriptView*)GetDlgItem(AFX_IDW_PANE_FIRST);
    //pWnd->Invalidate();
    //pWnd->SetRedraw();
    //return static_cast<LRESULT>(TRUE);
//}
void CChildFrame::OnPaint()
{
    CPaintDC dc(this);
    RECT rect = { 0 };

    GetClientRect(&rect);
    --rect.top; 
    ++rect.bottom;
    //ScreenToClient(&rect);
    //dc.SetBkColor(0x00cccccc);
    //CBrush brush;
    //brushBg.CreateSolidBrush(0x00)
    //CPen pen;
    //pen.CreatePen(PS_SOLID, 1, RGB(230, 230, 230));
    //CPen* pOldPen = dc.SelectObject(&pen);
    CBrush* pOrigBrush = (CBrush*)dc.SelectObject(&m_brushBkg);

    dc.Rectangle(&rect);
    //CBrush brushRed;
    //brushRed.CreateSolidBrush(RGB(240, 0, 0));
    //dc.SelectObject(&brushRed);
    CScriptView *pWnd = (CScriptView *)GetDlgItem(AFX_IDW_PANE_FIRST);
    //int mi = 0, ma = 0;
    //pWnd->GetScrollRange(SB_VERT, &mi, &ma);

    SIZE sz;
    sz.cy = m_imgSizeY;
	sz.cx = m_imgSizeX;

    POINT po;
    int img = -1; // ?

    po.x = IMG_X_OFFSET;// 3;
    for (map<int, BREAKPOINT>::iterator it = m_breakpoints.begin(); it != m_breakpoints.end(); ++it) {
        po.y = (it->first * m_imgSizeY) - m_pos + IMG_Y_OFFSET;//it->second.m_y - m_pos + 1;// -1;
        if (po.y + m_imgSizeY <= 0) continue;
        if (po.y >= rect.bottom) break;
        //m_imageBpList.SetOverlayImage(img, 1);
        //m_imageBpList.DrawIndirect(&dc, ii, po, sz, CPoint(0, 0), INDEXTOOVERLAYMASK(1));
        
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
            //po.x = IMG_X_OFFSET;// 6;
            m_imageBpList.DrawIndirect(&dc, img, po, sz, CPoint(0, 0));
        }
    }
    dc.SelectObject(pOrigBrush);
    //dc.SelectObject(pOldPen);
}

//void CChildFrame::HandleVScroll(UINT nPos) {
//
//    SetPos(nPos);
//    RECT r;
//    GetClientRect(&r);
//    r.right = m_rectBorder.left;
//    InvalidateRect(&r);//RedrawWindow(&r, 0, RDW_NOCHILDREN | RDW_ERASE | RDW_INTERNALPAINT | RDW_INVALIDATE | RDW_UPDATENOW);
//}

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
