
#include "stdafx.h"
#include "MainFrm.h"
#include "ClassView.h"
#include "Resource.h"
#include "MFCApplication1.h"

class CClassViewMenuButton : public CMFCToolBarMenuButton
{
	friend class CClassView;

	DECLARE_SERIAL(CClassViewMenuButton)

public:
	CClassViewMenuButton(HMENU hMenu = NULL) : CMFCToolBarMenuButton((UINT)-1, hMenu, -1)
	{
	}

	virtual void OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz = TRUE,
		BOOL bCustomizeMode = FALSE, BOOL bHighlight = FALSE, BOOL bDrawBorder = TRUE, BOOL bGrayDisabledButtons = TRUE)
	{
		pImages = CMFCToolBar::GetImages();

		CAfxDrawState ds;
		pImages->PrepareDrawImage(ds);

		CMFCToolBarMenuButton::OnDraw(pDC, rect, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);

		pImages->EndDrawImage(ds);
	}
};

IMPLEMENT_SERIAL(CClassViewMenuButton, CMFCToolBarMenuButton, 1)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFunctionDefTree::CFunctionDefTree()
{

}

CFunctionDefTree::~CFunctionDefTree()
{
}

BEGIN_MESSAGE_MAP(CFunctionDefTree, CTreeCtrl)
    ON_WM_LBUTTONDBLCLK()
    ON_WM_CREATE()
    //ON_COMMAND(ID_SET_STARTUP_SCRIPT, &CFileTree::OnSetStartupScript)
    //ON_COMMAND(ID_COMPILE, &CFileTree::OnCompile)
    //ON_COMMAND(ID_DECOMPILE, &CFileTree::OnDecompile)
    //ON_NOTIFY_REFLECT(NM_CLICK, &CFunctionDefTree::OnNMClick)
   // ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSymbolTree message handlers

int CFunctionDefTree::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CTreeCtrl::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  Add your specialized creation code here
    //LoadTree();
    return 0;
}

BOOL CFunctionDefTree::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    BOOL bRes = CTreeCtrl::OnNotify(wParam, lParam, pResult);

    NMHDR* pNMHDR = (NMHDR*)lParam;
    ASSERT(pNMHDR != NULL);

    if (pNMHDR && pNMHDR->code == TTN_SHOW && GetToolTips() != NULL)
    {
        GetToolTips()->SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
    }

    return bRes;
}


void CFunctionDefTree::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    CTreeCtrl::OnLButtonDblClk(nFlags, point);
    // TODO: Add your message handler code here and/or call default
    //OpenFile();
    HTREEITEM hItem = GetSelectedItem();
    //if (SymbolStore *pSymbolStore = theApp.GetSymbolStore()) {
        if (Node *pNode = (Node*)GetItemData(hItem)) {
            SYMBOL_DESC symDesc;
            pNode->GetSymbolDesc(symDesc);
            CMainFrame *pMainFrame = (CMainFrame*)theApp.m_pMainWnd;
            if (CScriptView *pScriptView = pMainFrame->GetScriptView(symDesc.m_fileId)) {
                //ui32 line = stoul(string(_bstr_t(strLine)));
                pScriptView->HighlightLine(HIGHLIGHT_FUNC_DEF, -1, symDesc.m_line - START_POSITION_OFFSET, INVALID_THREAD_ID, 0);
                return;
            }
        }
        POSITION pos = theApp.m_pDocManager->GetFirstDocTemplatePosition();
        if (CDocTemplate *pDocTemplate = theApp.m_pDocManager->GetNextDocTemplate(pos)) {
            CDocument* pOpenDocument = NULL;
            POSITION docPos = pDocTemplate->GetFirstDocPosition();
            while (docPos) {
                pOpenDocument = pDocTemplate->GetNextDoc(docPos);
                CString path = pOpenDocument->GetPathName();
                POSITION pos = pOpenDocument->GetFirstViewPosition();
                static_cast<CScriptView*>(pOpenDocument->GetNextView(pos))->ClearHighlight(false);
            }
        }

    //}
}

