
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "ScriptView.h"
#include "MainFrm.h"
#include "../Script/Debugger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace script;
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
	ON_COMMAND(ID_VIEW_FILEVIEW, &CMainFrame::OnViewFileView)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FILEVIEW, &CMainFrame::OnUpdateViewFileView)
	ON_COMMAND(ID_VIEW_CLASSVIEW, &CMainFrame::OnViewClassView)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CLASSVIEW, &CMainFrame::OnUpdateViewClassView)
	ON_COMMAND(ID_VIEW_OUTPUTWND, &CMainFrame::OnViewOutputWindow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUTWND, &CMainFrame::OnUpdateViewOutputWindow)
	ON_COMMAND(ID_VIEW_PROPERTIESWND, &CMainFrame::OnViewPropertiesWindow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PROPERTIESWND, &CMainFrame::OnUpdateViewPropertiesWindow)
	ON_WM_SETTINGCHANGE()
    ON_WM_DESTROY()
    ON_MESSAGE(WM_ON_DEBUGGER_STOP, &CMainFrame::OnDebuggerStop)
    ON_MESSAGE(WM_ON_DEBUGGER_RUN, &CMainFrame::OnDebuggerRun)
    ON_MESSAGE(WM_ON_DEBUGGER_PAUSE, &CMainFrame::OnDebuggerPause)
    ON_MESSAGE(WM_ON_DEBUGGER_STEPIN, &CMainFrame::OnDebuggerStepIn)
    ON_MESSAGE(WM_ON_DEBUGGER_STEPOVER, &CMainFrame::OnDebuggerStepOver)
    ON_MESSAGE(WM_ON_DEBUGGER_STEPOUT, &CMainFrame::OnDebuggerStepOut)
    ON_MESSAGE(WM_ON_DEBUGGER_SET_BREAKPOINT, &CMainFrame::OnDebuggerSetBreakPoint)
    ON_MESSAGE(WM_ON_DEBUGGER_RESET_BREAKPOINT, &CMainFrame::OnDebuggerResetBreakPoint)
    ON_MESSAGE(WM_ON_DEBUGGER_BREAK_HIT, &CMainFrame::OnDebuggerBreakHit)
    ON_MESSAGE(WM_ON_SET_STARTUP_SCRIPT, &CMainFrame::OnSetStartupScript)
    ON_MESSAGE(WM_ON_CLEAR_STARTUP_SCRIPT, &CMainFrame::OnClearStartupScript)
    //ON_MESSAGE(WM_ON_SCRIPT_COMPILE_ERROR, &CMainFrame::OnScriptCompileError)
    //ON_MESSAGE(WM_ON_END_DEBUG_THREAD, &CMainFrame::OnOnEndDebugThread)
    ON_MESSAGE(WM_ON_BEGIN_THREAD, &CMainFrame::OnBeginThread)
    ON_MESSAGE(WM_ON_END_THREAD, &CMainFrame::OnEndThread)
    ON_MESSAGE(WM_ON_END_SCRIPT_THREAD, &CMainFrame::OnEndScriptThread)
    
    ON_COMMAND(ID_DEBUG_RUN, &CMainFrame::OnCmdDebugRun)
    ON_COMMAND(ID_DEBUG_STOP, &CMainFrame::OnCmdDebugStop)
    ON_COMMAND(ID_DEBUG_PAUSE, &CMainFrame::OnCmdDebugPause)
    ON_COMMAND(ID_DEBUG_STEPIN, &CMainFrame::OnCmdDebugStepin)
    ON_COMMAND(ID_DEBUG_STEPOVER, &CMainFrame::OnCmdDebugStepover)
    ON_COMMAND(ID_DEBUG_STEPOUT, &CMainFrame::OnCmdDebugStepout)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame() : m_DebuggerEventsImpl(this), m_debuggerState(DEBUGGER_STOP), m_pRunner(0)/*, m_pSymbolStore(0)*/
{
	// TODO: add member initialization code here
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2008);
}

