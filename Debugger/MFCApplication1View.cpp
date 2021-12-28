
// MFCApplication1View.cpp : implementation of the CMFCApplication1View class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "MFCApplication1.h"
#endif

#include "MFCApplication1Doc.h"
#include "MFCApplication1View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCApplication1View

IMPLEMENT_DYNCREATE(CMFCApplication1View, CView)

BEGIN_MESSAGE_MAP(CMFCApplication1View, CView)
    ON_COMMAND(ID_DEBUG_RUN, &CMFCApplication1View::OnDebugRun)
    ON_COMMAND(ID_DEBUG_STOP, &CMFCApplication1View::OnDebugStop)
    ON_COMMAND(ID_DEBUG_PAUSE, &CMFCApplication1View::OnDebugPause)
    ON_COMMAND(ID_DEBUG_STEPIN, &CMFCApplication1View::OnDebugStepin)
    ON_COMMAND(ID_DEBUG_STEPOVER, &CMFCApplication1View::OnDebugStepover)
    ON_COMMAND(ID_DEBUG_STEPOUT, &CMFCApplication1View::OnDebugStepout)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMFCApplication1View::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
    ON_WM_CREATE()
END_MESSAGE_MAP()

// CMFCApplication1View construction/destruction

CMFCApplication1View::CMFCApplication1View(): m_htmlDlg(IDD_DIALOG1)
{
	// TODO: add construction code here

}

CMFCApplication1View::~CMFCApplication1View()
{
}

BOOL CMFCApplication1View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CMFCApplication1View drawing

void CMFCApplication1View::OnDraw(CDC* /*pDC*/)
{
	CMFCApplication1Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CMFCApplication1View printing


void CMFCApplication1View::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CMFCApplication1View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CMFCApplication1View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CMFCApplication1View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CMFCApplication1View::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CMFCApplication1View::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CMFCApplication1View diagnostics

#ifdef _DEBUG
void CMFCApplication1View::AssertValid() const
{
	CView::AssertValid();
}

void CMFCApplication1View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFCApplication1Doc* CMFCApplication1View::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCApplication1Doc)));
	return (CMFCApplication1Doc*)m_pDocument;
}
#endif //_DEBUG


// CMFCApplication1View message handlers


BOOL CMFCApplication1View::OnCreateAggregates()
{
    // TODO: Add your specialized code here and/or call the base class
    //m_htmlDlg.Create(IDD_DIALOG1, this);// .ShowWindow(1);
    return TRUE;
}


int CMFCApplication1View::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  Add your specialized creation code here
    OnCreateAggregates();
    return 0;
}

void CMFCApplication1View::OnDebugRun() {
    // TODO: Add your command handler code here
    //m_pRunner->Exec();
    /*if (!m_ThreadScriptRunner.GetThreadId()) {
    CFrameWnd *pFrame = GetParentFrame();

    CMainFrame *pMainFrame = (CMainFrame*)pFrame->GetParentFrame();
    m_ThreadScriptRunner.SetMainFrame(pMainFrame);
    m_ThreadScriptRunner.Start(m_ScriptFileName);
    }
    Debugger::GetInstance().Run();*/
}


void CMFCApplication1View::OnDebugStop() {
    // TODO: Add your command handler code here
    /*Debugger::GetInstance().Stop();
    m_ThreadScriptRunner.Stop();*/
}


void CMFCApplication1View::OnDebugPause() {
    // TODO: Add your command handler code here
    Debugger *pDebugger = theApp.GetDebugger();
    if (pDebugger) {
        pDebugger->Pause(PauseType(PAUSE_TYPE::PAUSE_ALL));
    }
}


void CMFCApplication1View::OnDebugStepin() {
    // TODO: Add your command handler code here
    Debugger *pDebugger = theApp.GetDebugger();
    if (pDebugger) {
        pDebugger->StepIn();
    }
}


void CMFCApplication1View::OnDebugStepover() {
    // TODO: Add your command handler code here
    Debugger *pDebugger = theApp.GetDebugger();
    if (pDebugger) {
        pDebugger->StepOver();
    }
}


void CMFCApplication1View::OnDebugStepout() {
    // TODO: Add your command handler code here
    Debugger *pDebugger = theApp.GetDebugger();
    if (pDebugger) {
        pDebugger->StepOut();
    }
}


BOOL CMFCApplication1View::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
    // TODO: Add your specialized code here and/or call the base class

    return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