//void CFunctionDefTree::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
//{
//    // TODO: Add your control notification handler code here
//    HTREEITEM hItem = ((NMTREEVIEWW*)pNMHDR)->itemNew.hItem;
//    *pResult = 0;
//    //HTREEITEM hItem = GetSelectedItem();
//    //if (SymbolStore *pSymbolStore = theApp.GetSymbolStore()) {
//    if (Node *pNode = (Node*)GetItemData(hItem)) {
//        SYMBOL_DESC symDesc;
//        pNode->GetSymbolDesc(symDesc);
//        CMainFrame *pMainFrame = (CMainFrame*)theApp.m_pMainWnd;
//        if (CScriptView *pScriptView = pMainFrame->GetScriptView(symDesc.m_fileId)) {
//            //ui32 line = stoul(string(_bstr_t(strLine)));
//            pScriptView->HighlightLine(HIGHLIGHT_BREAKPOINT, -1, symDesc.m_line - START_POSITION_OFFSET, INVALID_THREAD_ID, 0);
//        }
//    }
//    else {
//        POSITION pos = theApp.m_pDocManager->GetFirstDocTemplatePosition();
//        if (CDocTemplate *pDocTemplate = theApp.m_pDocManager->GetNextDocTemplate(pos)) {
//            CDocument* pOpenDocument = NULL;
//            POSITION docPos = pDocTemplate->GetFirstDocPosition();
//            while (docPos) {
//                pOpenDocument = pDocTemplate->GetNextDoc(docPos);
//                CString path = pOpenDocument->GetPathName();
//                POSITION pos = pOpenDocument->GetFirstViewPosition();
//                static_cast<CScriptView*>(pOpenDocument->GetNextView(pos))->ClearHighlight();
//            }
//        }
//    }
//}

/////////////////////////////////////////////////////////////////////////////

CClassView::CClassView()
{
	m_nCurrSort = ID_SORTING_GROUPBYTYPE;
}

CClassView::~CClassView()
{
}

BEGIN_MESSAGE_MAP(CClassView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
    //ON_WM_LBUTTONDBLCLK()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_CLASS_ADD_MEMBER_FUNCTION, OnClassAddMemberFunction)
	ON_COMMAND(ID_CLASS_ADD_MEMBER_VARIABLE, OnClassAddMemberVariable)
	ON_COMMAND(ID_CLASS_DEFINITION, OnClassDefinition)
	ON_COMMAND(ID_CLASS_PROPERTIES, OnClassProperties)
	ON_COMMAND(ID_NEW_FOLDER, OnNewFolder)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_COMMAND_RANGE(ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnSort)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnUpdateSort)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClassView message handlers

int CClassView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create views:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndClassView.Create(dwViewStyle, rectDummy, this, 2))
	{
		TRACE0("Failed to create Class View\n");
		return -1;      // fail to create
	}

	// Load images:
	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_SORT);
	m_wndToolBar.LoadToolBar(IDR_SORT, 0, 0, TRUE /* Is locked */);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	CMenu menuSort;
	menuSort.LoadMenu(IDR_POPUP_SORT);

    m_wndToolBar.ReplaceButton(ID_SORT_MENU, CClassViewMenuButton(menuSort.GetSubMenu(0)->GetSafeHmenu()));

	CClassViewMenuButton* pButton =  DYNAMIC_DOWNCAST(CClassViewMenuButton, m_wndToolBar.GetButton(0));

	if (pButton != NULL)
	{
		pButton->m_bText = FALSE;
		pButton->m_bImage = TRUE;
		pButton->SetImage(GetCmdMgr()->GetCmdImage(m_nCurrSort));
		pButton->SetMessageWnd(this);
	}

	// Fill in some static tree view data (dummy code, nothing magic here)
    FillClassView();

	return 0;
}