CMainFrame::~CMainFrame()
{

}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

	// Allow user-defined toolbars operations:
	InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);


	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Load menu item image (not placed on any standard toolbars):
	CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, theApp.m_bHiColorIcons ? IDB_MENU_IMAGES_24 : 0);

	// create docking windows
	if (!CreateDockingWindows())
	{
		TRACE0("Failed to create docking windows\n");
		return -1;
	}

	m_wndFileTree.EnableDocking(CBRS_ALIGN_ANY);
	m_wndClassView.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndFileTree);
	CDockablePane* pTabbedBar = NULL;
	m_wndClassView.AttachToTabWnd(&m_wndFileTree, DM_SHOW, TRUE, &pTabbedBar);
	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndOutput);
	m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndProperties);

	// set the visual manager and style based on persisted value
	OnApplicationLook(theApp.m_nAppLook);

	// Enable enhanced windows management dialog
	EnableWindowsDialog(ID_WINDOW_MANAGER, ID_WINDOW_MANAGER, TRUE);

	// Enable toolbar and docking window menu replacement
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

	// enable quick (Alt+drag) toolbar customization
	CMFCToolBar::EnableQuickCustomization();

	if (CMFCToolBar::GetUserImages() == NULL)
	{
		// load user-defined toolbar images
		if (m_UserImages.Load(_T(".\\UserImages.bmp")))
		{
			CMFCToolBar::SetUserImages(&m_UserImages);
		}
	}

	// enable menu personalization (most-recently used commands)
	// TODO: define your own basic commands, ensuring that each pulldown menu has at least one basic command.
	CList<UINT, UINT> lstBasicCommands;

	lstBasicCommands.AddTail(ID_FILE_NEW);
	lstBasicCommands.AddTail(ID_FILE_OPEN);
	lstBasicCommands.AddTail(ID_FILE_SAVE);
	lstBasicCommands.AddTail(ID_FILE_PRINT);
	lstBasicCommands.AddTail(ID_APP_EXIT);
	lstBasicCommands.AddTail(ID_EDIT_CUT);
	lstBasicCommands.AddTail(ID_EDIT_PASTE);
	lstBasicCommands.AddTail(ID_EDIT_UNDO);
    lstBasicCommands.AddTail(ID_DEBUG_RUN);
    lstBasicCommands.AddTail(ID_DEBUG_STOP);
    lstBasicCommands.AddTail(ID_DEBUG_PAUSE);
    lstBasicCommands.AddTail(ID_DEBUG_STEPIN);
    lstBasicCommands.AddTail(ID_DEBUG_STEPOVER);
    lstBasicCommands.AddTail(ID_DEBUG_STEPIN);
    lstBasicCommands.AddTail(ID_APP_ABOUT);
	lstBasicCommands.AddTail(ID_VIEW_STATUS_BAR);
	lstBasicCommands.AddTail(ID_VIEW_TOOLBAR);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2003);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_VS_2005);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLUE);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_SILVER);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLACK);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_AQUA);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_WINDOWS_7);
	lstBasicCommands.AddTail(ID_SORTING_SORTALPHABETIC);
	lstBasicCommands.AddTail(ID_SORTING_SORTBYTYPE);
	lstBasicCommands.AddTail(ID_SORTING_SORTBYACCESS);
	lstBasicCommands.AddTail(ID_SORTING_GROUPBYTYPE);

	CMFCToolBar::SetBasicCommands(lstBasicCommands);

    //Debugger::GetInstance().SetDebuggerNotify(&m_DebuggerEventsImpl);

    return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

BOOL CMainFrame::CreateDockingWindows()
{
	BOOL bNameValid;

	// Create class view
	CString strClassView;
	bNameValid = strClassView.LoadString(IDS_CLASS_VIEW);
	ASSERT(bNameValid);
	if (!m_wndClassView.Create(strClassView, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_CLASSVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Class View window\n");
		return FALSE; // failed to create
	}

	// Create file view
	CString strFileView;
	bNameValid = strFileView.LoadString(IDS_FILE_VIEW);
	ASSERT(bNameValid);
	if (!m_wndFileTree.Create(strFileView, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_FILEVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT| CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create File View window\n");
		return FALSE; // failed to create
	}

	// Create output window
	CString strOutputWnd;
	bNameValid = strOutputWnd.LoadString(IDS_OUTPUT_WND);
	ASSERT(bNameValid);
	if (!m_wndOutput.Create(strOutputWnd, this, CRect(0, 0, 100, 100), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Output window\n");
		return FALSE; // failed to create
	}

	// Create properties window
	CString strPropertiesWnd;
    bNameValid = strPropertiesWnd.LoadString(IDS_BREAKPOINTS);// default tab! //IDS_PROPERTIES_WND);
	ASSERT(bNameValid);
	if (!m_wndProperties.Create(strPropertiesWnd, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_PROPERTIESWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Properties window\n");
		return FALSE; // failed to create
	}

	SetDockingWindowIcons(theApp.m_bHiColorIcons);
	return TRUE;
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	HICON hFileViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_FILE_VIEW_HC : IDI_FILE_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndFileTree.SetIcon(hFileViewIcon, FALSE);

	HICON hClassViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_CLASS_VIEW_HC : IDI_CLASS_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndClassView.SetIcon(hClassViewIcon, FALSE);

	HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndOutput.SetIcon(hOutputBarIcon, FALSE);

	HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndProperties.SetIcon(hPropertiesBarIcon, FALSE);

}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnWindowManager()
{
	ShowWindowsDialog();
}

void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CMDIFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	m_wndOutput.UpdateFonts();
	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

void CMainFrame::OnViewFileView()
{
	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	m_wndFileTree.ShowPane(TRUE, FALSE, TRUE);
	m_wndFileTree.SetFocus();
}

void CMainFrame::OnUpdateViewFileView(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CMainFrame::OnViewClassView()
{
	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	m_wndClassView.ShowPane(TRUE, FALSE, TRUE);
	m_wndClassView.SetFocus();
}

void CMainFrame::OnUpdateViewClassView(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CMainFrame::OnViewOutputWindow()
{
	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	m_wndOutput.ShowPane(TRUE, FALSE, TRUE);
	m_wndOutput.SetFocus();
}

void CMainFrame::OnUpdateViewOutputWindow(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CMainFrame::OnViewPropertiesWindow()
{
	// Show or activate the pane, depending on current state.  The
	// pane can only be closed via the [x] button on the pane frame.
	m_wndProperties.ShowPane(TRUE, FALSE, TRUE);
	m_wndProperties.SetFocus();
}

void CMainFrame::OnUpdateViewPropertiesWindow(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}


BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	// base class does the real work

	if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}


	// enable customization button for all user toolbars
	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	for (int i = 0; i < iMaxUserToolbars; i ++)
	{
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar != NULL)
		{
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
		}
	}

	return TRUE;
}


void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CMDIFrameWndEx::OnSettingChange(uFlags, lpszSection);
	m_wndOutput.UpdateFonts();
}


void CMainFrame::OnDestroy()
{
    CMDIFrameWndEx::OnDestroy();

    // TODO: Add your message handler code here
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
    // TODO: Add your specialized code here and/or call the base class
    BOOL enabled = true;
    switch (nID) {
        /*case ID_CLEAR_STARTUP_SCRIPT:
            enabled = false;
            break;
        case ID_SET_STARTUP_SCRIPT:
            enabled = true;
            break;*/
        case ID_DEBUG_RUN:
        case ID_DEBUG_STEPIN:
        case ID_DEBUG_STEPOVER:
            switch (m_debuggerState) {
                case DEBUGGER_RUN:
                case DEBUGGER_STEP_IN:
                case DEBUGGER_STEP:
                case DEBUGGER_STEP_OUT:
                    enabled = false;
                    break;
                default:
                    enabled = IsStartUpScriptSet();
                    break;
            }
            break;
        case ID_DEBUG_STOP:
            switch (m_debuggerState) {
                case DEBUGGER_STOP:
                    enabled = false;
                    break;
                default:
                    enabled = IsStartUpScriptSet();
                    break;
            }
            break;
        case ID_DEBUG_PAUSE:
            switch (m_debuggerState) {
                case DEBUGGER_STOP:
                    enabled = false;
                    break;
                case DEBUGGER_PAUSE:
                    enabled = false;
                    break;
                default:
                    enabled = IsStartUpScriptSet();
                    break;
            }
            break;
        case ID_DEBUG_STEPOUT:
            switch (m_debuggerState) {
                case DEBUGGER_STOP:
                    enabled = false;
                    break;
                case DEBUGGER_RUN:
                case DEBUGGER_STEP_IN:
                case DEBUGGER_STEP:
                case DEBUGGER_STEP_OUT:
                    enabled = false;
                    break;
                default:
                    enabled = IsStartUpScriptSet();
                    break;
            }
            break;
        default:
            break;
    }

    return enabled ? CMDIFrameWndEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo) : false;
}

CScriptView* CMainFrame::GetScriptView(ui32 fileId) {
    string fileName;
    if (SymbolStore* pSymbolStore = theApp.GetSymbolStore()) {
        pSymbolStore->GetFileName(fileId, fileName);
        if (!fileName.empty()) {
            _bstr_t path(fileName.c_str());
            return GetScriptView((LPCTSTR)path);
        }
    }
    return 0;
}

CScriptView* CMainFrame::GetScriptView(LPCTSTR filePath) {
    CString path(filePath);
    int dot = path.ReverseFind('.');
    if (dot != -1) {
        CString right = path.Right(path.GetLength() - dot);
        if ((right == EXT_TEXT_SCRIPT_STRING) || (right == EXT_BIN_SCRIPT_STRING)) {
            if (CDocument *pDocument = theApp.OpenDocumentFile(filePath)) {
                POSITION pos = pDocument->GetFirstViewPosition();
                return static_cast<CScriptView*>(pDocument->GetNextView(pos));
            }
        }
    }
    return 0;
}

afx_msg LRESULT CMainFrame::OnDebuggerStop(WPARAM wParam, LPARAM lParam) {
    m_debuggerState = DEBUGGER_STOP;
    m_wndProperties.DisableTab(TRUE, wParam);
    EnableEditing(true);
    m_wndClassView.ClearClassView();
    OnViewFileView();
    return 0;
}

void CMainFrame::EnableEditing(bool enable) {
    POSITION pos = theApp.m_pDocManager->GetFirstDocTemplatePosition();
    if (CDocTemplate *pDocTemplate = theApp.m_pDocManager->GetNextDocTemplate(pos)) {
        CDocument* pOpenDocument = NULL;
        POSITION docPos = pDocTemplate->GetFirstDocPosition();
        while (docPos) {
            pOpenDocument = pDocTemplate->GetNextDoc(docPos);
            CString path = pOpenDocument->GetPathName();
            POSITION pos = pOpenDocument->GetFirstViewPosition();
            CScriptView* pScriptView = static_cast<CScriptView*>(pOpenDocument->GetNextView(pos));
            pScriptView->EnableEditing(enable);
            if (enable) {
                pScriptView->ClearHighlight(true);
            }
        }
    }
}

afx_msg LRESULT CMainFrame::OnDebuggerRun(WPARAM wParam, LPARAM lParam) {
    if (DEBUGGER_STOP == m_debuggerState) {
        EnableEditing(false);
    }
    m_debuggerState = DEBUGGER_RUN;
    m_wndProperties.DisableTab(TRUE, wParam);
    OnViewClassView();
    return 0;
}

afx_msg LRESULT CMainFrame::OnDebuggerPause(WPARAM wParam, LPARAM lParam) {
    m_debuggerState = DEBUGGER_PAUSE;
    //m_wndProperties.DisableTab(FALSE, wParam);
    return 0;
}

afx_msg LRESULT CMainFrame::OnDebuggerStepIn(WPARAM wParam, LPARAM lParam) {
    if (DEBUGGER_STOP == m_debuggerState) {
        EnableEditing(false);
    }
    m_debuggerState = DEBUGGER_STEP_IN;
    m_wndProperties.DisableTab(TRUE, wParam);
    return 0;
}

afx_msg LRESULT CMainFrame::OnDebuggerStepOver(WPARAM wParam, LPARAM lParam) {
    if (DEBUGGER_STOP == m_debuggerState) {
        EnableEditing(false);
    }
    m_debuggerState = DEBUGGER_STEP;
    m_wndProperties.DisableTab(TRUE, wParam);
    return 0;
}

afx_msg LRESULT CMainFrame::OnDebuggerStepOut(WPARAM wParam, LPARAM lParam) {
    if (DEBUGGER_STOP == m_debuggerState) {
        EnableEditing(false);
    }
    m_debuggerState = DEBUGGER_STEP_OUT;
    m_wndProperties.DisableTab(TRUE, wParam);
    return 0;
}

afx_msg LRESULT CMainFrame::OnDebuggerSetBreakPoint(WPARAM wParam, LPARAM lParam) {
    if (CScriptView *pScriptView = GetScriptView(wParam)) {
        pScriptView->SetBreakPointMarker(lParam, BREAKPOINT_VALID);
    }
    return 0;
}

afx_msg LRESULT CMainFrame::OnDebuggerResetBreakPoint(WPARAM wParam, LPARAM lParam) {
    if (CScriptView *pScriptView = GetScriptView(wParam)) {
        pScriptView->SetBreakPointMarker(lParam, BREAKPOINT_NONE);
    }
    return 0;
}

afx_msg LRESULT CMainFrame::OnDebuggerBreakHit(WPARAM wParam, LPARAM lParam) {
    BREAK_HIT_DATA *pData = (BREAK_HIT_DATA*)wParam;
    m_debuggerState = DEBUGGER_PAUSE;
    m_wndProperties.DisableTab(FALSE, pData->threadId);
    if (CScriptView *pScriptView = GetScriptView(pData->fileId)) {
        pScriptView->HighlightLine(HIGHLIGHT_ON_BREAK, pData->fileId, pData->line, pData->threadId, pData->pScope);
    }
    return 0;
}


afx_msg LRESULT CMainFrame::OnSetStartupScript(WPARAM wParam, LPARAM lParam) {
    //theApp.ReadScriptAsHTML();
    return 0;
}


afx_msg LRESULT CMainFrame::OnClearStartupScript(WPARAM wParam, LPARAM lParam) {
    //theApp.UnLoadScript();
    return 0;
}

//afx_msg LRESULT CMainFrame::OnScriptCompileError(WPARAM wParam, LPARAM lParam) {
//    if (CScriptView *pScriptView = GetScriptView(wParam)) {
//        pScriptView->HighlightLine(HIGHLIGHT_COMPILE_ERROR, wParam, lParam, INVALID_THREAD_ID, 0);
//    }
//    return 0;
//}

void CMainFrame::PropagateBreakpoints() {
    m_wndProperties.PropagateBreakpoints();
    m_wndClassView.FillClassView();
}

afx_msg LRESULT CMainFrame::OnBeginThread(WPARAM wParam, LPARAM lParam) {
    //m_wndProperties.ClearStackList();
    m_wndProperties.DisableTab(TRUE, wParam);
    m_wndProperties.OnBeginThread(wParam);
    return 0;
}

afx_msg LRESULT CMainFrame::OnEndThread(WPARAM wParam, LPARAM lParam) {
    //m_wndProperties.ClearStackList();
    m_wndProperties.DisableTab(FALSE, wParam);
    m_wndProperties.OnEndThread(wParam);
    return 0;
}

afx_msg LRESULT CMainFrame::OnEndScriptThread(WPARAM wParam, LPARAM lParam) {
    m_wndProperties.ClearGlobalVarList();
    theApp.OnDebugDumpMemory();
    theApp.UnLoadScript();
    //TODO("Must release Runner in the MFCApp thread!");
    theApp.ReleaseScriptThreadRunner();
   // m_pRunner = smart_ptr<ScriptRunner>(0);
    return 0;
}

afx_msg void CMainFrame::OnCmdDebugRun() {
    theApp.OnCmdDebugRun();
}

afx_msg void CMainFrame::OnCmdDebugStop() {
    theApp.OnCmdDebugStop();
}

afx_msg void CMainFrame::OnCmdDebugPause() {
    theApp.OnCmdDebugPause();
}

afx_msg void CMainFrame::OnCmdDebugStepin() {
    theApp.OnCmdDebugStepin();
}

afx_msg void CMainFrame::OnCmdDebugStepover() {
    theApp.OnCmdDebugStepover();
}

afx_msg void CMainFrame::OnCmdDebugStepout() {
    theApp.OnCmdDebugStepout();
}