void CClassView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}
//
//void CClassView::OnLButtonDblClk(UINT, CPoint) {
//
//}

void CClassView::FillClassView()
{
    SymbolStore *pSymStore = theApp.GetSymbolStore();
    Node *pRootNode = 0;
    if (Memory *pMemory = theApp.GetMemory()) {
        pRootNode = pMemory->GetRootScope();
    }

    if (!pRootNode || !pSymStore) {
        return;
    }
    HTREEITEM hRoot = m_wndClassView.InsertItem(_T("Function types"), 0, 0);
    m_wndClassView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

    Scope *pRootScope = pRootNode->QueryType<Scope>(TYPE_SCOPE);
    vector<Node*> &expressions = pRootScope->GetExpressionsRef();
    
    struct FileInfo {
        string      fileName;
        HTREEITEM   hItem;
        FileInfo(string fn, HTREEITEM h) : fileName(fn), hItem(h) {}
    };
    map<ui32, FileInfo> fileMap;
    for (ui32 i = 0; i < expressions.size(); ++i) {
        SYMBOL_DESC symDesc;
        Node *pExpNode = expressions[i];
        if (Function *pFunction = pExpNode->QueryType<Function>(TYPE_FUNCTION)) {
            HTREEITEM hItem = hRoot;
            string typeName, fileName;
            pSymStore->GetSymbolName(pFunction, typeName);
            if (pSymStore->GetFileName(pFunction->GetFileId(), fileName)) {
                pair<map<ui32, FileInfo>::iterator, bool> res = fileMap.insert(map<ui32, FileInfo>::value_type(pFunction->GetFileId(), FileInfo(fileName, 0)));
                if (res.second) {
                    hItem = m_wndClassView.InsertItem(_bstr_t(fileName.c_str()), 0, 0, hRoot);
                    m_wndClassView.SetItemState(hItem, TVIS_EXPANDED, TVIS_EXPANDED);
                    //m_wndClassView.Expand(hItem, TVE_EXPAND);
                    res.first->second.hItem = hItem;
                }
                else {
                    hItem = res.first->second.hItem;
                }
            }
            switch (pFunction->GetType()) {
                case TYPE_LOCK:
                case TYPE_UNLOCK: {
                    typeName = "@ " + typeName + "(@)";
                    HTREEITEM h = m_wndClassView.InsertItem(_bstr_t(typeName.c_str()), 5, 5, hItem);
                    break;
                }
                case TYPE_CAST: {
                    typeName = "@ " + typeName + "<Type>(@)";
                    HTREEITEM h = m_wndClassView.InsertItem(_bstr_t(typeName.c_str()), 5, 5, hItem);
                    break;
                }
                default:
                    FunctionRefType *pFunctionRefType = pFunction->GetFunctionRefTypeNode();
                    Type *pRetunrType = pFunctionRefType->GetReturnType();
                    string retType = pRetunrType->GetTypeString(pSymStore);
                    retType += " " + typeName + "(";
                    switch (pFunction->GetType()) {
                        case TYPE_ARR2STR:
                        case TYPE_WARR2STR:
                        case TYPE_STR2ARR:
                        case TYPE_STR2WARR:
                            if (Type *pArgType = static_cast<BuiltIn1Arg*>(pFunction)->GetArgType()) {
                                retType += pArgType->GetTypeString(pSymStore);
                            }
                        case TYPE_SIZEOF:
                            retType += ")";
                            {
                                HTREEITEM h = m_wndClassView.InsertItem(_bstr_t(retType.c_str()), 5, 5, hItem);
                                //m_wndClassView.SetItemData(h, 0);
                            }
                            break;
                        default:
                            ui32 argCount = pFunctionRefType->GetArgCount();
                            for (ui32 i = 0; i < argCount; ++i) {
                                if (Type *pArgType = pFunctionRefType->GetArgType(i + 1)) {
                                    retType += pArgType->GetTypeString(pSymStore);
                                    if (i + 1 != argCount) {
                                        retType += ", ";
                                    }
                                }
                            }
                            retType += ")";
                            {
                                HTREEITEM h = m_wndClassView.InsertItem(_bstr_t(retType.c_str()), 3, 3, hItem);
                                m_wndClassView.SetItemData(h, (DWORD_PTR)pFunction);
                            }
                            break;
                    }
                    break;
            }

            //return retType;
        }
        else if (LibType *pLibType = pExpNode->QueryType<LibType>(TYPE_LIB)) {
            string typeName, fileName, libName;
            HTREEITEM hItem = hRoot;
            Variable *pLibVariable = pLibType->GetVariable(0);
            pSymStore->GetSymbolName(pLibVariable->GetSymbolId(), libName);
            if (pSymStore->GetFileName(pLibType->GetFileId(), fileName)) {
                pair<map<ui32, FileInfo>::iterator, bool> res = fileMap.insert(map<ui32, FileInfo>::value_type(pLibType->GetFileId(), FileInfo(fileName, 0)));
                if (res.second) {
                    hItem = m_wndClassView.InsertItem(_bstr_t(fileName.c_str()), 0, 0, hRoot);
                    m_wndClassView.SetItemState(hItem, TVIS_EXPANDED, TVIS_EXPANDED);
                    //m_wndClassView.SetItemData(hItem, 0);
                    //m_wndClassView.Expand(hItem, TVE_EXPAND);
                    res.first->second.hItem = hItem;
                }
                else {
                    hItem = res.first->second.hItem;
                }
            }
            pSymStore->GetSymbolName(pLibType, typeName);
            libName += ": " + typeName;
            hItem = m_wndClassView.InsertItem(_bstr_t(libName.c_str()), 1, 1, hItem);
            m_wndClassView.SetItemData(hItem, (DWORD_PTR)pLibType);

            ui32 count = pLibType->GetSubTypeCount();
            for (ui32 i = 0; i < count; ++i) {
                FunctionRefType *pFunctionRefType = pLibType->GetSubType(i)->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE);
                pSymStore->GetSymbolName(pFunctionRefType, typeName);
                Type *pRetunrType = pFunctionRefType->GetReturnType();
                string retType = pRetunrType->GetTypeString(pSymStore);
                retType += " " + typeName + "(";
                ui32 argCount = pFunctionRefType->GetArgCount();
                for (ui32 i = 0; i < argCount; ++i) {
                    if (Type *pArgType = pFunctionRefType->GetArgType(i + 1)) {
                        retType += pArgType->GetTypeString(pSymStore);
                        if (i + 1 != argCount) {
                            retType += ", ";
                        }
                    }
                }
                retType += ")";
                {
                    HTREEITEM h = m_wndClassView.InsertItem(_bstr_t(retType.c_str()), 3, 3, hItem);
                    m_wndClassView.SetItemData(h, (DWORD_PTR)pFunctionRefType);
                }
            }
        }
    }
    m_wndClassView.Expand(hRoot, TVE_EXPAND);
	//HTREEITEM hRoot = m_wndClassView.InsertItem(_T("FakeApp classes"), 0, 0);
	//m_wndClassView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	//HTREEITEM hClass = m_wndClassView.InsertItem(_T("CFakeAboutDlg"), 1, 1, hRoot);
	//m_wndClassView.InsertItem(_T("CFakeAboutDlg()"), 3, 3, hClass);

	//m_wndClassView.Expand(hRoot, TVE_EXPAND);

	//hClass = m_wndClassView.InsertItem(_T("CFakeApp"), 1, 1, hRoot);
	//m_wndClassView.InsertItem(_T("CFakeApp()"), 3, 3, hClass);
	//m_wndClassView.InsertItem(_T("InitInstance()"), 3, 3, hClass);
	//m_wndClassView.InsertItem(_T("OnAppAbout()"), 3, 3, hClass);

	//hClass = m_wndClassView.InsertItem(_T("CFakeAppDoc"), 1, 1, hRoot);
	//m_wndClassView.InsertItem(_T("CFakeAppDoc()"), 4, 4, hClass);
	//m_wndClassView.InsertItem(_T("~CFakeAppDoc()"), 3, 3, hClass);
	//m_wndClassView.InsertItem(_T("OnNewDocument()"), 3, 3, hClass);

	//hClass = m_wndClassView.InsertItem(_T("CFakeAppView"), 1, 1, hRoot);
	//m_wndClassView.InsertItem(_T("CFakeAppView()"), 4, 4, hClass);
	//m_wndClassView.InsertItem(_T("~CFakeAppView()"), 3, 3, hClass);
	//m_wndClassView.InsertItem(_T("GetDocument()"), 3, 3, hClass);
	//m_wndClassView.Expand(hClass, TVE_EXPAND);

	//hClass = m_wndClassView.InsertItem(_T("CFakeAppFrame"), 1, 1, hRoot);
	//m_wndClassView.InsertItem(_T("CFakeAppFrame()"), 3, 3, hClass);
	//m_wndClassView.InsertItem(_T("~CFakeAppFrame()"), 3, 3, hClass);
	//m_wndClassView.InsertItem(_T("m_wndMenuBar"), 6, 6, hClass);
	//m_wndClassView.InsertItem(_T("m_wndToolBar"), 6, 6, hClass);
	//m_wndClassView.InsertItem(_T("m_wndStatusBar"), 6, 6, hClass);

	//hClass = m_wndClassView.InsertItem(_T("Globals"), 2, 2, hRoot);
	//m_wndClassView.InsertItem(_T("theFakeApp"), 5, 5, hClass);
	//m_wndClassView.Expand(hClass, TVE_EXPAND);
}

void CClassView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndClassView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// Select clicked item:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != NULL)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	CMenu menu;
	menu.LoadMenu(IDR_POPUP_SORT);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}
}

void CClassView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndClassView.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

BOOL CClassView::PreTranslateMessage(MSG* pMsg)
{
	return CDockablePane::PreTranslateMessage(pMsg);
}

void CClassView::OnSort(UINT id)
{
	if (m_nCurrSort == id)
	{
		return;
	}

	m_nCurrSort = id;

	CClassViewMenuButton* pButton =  DYNAMIC_DOWNCAST(CClassViewMenuButton, m_wndToolBar.GetButton(0));

	if (pButton != NULL)
	{
		pButton->SetImage(GetCmdMgr()->GetCmdImage(id));
		m_wndToolBar.Invalidate();
		m_wndToolBar.UpdateWindow();
	}
}

void CClassView::OnUpdateSort(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(pCmdUI->m_nID == m_nCurrSort);
}

void CClassView::OnClassAddMemberFunction()
{
	AfxMessageBox(_T("Add member function..."));
}

void CClassView::OnClassAddMemberVariable()
{
	// TODO: Add your command handler code here
}

void CClassView::OnClassDefinition()
{
	// TODO: Add your command handler code here
}

void CClassView::OnClassProperties()
{
	// TODO: Add your command handler code here
}

void CClassView::OnNewFolder()
{
	AfxMessageBox(_T("New Folder..."));
}

void CClassView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_wndClassView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CClassView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndClassView.SetFocus();
}

void CClassView::OnChangeVisualStyle()
{
	m_ClassViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_CLASS_VIEW_24 : IDB_CLASS_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_ClassViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_ClassViewImages.Add(&bmp, RGB(255, 0, 0));

	m_wndClassView.SetImageList(&m_ClassViewImages, TVSIL_NORMAL);

	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_SORT_24 : IDR_SORT, 0, 0, TRUE /* Locked */);
}




void CFunctionDefTree::OnLButtonUp(UINT nFlags, CPoint point)
{
    // TODO: Add your message handler code here and/or call default

    CTreeCtrl::OnLButtonUp(nFlags, point);
    OnLButtonDblClk(nFlags, point);
}
