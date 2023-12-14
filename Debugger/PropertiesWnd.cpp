
#include "stdafx.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "MFCApplication1.h"
#include <iostream>
//#include <fstream>
//#include <string>
#include <sstream>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CPropertiesWnd::CPropertiesWnd() : m_viewAsType(CVarList::VIEW_AS_DEFAULT), m_viewAddress(false)
{
	//m_nComboHeight = 0;
}

CPropertiesWnd::~CPropertiesWnd() {

}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
    ON_WM_LBUTTONUP()
    ON_WM_CLOSE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd () == NULL || (AfxGetMainWnd() != NULL && AfxGetMainWnd()->IsIconic()))
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);
}

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDockablePane::OnCreate(lpCreateStruct) == -1)
        return -1;

    LoadData();

    CRect rectDummy;
    rectDummy.SetRectEmpty();

    // Create tabs window:
    if (!m_tabsWnd.Create(CMFCTabCtrl::STYLE_FLAT, rectDummy, this, 1)) {
        TRACE0("Failed to create output tab window\n");
        return -1;      // fail to create
    }

    m_wndBreakpointList.Create(WS_VISIBLE | WS_CHILD | LVS_REPORT, rectDummy, &m_tabsWnd, 2);
    m_wndBreakpointList.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    m_wndGlobalVarList.Create(WS_VISIBLE | WS_CHILD | LVS_REPORT, rectDummy, &m_tabsWnd, 3);
    m_wndGlobalVarList.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    m_wndThreadList.Create(WS_VISIBLE | WS_CHILD | LVS_REPORT, rectDummy, &m_tabsWnd, 4);
    m_wndThreadList.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    //m_wndWatchList.Create(WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS, rectDummy, &m_tabsWnd, 4);
    //m_wndWatchList.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    CString strTabName;

    BOOL bNameValid = strTabName.LoadString(IDS_BREAKPOINTS);
    ASSERT(bNameValid);
    m_tabsWnd.AddTab(&m_wndBreakpointList, strTabName);
    

    bNameValid = strTabName.LoadString(IDS_GLOBAL_VARS_TAB);
    ASSERT(bNameValid);
    m_tabsWnd.AddTab(&m_wndGlobalVarList, strTabName, (UINT)0);
    

    bNameValid = strTabName.LoadString(IDS_THREADS_TAB);
    ASSERT(bNameValid);
    m_tabsWnd.AddTab(&m_wndThreadList, strTabName, (UINT)0);
    //bNameValid = strTabName.LoadString(IDS_WATCH_TAB);
    //ASSERT(bNameValid);
    //m_tabsWnd.AddTab(&m_wndWatchList, strTabName, (UINT)0),

    m_imageList.DeleteImageList();

    UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_TREE_MARKERS : IDB_CLASS_VIEW;

    CBitmap bmp;
    if (!bmp.LoadBitmap(uiBmpId))
    {
        TRACE(_T("Can't load bitmap: %x\n"), uiBmpId);
        ASSERT(FALSE);
        return -1;
    }

    BITMAP bmpObj;
    bmp.GetBitmap(&bmpObj);

    UINT nFlags = ILC_MASK;

    nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

    m_imageList.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
    m_imageList.Add(&bmp, RGB(255, 0, 0));
    
    m_wndGlobalVarList.SetImageList(&m_imageList, LVSIL_SMALL);
    m_wndThreadList.SetImageList(&m_imageList, LVSIL_SMALL);
    //m_wndWatchList.SetImageList(&m_imageList, LVSIL_SMALL);

    uiBmpId = /*theApp.m_bHiColorIcons ? IDB_BREAKPOINTS : */IDB_BITMAP5;//IDB_BITMAP2;
    bmp.DeleteObject();
    bmp.LoadBitmap(uiBmpId);
    bmp.GetBitmap(&bmpObj);
    
    m_imageBpList.Create(20, bmpObj.bmHeight, nFlags, 0, 0);
    m_imageBpList.Add(&bmp, RGB(255, 0, 0));

    m_wndBreakpointList.SetImageList(&m_imageBpList, LVSIL_SMALL);

    m_wndGlobalVarList.m_pPropWnd = this;

    InitVarListHeader(m_wndGlobalVarList);
    m_wndThreadList.Init();
    //InitVarListHeader(m_wndWatchList);
    //m_wndWatchList.Load();
    //m_wndWatchList.AddSimpleNode(CString(""), (LPARAM)0, CString());
    InitBreakpointListHeader();
    m_wndBreakpointList.Load();
	return 0;
}

void CPropertiesWnd::OnBeginThread(ThreadId threadId) {
    CRect rect;

    rect.SetRectEmpty();
    m_tabsWnd.GetClientRect/*.GetWndArea*/(rect);

    rect.bottom = rect.Height() / 2;

    TAB_HOLDER  *pTabHolder = new TAB_HOLDER();

    m_tabHolders.insert(pair<ThreadId, TAB_HOLDER*>(threadId, pTabHolder));
    pTabHolder->Create(&m_tabsWnd, rect, m_tabHolders.size(), threadId);
    pTabHolder->m_pVarListWnd->SetImageList(&m_imageList, LVSIL_SMALL);

    pTabHolder->m_pVarListWnd->m_pPropWnd = this;

    InitVarListHeader(*pTabHolder->m_pVarListWnd);
    InitStackListHeader(*pTabHolder->m_pStackListWnd);
    UpdateStackList(threadId);
    m_wndThreadList.Add(threadId);
}

void CPropertiesWnd::TabHolder::Create(CMFCTabCtrl *pTabsWnd, CRect &rect, int idx, ThreadId threadId) {
    m_threadId = threadId;
    m_pTabsWnd = pTabsWnd;
    m_pSplitterWnd = new CSplitterWnd();
    m_pVarListWnd = new CVarList();
    m_pStackListWnd = new CStackList();


    m_pSplitterWnd->CreateStatic(m_pTabsWnd, 2, 1);

    m_pSplitterWnd->CreateView(0, 0, RUNTIME_CLASS(CSplitterView), CSize(rect.Width(), rect.Height()), 0);
    CSplitterView *pView = (CSplitterView*)m_pSplitterWnd->GetPane(0, 0);
    pView->SetChild(m_pVarListWnd);
    m_pVarListWnd->Create(WS_VISIBLE | WS_CHILD | LVS_REPORT, rect, pView, 8 + 2 * idx - 1);
    m_pVarListWnd->SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_pSplitterWnd->CreateView(1, 0, RUNTIME_CLASS(CSplitterView), CSize(rect.Width(), rect.Height()), 0);
    pView = (CSplitterView*)m_pSplitterWnd->GetPane(1, 0);
    pView->SetChild(m_pStackListWnd);
    m_pStackListWnd->Create(WS_VISIBLE | WS_CHILD | LVS_REPORT, rect, pView, 8 + 2 * idx);
    m_pStackListWnd->SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    CString strName = GetName();
    m_pTabsWnd->AddTab(m_pSplitterWnd, strName);// , (UINT)2 + threadId);
}

CString CPropertiesWnd::TabHolder::GetName() {
    _bstr_t name("Thread: ");
    name += to_string(m_threadId).c_str();
    return CString(name.GetBSTR());
}

void CPropertiesWnd::OnEndThread(ThreadId threadId) {
    TAB_HOLDER_MAP::iterator it = m_tabHolders.find(threadId);
    if (it != m_tabHolders.end()) {
        CString strName = it->second->GetName();
        for (int i = 0; i < m_tabsWnd.GetTabsNum(); ++i) {
            CString strLabel;
            m_tabsWnd.GetTabLabel(i, strLabel);
            if (strName == strLabel) {
                it->second->m_pVarListWnd->ClearList();
                //it->second->m_pVarListWnd->SetScope(0);
                m_tabsWnd.LockWindowUpdate();
                if (m_tabsWnd.GetActiveTab() == i) {
                    m_tabsWnd.SetActiveTab(i - 1);
                }
                m_tabsWnd.RemoveTab(i);
                m_tabsWnd.UnlockWindowUpdate();
                delete it->second;
                m_tabHolders.erase(it);
                break;
            }
        }
    }
    if (!m_tabHolders.size()) { // main thread terminating
        ClearGlobalVarList();
    }
    m_wndThreadList.Remove(threadId);
}

//
void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
    m_tabsWnd.SetWindowPos(NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void CPropertiesWnd::InitStackListHeader(CListCtrl &listCtrl) {
    // Insert the columns into the list view.
    listCtrl.InsertColumn(0, L"Scope Name");
    listCtrl.SetColumnWidth(0, 200);

    listCtrl.InsertColumn(1, L"Code Location");
    listCtrl.SetColumnWidth(1, 80);
    
    listCtrl.InsertColumn(2, L"Line");
    listCtrl.SetColumnWidth(2, 40);
    
    listCtrl.InsertColumn(3, L"File Path");
    listCtrl.SetColumnWidth(3, 300);
}

void CPropertiesWnd::InitVarListHeader(CVarList &varList) {
    varList.InsertColumn(0, L"Variable");
    varList.SetColumnWidth(0, 100);
    varList.InsertColumn(1, L"Value");
    varList.SetColumnWidth(1, 120);
    varList.InsertColumn(2, L"Type");
    varList.SetColumnWidth(2, 200);
    if (m_viewAddress) {
        varList.InsertColumn(3, L"Address");
        varList.SetColumnWidth(3, 100);
    }
}

void CPropertiesWnd::InitBreakpointListHeader() {
    m_wndBreakpointList.InsertColumn(0, L"Line");
    m_wndBreakpointList.SetColumnWidth(0, 50);
    m_wndBreakpointList.InsertColumn(1, L"File");
    m_wndBreakpointList.SetColumnWidth(1, 100);
    m_wndBreakpointList.InsertColumn(2, L"Path");
    m_wndBreakpointList.SetColumnWidth(2, 300);
}

void CPropertiesWnd::ClearGlobalVarList() {
    //m_wndStackList.DeleteAllItems();
    //m_wndLocalVarList.ClearList();
    //m_wndLocalVarList.SetScope(0);
    m_wndGlobalVarList.ClearList();
    m_wndGlobalVarList.SetScope(0, INVALID_STACK_FRAME_IDX);
}

void CPropertiesWnd::UpdateVarList(ThreadId threadId, ui32 frameIdx, Scope *pScope, CVarList &varlList) {
    if (varlList.SetScope(pScope, frameIdx)) {
        varlList.ClearList();
        varlList.LoadVarList(threadId, frameIdx);
    }
    else {
        varlList.Invalidate();
    }
}

void CPropertiesWnd::UpdateActiveVarList(ThreadId threadId, ui32 frameIdx, Scope *pFunctionScope) {
    TAB_HOLDER_MAP::iterator it = m_tabHolders.find(threadId);
    if (it != m_tabHolders.end()) {
        CVarList *pVarList = it->second->m_pVarListWnd;
        if (pVarList->SetScope(pFunctionScope, frameIdx)) {
            it->second->m_pVarListWnd->ClearList();
            it->second->m_pVarListWnd->LoadVarList(threadId, frameIdx);
        }
        else {
            it->second->m_pVarListWnd->Invalidate();
        }

    }
}

void CPropertiesWnd::UpdateStackList(ThreadId threadId) {
    if (Memory *pMemory = theApp.GetMemory()) {
        StackMemory *pStackMemory = pMemory->GetMainStackMemory();
        Scope *pScope = pMemory->GetRootScope();
        UpdateVarList(threadId, 0, pScope, m_wndGlobalVarList);
        pStackMemory = pMemory->GetThreadMemory(threadId);
        ui32 count = pStackMemory->GetFrameIdx();

        CVarList   *pVarListWnd = 0;
        CStackList *pStackListWnd = 0;
        TAB_HOLDER_MAP::iterator it = m_tabHolders.find(threadId);
        if (it != m_tabHolders.end()) {
            pVarListWnd = it->second->m_pVarListWnd;
            pStackListWnd = it->second->m_pStackListWnd;
            it->second->m_pStackListWnd->DeleteAllItems();
            if (!count) {
                UpdateVarList(threadId, count, pScope, *pVarListWnd);
            }
        }
        else {
            return;
        }
        SymbolStore *pSymStore = theApp.GetSymbolStore();
        if (!pSymStore) {
            return;
        }
        ++count;
        for (ui32 frameIdx = 0; frameIdx < count; ++frameIdx) {
            StackMemory *pStackMemory = pMemory->GetThreadMemory(threadId);
            string fooType, location, fileName;
            ui32 line, pos;
            if (pStackMemory->GetFrameFunctionAsStrings(frameIdx, fooType, location, fileName, line, pos)) {
                if (frameIdx + 1 == count) {
                    Node  *pFunctionNode = 0;
                    Scope *pFunctionScope = 0;
                    Node  *pCurrentNode = 0;
                    pStackMemory->GetFrameFunctionAndScope(frameIdx, pFunctionNode, pFunctionScope, pCurrentNode);
                    UpdateVarList(threadId, frameIdx, pFunctionScope, *pVarListWnd);
                    m_wndThreadList.SetSymbol(threadId, _bstr_t(fooType.c_str()), _bstr_t(to_string(line).c_str()), _bstr_t(fileName.c_str()));
                }

                pStackListWnd->AddData(0, 0, (LPWSTR)_bstr_t(fooType.c_str()), frameIdx);
                // location        
                pStackListWnd->AddData(0, 1, (LPWSTR)_bstr_t(location.c_str()), 0);
                // Line
                pStackListWnd->AddData(0, 2, (LPWSTR)_bstr_t(to_string(line).c_str()), 0);
                // File name
                pStackListWnd->AddData(0, 3, (LPWSTR)_bstr_t(fileName.c_str()), 0);
            }
        }
    }
}

CVarList::VIEW_AS_TYPE CPropertiesWnd::GetViewAs() {
    return m_viewAsType;
}

void CPropertiesWnd::SetViewAs(CVarList::VIEW_AS_TYPE viewAs) {
    m_viewAsType = viewAs;
}

bool CPropertiesWnd::GetViewAddress() {
    return m_viewAddress;
}

void CPropertiesWnd::SetViewAddress(bool viewAddress) {
    m_viewAddress = viewAddress;
}

void CPropertiesWnd::ToggleAddress() {
    if (m_viewAddress) {
        m_wndGlobalVarList.InsertColumn(3, L"Address");
        m_wndGlobalVarList.SetColumnWidth(3, 100);
        for (TAB_HOLDER_MAP::iterator it = m_tabHolders.begin(); it != m_tabHolders.end(); ++it) {
            it->second->m_pVarListWnd->InsertColumn(3, L"Address");
            it->second->m_pVarListWnd->SetColumnWidth(3, 100);
        }
    }
    else {
        m_wndGlobalVarList.DeleteColumn(3);
        for (TAB_HOLDER_MAP::iterator it = m_tabHolders.begin(); it != m_tabHolders.end(); ++it) {
            it->second->m_pVarListWnd->DeleteColumn(3);
        }
    }
}


void CPropertiesWnd::DisableTab(BOOL disable, ThreadId threadId) {
    TAB_HOLDER_MAP::iterator it = m_tabHolders.find(threadId);
    if (it != m_tabHolders.end()) {
        CString strName = it->second->GetName();
        for (int i = 0; i < m_tabsWnd.GetTabsNum(); ++i) {
            CString strLabel;
            m_tabsWnd.GetTabLabel(i, strLabel);
            if (strName == strLabel) {
                //COLORREF color = -1;
                //m_tabsWnd.SetTabTextColor(i, disable ? 0x004040ff : 0xffffffff);
                m_tabsWnd.SetBkColor(i, disable);
                it->second->m_pVarListWnd->SetTextColor(disable ? 0x004040ff : 0x00000000);
                it->second->m_pStackListWnd->SetTextColor(disable ? 0x004040ff : 0x00000000);
                //m_tabsWnd.SetTabTextColor(i, disable ? 0x0099ff99 : 0x00000000);
                //m_tabsWnd.SetActiveTabColor(0x009999ff);
                //m_tabsWnd.SetImageList(IDI_THREAD_RUNNING);
                //m_tabsWnd.SetTabIcon(i, disable ? 0 : -1);
                m_tabsWnd.GetTabWnd(i)->EnableWindow(!disable);
                break;
            }
        }
    }
    m_wndThreadList.SetBlocked(threadId, disable == TRUE);
}

void CTabs::SetBkColor(int iTab, BOOL disabled) {
    SetTabBkColor(iTab, disabled ? m_disabledBkColor : m_defaultBkColor);
    InvalidateTab(iTab);
}

BOOL CTabs::SetActiveTab(int iTab) {
    if (CMFCTabCtrl::SetActiveTab(iTab)) {
        CString title;
        CMFCTabCtrl::GetTabLabel(iTab, title);
        static_cast<CPropertiesWnd*>(GetParent())->SetWindowText(title);
        COLORREF c = GetTabBkColor(iTab);
        //if (c == m_disbaledBkColor) {
            SetActiveTabColor(c);
        //}
        //else {
        //    SetActiveTabColor(m_defaultBkColor);
        //}
        InvalidateTab(iTab);
        return TRUE;
    }
    return FALSE;
}

BREAKPOINT_MARKER CPropertiesWnd::InsertBreakpoint(_bstr_t &path, _bstr_t &fileName, _bstr_t &line) {
    return m_wndBreakpointList.InsertBreakpoint(path, fileName, line);
}

void CPropertiesWnd::DeleteBreakpoint(_bstr_t &path, _bstr_t &fileName, _bstr_t &line) {
    m_wndBreakpointList.DeleteBreakpoint(path, fileName, line);
}

void CPropertiesWnd::GetBreakpoints(_bstr_t &path, _bstr_t &fileName, vector<BREAKPOINT_INFO> &breakpoints) {
    m_wndBreakpointList.GetBreakpoints(path, fileName, breakpoints); 
}

void CPropertiesWnd::PropagateBreakpoints() {
    m_wndBreakpointList.PropagateBreakpoints();
}

void CPropertiesWnd::InvalidateAllBreakpoints() {
    m_wndBreakpointList.InvalidateAllBreakpoints();
}
void CPropertiesWnd::MarkAllValidBreakpoints(_bstr_t &path, _bstr_t &fileName) {
    m_wndBreakpointList.MarkAllValidBreakpoints(path, fileName);
}

void CPropertiesWnd::UpdateBreakpointLine(_bstr_t &path, _bstr_t &fileName, _bstr_t &oldLine, _bstr_t &newLine) {
    m_wndBreakpointList.UpdateBreakpointLine(path, fileName, oldLine, newLine);
}

void CPropertiesWnd::RestoreBreakpopints(_bstr_t &path, _bstr_t &fileName, unordered_set<ui32> &breakpointLines) {
    m_wndBreakpointList.RestoreBreakpopints(path, fileName, breakpointLines);
}

void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	//m_wndLocalVarsList.SetFocus();
    //m_wndStackList.SetFocus();
    m_wndGlobalVarList.SetFocus();
}

void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	//SetPropListFont();
}

/*****************************************/

BEGIN_MESSAGE_MAP(CVarList, CListCtrl)
    ON_NOTIFY_REFLECT(NM_CLICK, &CVarList::OnNMClick)
    ON_NOTIFY_REFLECT(LVN_GETDISPINFO, &CVarList::OnLvnGetdispinfo)
    //ON_COMMAND(IDR_POPUP_VARLIST, &CVarList::OnIdrPopupVarlist)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_POPUP_VIEWASBINARY, &CVarList::OnPopupViewasbinary)
    ON_COMMAND(ID_POPUP_VIEWASDEFAULT, &CVarList::OnPopupViewasdefault)
    ON_COMMAND(ID_POPUP_VIEWASHEX, &CVarList::OnPopupViewashex)
    ON_COMMAND(ID_POPUP_VIEWADDRESS, &CVarList::OnPopupViewaddress)
    ON_UPDATE_COMMAND_UI(ID_POPUP_VIEWADDRESS, &CVarList::OnUpdatePopupViewaddress)
    ON_UPDATE_COMMAND_UI(ID_POPUP_VIEWASBINARY, &CVarList::OnUpdatePopupViewasbinary)
    ON_UPDATE_COMMAND_UI(ID_POPUP_VIEWASDEFAULT, &CVarList::OnUpdatePopupViewasdefault)
    ON_UPDATE_COMMAND_UI(ID_POPUP_VIEWASHEX, &CVarList::OnUpdatePopupViewashex)
    ON_COMMAND(ID_POPUP_COPY, &CVarList::OnPopupCopy)
END_MESSAGE_MAP()

BOOL CVarList::SetScope(Scope *pScope, ui32 frameIdx) { 
    if ((m_pScope != pScope) || (frameIdx != m_frameIdx)) { 
        m_pScope = pScope;
        m_frameIdx = frameIdx;
        return TRUE;
    }
    return FALSE;
}

void CVarList::LoadFooArgList(ThreadId threadId, ui32 frameIdx) {
    Memory *pMemory = theApp.GetMemory();
    StackMemory *pStackMemory = pMemory->GetThreadMemory(threadId);
    ui32 topFrameIdx = pStackMemory->GetFrameIdx();
    if (INVALID_STACK_FRAME_IDX != frameIdx) {
        pStackMemory->SetFrameIndex(frameIdx);
    }
    else {
        frameIdx = topFrameIdx;
    }

    ui32 globalBit = (frameIdx == 0) ? MEMORY_BLOCK_BIT_GLOBAL : 0;
    if (SymbolStore *pSymStore = theApp.GetSymbolStore()) {
        ui32 count = pStackMemory->GetCurFrameMemoryBlockRefCount();
        Node *pFunctionNode = frameIdx ? pStackMemory->GetFrameFunctionNode(frameIdx) : 0;
        FunctionRefType *pFunctionRefType = 0;
        vector<LOCAL_VAR_DESCRIPTOR> argDesc;
        if (pFunctionNode) {
            switch (pFunctionNode->GetType()) {
                case TYPE_FUNCTION_CALLBACK_PTR:
                    pFunctionNode = pFunctionNode->GetParent()->QueryType<Function>(TYPE_FUNCTION);
                case TYPE_FUNCTION:
                case TYPE_FUNCTION_PTR:
                case TYPE_FUNCTION_LIB_PTR:
                    pFunctionRefType = ((Function*)pFunctionNode)->GetFunctionRefTypeNode();
                    break;
                default:
                    assert(0);
                    break;
            }
            if (pFunctionRefType) {
                pFunctionRefType->GetArgDescs(argDesc);
            }
        }
        for (ui32 i = 0; i < count; ++i) {
            MemoryBlockRef *pMemoryBlockRef = pStackMemory->GetMemoryBlockRef(i);
            string varName = to_string(i), typeName;
            Type *pType = 0;
            if (argDesc.size() > i) {
                pType = (Type*)argDesc[i];// it gets argument's declared type including reference(s)
            }
            else {
                pType = pMemoryBlockRef->GetReferenceTypeNode();
            }
            typeName = pType->GetTypeString(pSymStore);
            vector<ItemDesc*>::iterator it = m_ItemDescVector.begin() + i;
//#ifdef _DEBUG
            m_ItemDescVector.insert(it, new ItemDesc(string(), varName, typeName, threadId, frameIdx, i, pType));
//#else // _DEBUG
//            m_ItemDescVector.insert(it, new ItemDesc(varName, pMemoryBlockRef, pMemoryBlockRef->GetOffset(), pType));
//#endif // _DEBUG

            switch (pType->GetType()) {
                case TYPE_ARRAY:
                case TYPE_AGGREGATE_TYPE:
                case TYPE_LIB:
                case TYPE_ERROR_TYPE:
                    AddTreeNode();
                    break;
                default:
                    AddSimpleNode();
                    break;
            }
        }

        if (INVALID_STACK_FRAME_IDX != frameIdx) {
            pStackMemory->SetFrameIndex(topFrameIdx);
        }
    }
}

void CVarList::LoadVarList(ThreadId threadId, ui32 frameIdx) {
    if (m_pScope) {
        Memory *pMemory = theApp.GetMemory();
        StackMemory *pStackMemory = pMemory->GetThreadMemory(threadId);
        ui32 topFrameIdx = pStackMemory->GetFrameIdx();
        if (INVALID_STACK_FRAME_IDX != frameIdx) {
            pStackMemory->SetFrameIndex(frameIdx);
        }
        else {
            frameIdx = topFrameIdx;
        }
        ui32 globalBit = (frameIdx == 0) ? MEMORY_BLOCK_BIT_GLOBAL : 0;
        if (SymbolStore *pSymStore = theApp.GetSymbolStore()) {
            vector<LOCAL_VAR_DESCRIPTOR> &locaVarDesc = m_pScope->GetLocalVarDescriptors();
            int i = 0;
            //_bstr_t itemPath;
            for (vector<LOCAL_VAR_DESCRIPTOR>::iterator it = locaVarDesc.begin(); it != locaVarDesc.end(); ++it, ++i) {
                string varName, typeName;
                pSymStore->GetSymbolName(it->m_desc.m_symId, varName);
                Type *pType = (*it);
                typeName = pType->GetTypeString(pSymStore);
                MemoryBlockRef *pMemoryBlockRef = pStackMemory->GetMemoryBlockRef(i | globalBit);
                vector<ItemDesc*>::iterator itDesc = m_ItemDescVector.begin() + i;
//#ifdef _DEBUG
                m_ItemDescVector.insert(itDesc, new ItemDesc(string(), varName, typeName, threadId, frameIdx, i, pType));
//#else // _DEUBG
//                m_ItemDescVector.insert(itDesc, new ItemDesc(to_string(i), pMemoryBlockRef, pMemoryBlockRef->GetOffset(), pType));
//#endif // _DEBUG
                
                switch (pType->GetType()) {
                    case TYPE_ARRAY:
                    case TYPE_AGGREGATE_TYPE:
                    case TYPE_LIB:
                    case TYPE_ERROR_TYPE:
                        AddTreeNode();
                        break;
                    default:
                        AddSimpleNode();
                        break;
                }
            }
        }
        if (INVALID_STACK_FRAME_IDX != frameIdx) {
            pStackMemory->SetFrameIndex(topFrameIdx);
        }
    }
    else {
        LoadFooArgList(threadId, frameIdx);
    }
}

void CVarList::AddVarName(int row, IMAGE_TYPE img, int indent) {
    LVITEM lv;
    lv.iItem = row;
    lv.iSubItem = 0;
    lv.pszText = LPSTR_TEXTCALLBACK;
    lv.mask = LVIF_TEXT | LVIF_INDENT;
    lv.iIndent = indent;
    lv.iImage = img;
    lv.mask |= LVIF_IMAGE;
    InsertItem(&lv);

}

void CVarList::SetVarValue(int row) {
    LVITEM lv;
    lv.iItem = row;
    lv.iSubItem = 1;
    lv.pszText = LPSTR_TEXTCALLBACK;
    lv.mask = LVIF_TEXT;
    SetItem(&lv);
}

void CVarList::SetVarTypeName(int row) {
    LVITEM lv;
    lv.iItem = row;
    lv.iSubItem = 2;
    lv.pszText = LPSTR_TEXTCALLBACK;
    lv.mask = LVIF_TEXT;
    SetItem(&lv);
}

void CVarList::AppendNode(IMAGE_TYPE img) {
    int count = GetItemCount();
    AddVarName(count, img, 0);
    SetVarValue(count);
    SetVarTypeName(count);
}

void CVarList::AddSimpleNode() {
    AppendNode(IMG_NONE);
}

void CVarList::AddTreeNode() {
    AppendNode(IMG_COLLAPSED);
}

void CVarList::UpdateImg(int item, int indent, IMAGE_TYPE img) {
    LVITEM lv;
    lv.iItem = item;
    lv.iSubItem = 0;
    lv.iImage = img;
    lv.mask = LVIF_IMAGE;
    SetItem(&lv);
}

void CVarList::InsertTreeNode(int idxParent, int parentIndent, IMAGE_TYPE img, LPARAM pData) {
    int next = idxParent + 1;
    AddVarName(next, img, parentIndent + 1);
    SetVarValue(next);
    SetVarTypeName(next);
}

void CVarList::ExpandTreeNode(LVITEM &lv) {
    // must insert an item in collapsed state
    SymbolStore *pSymStore = theApp.GetSymbolStore();
    if (!pSymStore) {
        return;
    }
    vector<ItemDesc*>::iterator itDesc = m_ItemDescVector.begin() + lv.iItem;

    if (itDesc != m_ItemDescVector.end()) {
        ItemDesc *pItemDesc = *itDesc;
        if (MemoryBlockRef *pMemoryBlockRef = pItemDesc->GetMemoryBlockRef(*theApp.GetMemory())) {
            Type *pType = pMemoryBlockRef->GetReferenceTypeNode();
            switch (pType->GetType()) {
                case TYPE_ARRAY:
                    ExpandArray(pItemDesc->GetItemPath(), lv, pSymStore, *pItemDesc);
                    break;
                case TYPE_AGGREGATE_TYPE:
                case TYPE_ERROR_TYPE:
                    ExpandAggregate(pItemDesc->GetItemPath(), lv.iItem, lv.iIndent, pSymStore, *pItemDesc);
                    break;
                case TYPE_LIB:
                    ExpandLib(pItemDesc->GetItemPath(), lv.iItem, pSymStore, *pItemDesc);
                    break;
                default:
                    assert(0);
                    break;
            }
            pMemoryBlockRef->Release();
        }
    }
}

void CVarList::SubscriptCounter::PushMax(ui32 maxValue) {
    digits.push_back(_(maxValue));
}

void CVarList::SubscriptCounter::PushDigit() {
    for (ui32 i = 0; i < digits.size(); ++i) {
        _ &v = digits[i];
        if (v.m_max - 1 > v.m_current) {
            ++v.m_current;
            break;
        }
        else {
            v.m_current = 0;
        }
    }
}

void CVarList::SubscriptCounter::GetSubscriptString(string &s) {
    s = "[";
    for (ui32 i = 0; i < digits.size(); ++i) {
        s += to_string(digits[i].m_current);
        if (i + 1 != digits.size()) {
            s += ",";
        }
    }
    s += "]";
}

void CVarList::ExpandArray(string &itemPath, LVITEM &lv, SymbolStore *pSymStore, ItemDesc &itemDesc) {
    Memory &mem = *theApp.GetMemory();
    MemoryBlockRef *pMemoryBlockRef = itemDesc.GetMemoryBlockRef(mem);
    if (!pMemoryBlockRef) {
        return;
    }
    ArrayType *pArrayType = static_cast<ArrayType*>(pMemoryBlockRef->GetReferenceTypeNode());
    Type *pValueType = pArrayType->GetTypeNode();
    //bool reference = (pValueType->GetPassBy() != Type::BY_VALUE);
    ui32 dimCount = pArrayType->GetDimensionCount();
    ui32 valueSize = pValueType->GetValueSize();
    ui32 offset = pMemoryBlockRef->GetOffset();
    ui32 itemCount;
    SubscriptCounter ctr;
    if (dimCount == 0) {
        dimCount = 1;
        itemCount = (pMemoryBlockRef->GetMemoryBlock()->GetSize() - offset) / valueSize;
        ctr.PushMax(itemCount);
    }
    else {
        itemCount = 1;
        for (ui32 i = 0; i < dimCount; ++i) {
            ui32 subscript = pArrayType->GetDimensionSubscript(i);
            itemCount *= subscript;
            ctr.PushMax(subscript);
        }
    }
    if (itemCount) {
        UpdateImg(lv.iItem, lv.iIndent, IMG_EXPANDED);
    }
    string valueName, valueTypeName;
    valueTypeName = pValueType->GetTypeString(pSymStore);

    //MemoryBlockRef *_pMemoryBlockRef = 0;
    string subscriptStr;
    for (ui32 i = 0; i < itemCount; ++i) {

        if (i != 0) {
            ctr.PushDigit();
        }
        
        string subItemPath;
        if (itemPath.length()) {
            subItemPath = itemPath + "_" + to_string(i);
        }
        else {
            subItemPath = to_string(i);
        }
        valueName = _bstr_t(lv.pszText);
        ctr.GetSubscriptString(subscriptStr);
        valueName = valueName + subscriptStr;// "[" + to_string(i) + "]";
        vector<ItemDesc*>::iterator itDesc = m_ItemDescVector.begin() + lv.iItem + i + 1;
//#ifdef _DEBUG
        //MemoryBlockRef *pStackMemoryBlockRef = itemDesc.GetStackMemoryBlockRef(mem);
        m_ItemDescVector.insert(itDesc, new ItemDesc(subItemPath, valueName, valueTypeName, itemDesc.GetThreadId(), itemDesc.GetStackFrame(), itemDesc.GetVarIdx(), /*offset,*/ pValueType));
//#else // _DEUBG
//        m_ItemDescVector.insert(itDesc, new ItemDesc(subItemPath, pMemoryBlockRef, offset, pValueType));
//#endif // _DEBUG

        offset += valueSize;

        IMAGE_TYPE imgType;
        switch (pValueType->GetType()) {
            case TYPE_ARRAY:
            case TYPE_AGGREGATE_TYPE:
            case TYPE_LIB:
            case TYPE_ERROR_TYPE:
                imgType = IMG_COLLAPSED;
                break;
            default:
                imgType = IMG_NONE;
                break;
        }
        InsertTreeNode(lv.iItem + i, lv.iIndent, imgType, (LPARAM)i);

        valueName.clear();
    }
    pMemoryBlockRef->Release();
}

void CVarList::ExpandAggregate(string &itemPath, int row, int indent, SymbolStore *pSymStore, ItemDesc &itemDesc) {
    Memory &mem = *theApp.GetMemory();
    MemoryBlockRef *pMemoryBlockRef = itemDesc.GetMemoryBlockRef(mem);
    if (!pMemoryBlockRef) {
        return;
    }
    AggregateType *pAggregateType = static_cast<AggregateType*>(pMemoryBlockRef->GetReferenceTypeNode());
    ui32 subTypeCount = pAggregateType->GetSubTypeCount();

    UpdateImg(row, indent, IMG_EXPANDED);
    
    Memory *pMem = theApp.GetMemory();
    //MemoryBlockRef  *_pMemoryBlockRef = 0;
    ui32 newRow = row;
    for (ui32 i = 0; i < subTypeCount; ++i) {
        Type *pSubType = pAggregateType->GetSubType(i);
        ui32 varCount = pSubType->GetVarCount();
        for (ui32 k = 0; k < varCount; ++k) {
            Variable *pVariable = pSubType->GetVariable(k);
            string memberTypeName, memberName;
            Type *pMemberType = pVariable->GetTypeNode();
            memberTypeName = pMemberType->GetTypeString(pSymStore);
            pSymStore->GetSymbolName(pVariable->GetSymbolId(), memberName);
            ui32 offset = pMemoryBlockRef->GetOffset() + pVariable->GetAlignedMemberOffset();
            //ui32 offset = pVariable->GetAlignedMemberOffset();
            string subItemPath;
            if (itemPath.length()) {
                subItemPath = itemPath + "_" + to_string(i) + "_" + to_string(k);
            }
            else {
                subItemPath = to_string(i) + "_" + to_string(k);
            }

            //string subItemPath = itemPath + "_" + to_string(newRow - row);
            vector<ItemDesc*>::iterator itDesc = m_ItemDescVector.begin() + newRow + 1;
//#ifdef _DEBUG
            //MemoryBlockRef *pStackMemoryBlockRef = itemDesc.GetStackMemoryBlockRef(mem);
            m_ItemDescVector.insert(itDesc, new ItemDesc(subItemPath, memberName, memberTypeName, itemDesc.GetThreadId(), itemDesc.GetStackFrame(), itemDesc.GetVarIdx(), /*offset, */pMemberType));
//#else // _DEUBG
//            m_ItemDescVector.insert(itDesc, new ItemDesc(subItemPath, pMemoryBlockRef, offset, pMemberType));
//#endif // _DEBUG

            IMAGE_TYPE imgType;
            switch (pSubType->GetType()) {
                case TYPE_ARRAY:
                case TYPE_AGGREGATE_TYPE:
                case TYPE_LIB:
                case TYPE_ERROR_TYPE:
                    imgType = IMG_COLLAPSED;
                    break;
                default:
                    imgType = IMG_NONE;
                    break;
            }
            InsertTreeNode(newRow++, indent, imgType, (LPARAM)newRow);
        }
    }
    pMemoryBlockRef->Release();
}

void CVarList::ExpandLib(string &itemPath, int row, SymbolStore *pSymStore, ItemDesc &itemDesc) {
    Memory &mem = *theApp.GetMemory();
    MemoryBlockRef *pMemoryBlockRef = itemDesc.GetMemoryBlockRef(mem);
    if (!pMemoryBlockRef) {
        return;
    }

    LibType *pLibType = static_cast<LibType*>(pMemoryBlockRef->GetReferenceTypeNode());
    ui32 count = pLibType->GetSubTypeCount();

    UpdateImg(row, 0, IMG_EXPANDED);

    for (ui32 i = 0; i < count; ++i) {
        Type *pSubType = pLibType->GetSubType(i);
        Variable *pVariable = pSubType->GetVariable(0);
        string typeName = pSubType->GetTypeString(pSymStore);
        string name;
        pSymStore->GetSymbolName(pSubType, name);
        name = "function<" + name + ">";
        string subItemPath;
        if (itemPath.length()) {
            subItemPath = itemPath + "_" + to_string(i) + "_0";
        }
        else {
            subItemPath = to_string(i) + "_0";
        }

        //string subItemPath = itemPath + "_" + to_string(i);
        ui32 offset = pMemoryBlockRef->GetOffset() + pVariable->GetAlignedMemberOffset();
        //ui32 offset = pVariable->GetAlignedMemberOffset();
        vector<ItemDesc*>::iterator itDesc = m_ItemDescVector.begin() + row + 1;
//#ifdef _DEBUG
        //MemoryBlockRef *pStackMemoryBlockRef = itemDesc.GetStackMemoryBlockRef(mem);
        m_ItemDescVector.insert(itDesc, new ItemDesc(subItemPath, name, typeName, itemDesc.GetThreadId(), itemDesc.GetStackFrame(), itemDesc.GetVarIdx(), /*offset, */pSubType));
//#else // _DEBUG
//        m_ItemDescVector.insert(itDesc, new ItemDesc(subItemPath, pMemoryBlockRef, offset, pSubType));
//#endif // _DEBUG
        InsertTreeNode(row + i, 0, IMG_NONE, (LPARAM)row);
    }
    pMemoryBlockRef->Release();
}

void CVarList::CollapseTreeNode(LVITEM &lv) {
    // must remove sub items if indent is larger then this item's indent until indent is equal to this item
    UpdateImg(lv.iItem, lv.iIndent, IMG_COLLAPSED);
    LVITEM nextLV = lv;
    ++nextLV.iItem;
    while (GetItem(&nextLV)) {
        if (nextLV.iIndent <= lv.iIndent) {
            break;
        }
        DeleteItem(nextLV.iItem);
        vector<ItemDesc*>::iterator it = m_ItemDescVector.begin() + nextLV.iItem;
        delete *it;
        m_ItemDescVector.erase(it);
    }
}

void CVarList::ClearList() {
    DeleteAllItems();
    for (ui32 i = 0; i < m_ItemDescVector.size(); ++i) {
        delete m_ItemDescVector[i];
    }
    m_ItemDescVector.clear();
}

void CVarList::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    if (pNMItemActivate->iItem == -1) return;

    // TODO: Add your control notification handler code here
    if (pNMItemActivate->iSubItem == 0) {
        LVITEM lv = { 0 };
        lv.iItem = pNMItemActivate->iItem;
        lv.iSubItem = 0;
        WCHAR buffer[50];
        lv.pszText = buffer;
        lv.cchTextMax = sizeof(buffer);
        //lv.pszText = str;
        lv.mask = LVIF_TEXT | LVIF_INDENT | LVIF_IMAGE | LVIF_PARAM;
        //lv.iIndent = indent;
        if (GetItem(&lv)) {
            LONG x0 = lv.iIndent * 16,
                 x1 = x0 + 16;
            if (pNMItemActivate->ptAction.x >= x0 && pNMItemActivate->ptAction.x <= x1) {
                switch (lv.iImage) {
                    case IMG_COLLAPSED:
                        ExpandTreeNode(lv);
                        break;
                    case IMG_EXPANDED:
                        CollapseTreeNode(lv);
                        break;
                    case IMG_NONE:
                    default:
                        break;
                }
            }
        }
    }
    *pResult = 0;
}

void CVarList::SetItemString(LVITEMW &item, _bstr_t &src) {
    ui32 maxCharCount = item.cchTextMax - 4;
    if (maxCharCount < src.length()) {
        memcpy(item.pszText, src.GetBSTR(), maxCharCount * 2);
        memcpy(&item.pszText[maxCharCount], L"...", 8);
    }
    else {
        memcpy(item.pszText, src.GetBSTR(), (src.length() + 1) * 2);
    }
}

void CVarList::OnIdrPopupVarlist()
{
    // TODO: Add your command handler code here
}


void CVarList::OnContextMenu(CWnd* pWnd, CPoint point)
{
    CMenu menu;
    menu.LoadMenu(IDR_POPUP_VARLIST);

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

void CVarList::OnUpdatePopupViewaddress(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(m_pPropWnd->GetViewAddress());
}
void CVarList::OnUpdatePopupViewasbinary(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(m_pPropWnd->GetViewAs() == VIEW_AS_BINARY);
}

void CVarList::OnUpdatePopupViewasdefault(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(m_pPropWnd->GetViewAs() == VIEW_AS_DEFAULT);
}

void CVarList::OnUpdatePopupViewashex(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(m_pPropWnd->GetViewAs() == VIEW_AS_HEX);
}

void CVarList::OnPopupViewasbinary()
{
    m_pPropWnd->SetViewAs(VIEW_AS_BINARY);
    RedrawWindow();
}


void CVarList::OnPopupViewasdefault()
{
    m_pPropWnd->SetViewAs(VIEW_AS_DEFAULT);
    RedrawWindow();
}


void CVarList::OnPopupViewashex()
{
    m_pPropWnd->SetViewAs(VIEW_AS_HEX);
    RedrawWindow();
}

void CVarList::OnPopupViewaddress()
{
    m_pPropWnd->SetViewAddress(!m_pPropWnd->GetViewAddress());
    m_pPropWnd->ToggleAddress();
    RedrawWindow();
}

void CVarList::OnPopupCopy()
{
    int i = GetSelectionMark();
    CString name = GetItemText(i, 0),
        value = GetItemText(i, 1),
        type = GetItemText(i, 2),
        address = GetItemText(i, 3);// +":\r\n";
    string s;// = (LPCSTR)_bstr_t((LPCWSTR)address);
    if (type == "string") {
        s += UnEscapeString(string((LPCSTR)_bstr_t((LPCWSTR)value)));
    }
    else {
        s += (LPCSTR)_bstr_t((LPCWSTR)value);
    }
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, s.length() + 1);
    memcpy(GlobalLock(hMem), s.c_str(), s.length() + 1);
    GlobalUnlock(hMem);
    OpenClipboard();
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
}

string CVarList::EscapeString(string &token) {
    string s;
    for (ui32 i = 0; i < token.size(); ++i) {
        switch (token[i]) {
            case '\a':
                s.push_back('\\');
                s.push_back('a');
                break;
            case '\b':
                s.push_back('\\');
                s.push_back('b');
                break;
            case '\f':
                s.push_back('\\');
                s.push_back('f');
                break;
            case '\n':
                s.push_back('\\');
                s.push_back('n');
                break;
            case '\r':
                s.push_back('\\');
                s.push_back('r');
                break;
            case '\t':
                s.push_back('\\');
                s.push_back('t');
                break;
            case '\v':
                s.push_back('\\');
                s.push_back('v');
                break;
            case '\\':
                s.push_back('\\');
                s.push_back('\\');
                break;
            case '\'':
                s.push_back('\\');
                s.push_back('\'');
                break;
            case '\"':
                s.push_back('\\');
                s.push_back('\"');
                break;
            default:
                s.push_back(token[i]);
                break;
        }
    }
    return s;
}

string CVarList::UnEscapeString(string &token) {
    string s;
    char ch;// = token[0];
    //if ((ch == '\"') || (ch == '\'')) {
        //s.push_back(ch);
    size_t len = token.size() - 1;
        for (ui32 i = 1; i < len; ++i) {
            if (token[i] == '\\') {
                ++i;
                switch (token[i]) {
                    case 'a':
                        ch = 0x07;
                        break;
                    case 'b':
                        ch = 0x08;
                        break;
                    case 'f':
                        ch = 0x0c;
                        break;
                    case 'n':
                        ch = 0x0a;
                        break;
                    case 'r':
                        ch = 0x0d;
                        break;
                    case 't':
                        ch = 0x09;
                        break;
                    case 'v':
                        ch = 0x0b;
                        break;
                    case '\\':
                    case '\'':
                    case '\"':
                        ch = token[i];
                        break;
                    default:
                        break;
                }
            }
            else {
                ch = token[i];
            }
            s.push_back(ch);
        }
        //token = s;
    //}
    return s;
}


void CVarList::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
    // TODO: Add your control notification handler code here

    CMainFrame* pMainFrame = (CMainFrame*)theApp.GetMainWnd();
    switch (pDispInfo->item.iSubItem)
    {
        case 0: {
            vector<ItemDesc*>::iterator it = m_ItemDescVector.begin() + pDispInfo->item.iItem;
            if (it != m_ItemDescVector.end()) {
                ItemDesc *pItemDesc = *it;
                SetItemString(pDispInfo->item, pItemDesc->m_name);
            }
            break;
        }
        case 1: {
            vector<ItemDesc*>::iterator it = m_ItemDescVector.begin() + pDispInfo->item.iItem;
            Memory *pMemory = theApp.GetMemory();
            if ((pMemory != 0) && (it != m_ItemDescVector.end())) {
                string str;
                ItemDesc *pItemDesc = *it;
                try {
                    if (GetParent()->IsWindowEnabled()) {

                        bool locked = false;
                        if (MemoryBlockRef *pMemoryBlockRef = pItemDesc->GetMemoryBlockRef(*pMemory)) {
                            Type *pType = pMemoryBlockRef->GetReferenceTypeNode();
                            locked = pMemoryBlockRef->GetMemoryBlock()->IsLocked();
                            string strAddr;
                            //CPropertiesWnd *pPropWnd = static_cast<CPropertiesWnd*>(GetParent()->GetParent());
                            if (m_pPropWnd->GetViewAddress()) {
                                Value addr;
                                addr.Set((ui32)pMemoryBlockRef->GetMemoryBlock()->GetPtr<void*>(pMemoryBlockRef->GetOffset()));
                                addr.GetHexString(strAddr);
                                pItemDesc->m_address = strAddr.c_str();
                            }

                            switch (pType->GetType()) {
                                case TYPE_ARRAY:
                                case TYPE_AGGREGATE_TYPE:
                                case TYPE_LIB:
                                case TYPE_ERROR_TYPE:
                                    str = "<...>";
                                    break;
                                case TYPE_FUNCTION_REF_TYPE: {
                                    Value val(pMemoryBlockRef, pType);
                                    if (!val.GetHexString(str)) {
                                        str = "<...>";
                                    }
                                    //std::stringstream sstream;
                                    //sstream << "0x" << std::hex << val.GetUI32();
                                    //str = sstream.str();
                                    break;
                                }
                                case TYPE_STRING: {
                                    Value val(pMemoryBlockRef, pType);
                                    str = "\"" + EscapeString(val.GetString()) + "\"";
                                    break;
                                }
                                case TYPE_FLOAT:
                                case TYPE_DOUBLE: {
                                    Value val(pMemoryBlockRef, pType);
                                    str = val.GetString();
                                    break;
                                }
                                default: {
                                    Value val(pMemoryBlockRef, pType);
                                    ui16 ch = -1;
                                    switch (m_pPropWnd->GetViewAs()) {
                                        case VIEW_AS_BINARY:
                                            if (!val.GetBinaryString(str)) {
                                                str = "<...>";
                                            }
                                            break;
                                        case VIEW_AS_HEX:
                                            if (!val.GetHexString(str)) {
                                                str = "<...>";
                                            }
                                            break;
                                        default:
                                            str = val.GetString();
                                            break;
                                    }
                                    switch (pType->GetType()) {
                                        case TYPE_UI8:
                                        case TYPE_I8:
                                            if (char ch = val.GetUI8()) {
                                                str = str + " \'" + ch + "\'";
                                            }
                                            else {
                                                str = str + " \'\\0\'";
                                            }
                                            break;
                                    }

                                    break;
                                }
                            }

                            pMemoryBlockRef->Release();
                        }
                        else {
                            str = "<null>";
                        }
                        if (locked) {
                            str = "LOCKED: " + str;
                        }
                        pItemDesc->m_value = str.c_str();
                    }
                    else {
                        int i =9;
                        i = i + 1;
                    }
                    SetItemString(pDispInfo->item, pItemDesc->m_value);
                }
                catch (...) {
                    pItemDesc->m_value = "INVALID";
                    SetItemString(pDispInfo->item, pItemDesc->m_value);
                    break;
                }
            }
            break;
        }
        case 2: {
            vector<ItemDesc*>::iterator it = m_ItemDescVector.begin() + pDispInfo->item.iItem;
            if (it != m_ItemDescVector.end()) {
                ItemDesc *pItemDesc = *it;
                SetItemString(pDispInfo->item, pItemDesc->m_type);
            }
            break;
        }
        case 3: {
            vector<ItemDesc*>::iterator it = m_ItemDescVector.begin() + pDispInfo->item.iItem;
            if (it != m_ItemDescVector.end()) {
                ItemDesc *pItemDesc = *it;
                SetItemString(pDispInfo->item, pItemDesc->m_address);
            }
            break;
        }
        default:
            break;
    }
    *pResult = 0;
}

/***********************************************************/

CVarList::ItemDesc& CVarList::ItemDesc::operator=(const CVarList::ItemDesc& that) {
    m_name                  = that.m_name;
    m_value                 = that.m_value;
    m_type                  = that.m_type;
    m_pItemType             = that.m_pItemType;
    m_threadId              = that.m_threadId;
    m_stackFrame            = that.m_stackFrame;
    m_varIdx                = that.m_varIdx;
    m_itemPath              = that.m_itemPath;
    m_address               = that.m_address;
//#ifdef _DEBUG
//#endif // _DEBUG
    return *this;
}

//#ifdef _DEBUG

CVarList::ItemDesc::ItemDesc(string& itemPath, string &name, string &type, ThreadId threadId, ui32 stackFrame, ui32 varIdx, Type* pItemType) :
    m_pItemType(pItemType),
    m_threadId(threadId),
    m_stackFrame(stackFrame),
    m_varIdx(varIdx),
    m_itemPath(itemPath),
    m_name(name.c_str()),
    m_value(""),
    m_type(type.c_str()),
    m_address("") {

}

CVarList::ItemDesc::ItemDesc(const ItemDesc& that) : 
    m_pItemType(0),
    m_threadId(INVALID_THREAD_ID),
    m_stackFrame(INVALID_STACK_FRAME_IDX),
    m_varIdx(INVALID_VAR_IDX) {
    *this = that;
}

CVarList::ItemDesc::~ItemDesc() {
}

MemoryBlockRef* CVarList::ItemDesc::GetStackMemoryBlockRef(Memory &mem) {
    StackMemory *pStackMemory = mem.GetThreadMemory(m_threadId);

    MemoryBlockRef *pMemoryBlockRef = pStackMemory->GetMemoryBlockRef(m_varIdx, m_stackFrame);
    return pMemoryBlockRef;
}
// must release reference of returned block!
MemoryBlockRef* CVarList::ItemDesc::GetMemoryBlockRef(Memory &mem) { 
    MemoryBlockRef *pMemoryBlockRef = GetStackMemoryBlockRef(mem);

    ui32 offsetOut = 0;
    MemoryBlock *pMemoryBlock = pMemoryBlockRef->GetReferenceTypeNode()->FindMemoryBlock(m_itemPath.c_str(), pMemoryBlockRef->GetMemoryBlock(), pMemoryBlockRef->GetOffset(), offsetOut, mem);
    if (pMemoryBlock) {
        MemoryBlockRef *pOutMemoryBlockRef = new MemoryBlockRef(m_pItemType, offsetOut, pMemoryBlock);
        pMemoryBlock->Release();
        return pOutMemoryBlockRef;
    }
    return 0;
}

/*****************************************/

IMPLEMENT_DYNCREATE(CSplitterView, CView);

BEGIN_MESSAGE_MAP(CSplitterView, CView)
    ON_WM_SIZE()
END_MESSAGE_MAP()


void CSplitterView::OnDraw(CDC* pDC) {
    CView::OnDraw(pDC);
}

void CSplitterView::OnSize(UINT nType, int cx, int cy) {
    
    CView::OnSize(nType, cx, cy);
    if (m_pChild) {
        m_pChild->SetWindowPos(NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
    }
}

/*********************************************/

BEGIN_MESSAGE_MAP(CThreadList, CListCtrl)
    ON_NOTIFY_REFLECT(NM_CLICK, &CThreadList::OnNMClick)
    ON_NOTIFY_REFLECT(LVN_GETDISPINFO, &CThreadList::OnLvnGetdispinfo)
END_MESSAGE_MAP()

void CThreadList::Init() {
    InsertColumn(0, L"Thread Id");
    SetColumnWidth(0, 80);

    InsertColumn(1, L"Status");
    SetColumnWidth(1, 60);

    InsertColumn(2, L"Code Location");
    SetColumnWidth(2, 160);

    InsertColumn(3, L"Line");
    SetColumnWidth(3, 40);

    InsertColumn(4, L"File Path");
    SetColumnWidth(4, 300);
}
// columns: imgActiveMarker, threadId, status, CurrentStackSymbol
void CThreadList::Add(ThreadId threadId) {
    int pos = GetItemCount();
    
    LVITEM lv;
    lv.iItem = pos;
    lv.iSubItem = 0;
    lv.pszText = LPSTR_TEXTCALLBACK;
    lv.mask = LVIF_TEXT | LVIF_INDENT | LVIF_PARAM;
    lv.iIndent = 1;
    lv.iImage = 2;
    lv.mask |= LVIF_IMAGE;

    //if (col == 0) {
    lv.lParam = (LPARAM)threadId;
    InsertItem(&lv);
    //}
    //else {
    //    SetItem(&lv);
    //}
}

void CThreadList::Remove(ThreadId threadId) {
    int count = GetItemCount();
    //CString strThreadId = _bstr_t(_variant_t(threadId));
    for (int i = 0; i < count; ++i) {
        if (GetItemData(i) == threadId) {
            DeleteItem(i);
            break;
        }
    }
}

void CThreadList::SetBlocked(ThreadId threadId, bool blocked) {
    int count = GetItemCount();
    LVITEM lv;
    lv.mask = LVIF_TEXT;
    lv.iSubItem = 1;
    for (int i = 0; i < count; ++i) {
        if (GetItemData(i) == threadId) {
            lv.iItem = i;
            lv.pszText = blocked ? L"blocked" : L"";
            SetItem(&lv);
            if (blocked) {
                TODO("Refactor it!");
                if (Memory *pMemory = theApp.GetMemory()) {
                    //SymbolStore *pSymbolStore = theApp.GetSymbolStore();
                    StackMemory *pStackMemory = pMemory->GetThreadMemory(threadId);
                    ui32 frameIdx = pStackMemory->GetFrameIdx();
                    string fooType, location, fileName;
                    ui32 line, pos;
                    if (pStackMemory->GetFrameFunctionAsStrings(frameIdx, fooType, location, fileName, line, pos)) {
                        SetItemText(i, 2, _bstr_t(fooType.c_str()));
                        SetItemText(i, 3, _bstr_t(_variant_t(line)));
                        SetItemText(i, 4, _bstr_t(fileName.c_str()));
                    }
                }
            }
            break;
        }
    }
}

void CThreadList::SetActive(ThreadId threadId) {
    int count = GetItemCount();
    LVITEM lv;
    lv.iSubItem = 0;
    lv.mask = LVIF_IMAGE;
    for (int i = 0; i < count; ++i) {
        lv.iItem = i;
        lv.iImage = (GetItemData(i) == threadId) ? 3 : 2;
        SetItem(&lv);
    }
}

void CThreadList::SetSymbol(ThreadId threadId, _bstr_t location, _bstr_t line, _bstr_t file) {
    int count = GetItemCount();
    LVITEM lv;
    lv.mask = LVIF_TEXT;
    for (int i = 0; i < count; ++i) {
        if (GetItemData(i) == threadId) {
            lv.iItem = i;
            lv.iSubItem = 2;
            lv.pszText = (BSTR)location;
            SetItem(&lv);
            lv.iSubItem = 3;
            lv.pszText = (BSTR)line;
            SetItem(&lv);
            lv.iSubItem = 4;
            lv.pszText = (BSTR)file;
            SetItem(&lv);
        }
    }
    SetActive(threadId);
}

//void CThreadList::AddData(int row, int col, LPWSTR str, ui32 stackIdx) {
//
//    LVITEM lv;
//    lv.iItem = row;
//    lv.iSubItem = col;
//    lv.pszText = str;
//    lv.mask = LVIF_TEXT;
//    if (col == 0) {
//        lv.mask |= LVIF_PARAM;
//        lv.lParam = (LPARAM)stackIdx;
//        InsertItem(&lv);
//    }
//    else {
//        SetItem(&lv);
//    }
//
//}

void CThreadList::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
    // TODO: Add your control notification handler code here

    CMainFrame* pMainFrame = (CMainFrame*)theApp.GetMainWnd();
    switch (pDispInfo->item.iSubItem) {
        case 0: {// thread  id and marker
            _bstr_t v(_variant_t((ui32)pDispInfo->item.lParam));// (_variant_t(GetItemData()));
            memcpy(pDispInfo->item.pszText, (BSTR)v, (v.length() + 1) * 2);
            break;
        }
        case 1: // status
            break;
        case 2: // symbol on the stack
            break;
        default:
            break;
    }
    
    *pResult = 0;
}

//ui32 CStackList::GetThreadId() {
//    CMFCTabCtrl *pTabs = (CMFCTabCtrl*)GetParent()->GetParent()->GetParent();
//    CString label;
//    pTabs->GetTabLabel(pTabs->GetActiveTab(), label);
//    label.Replace(L"Thread: ", L"");
//    string str = _bstr_t(label);
//    return stoi(str);
//}
//
void CThreadList::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult) {
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    if (pNMItemActivate->iItem == -1) return;
    LVITEM lv = { 0 };
    ThreadId threadId = GetItemData(pNMItemActivate->iItem);
    if (Memory *pMemory = theApp.GetMemory()) {
        StackMemory *pStackMemory = pMemory->GetThreadMemory(threadId);
        ui32 frameIdx = pStackMemory->GetFrameIdx();
        Node *pFunctionNode = 0, *pCurrentNode = 0;
        Scope *pFunctionScope = 0;
        pStackMemory->GetFrameFunctionAndScope(frameIdx, pFunctionNode, pFunctionScope, pCurrentNode);
        if (!pFunctionScope && frameIdx) {
            pStackMemory->GetFrameFunctionAndScope(frameIdx - 1, pFunctionNode, pFunctionScope, pCurrentNode);
        }
        ui32 fileId = pCurrentNode->GetFileId();
        ui32 line = pCurrentNode->GetLine();
        if (CScriptView *pScriptView = ((CMainFrame*)theApp.m_pMainWnd)->GetScriptView(fileId)) {
            pScriptView->HighlightLine(HIGHLIGHT_STACK_CALL, fileId, line - START_POSITION_OFFSET, INVALID_THREAD_ID, 0);
        }
    }

    *pResult = 0;
}

/*********************************************/

BEGIN_MESSAGE_MAP(CStackList, CListCtrl)
    ON_NOTIFY_REFLECT(NM_CLICK, &CStackList::OnNMClick)
END_MESSAGE_MAP()

void CStackList::AddData(int row, int col, LPWSTR str, ui32 stackIdx) {

    LVITEM lv;
    lv.iItem = row;
    lv.iSubItem = col;
    lv.pszText = str;
    lv.mask = LVIF_TEXT;
    if (col == 0) {
        lv.mask |= LVIF_PARAM;
        lv.lParam = (LPARAM)stackIdx;
        InsertItem(&lv);
    }
    else {
        SetItem(&lv);
    }

}

ui32 CStackList::GetThreadId() {
    CMFCTabCtrl *pTabs = (CMFCTabCtrl*)GetParent()->GetParent()->GetParent();
    CString label;
    pTabs->GetTabLabel(pTabs->GetActiveTab(), label);
    label.Replace(L"Thread: ", L"");
    string str = _bstr_t(label);
    return stoi(str);
}

void CStackList::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult) {
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    if (pNMItemActivate->iItem == -1) return;

    // TODO: Add your control notification handler code here
    //ui32 stackIdx = (ui32)GetItemData(pNMItemActivate->iItem);
    LVITEM lv = { 0 };
    lv.iItem = pNMItemActivate->iItem;
    lv.iSubItem = 2;
    WCHAR buffer[50];
    buffer[0] = 0;
    lv.pszText = buffer;
    lv.cchTextMax = sizeof(buffer);
    //lv.pszText = str;
    lv.mask = LVIF_TEXT | LVIF_PARAM;
    if (GetItem(&lv)) {
        ui32 stackIdx = lv.lParam;
        ui32 line = stoul(string(_bstr_t(buffer)));
        ThreadId threadId = GetThreadId();
        Memory *pMemory = theApp.GetMemory();
        Node *pFunctionNode = 0, *pCurrentNode = 0;
        Scope *pFunctionScope = 0;
        StackMemory *pStackMemory = pMemory->GetThreadMemory(threadId);
        pStackMemory->GetFrameFunctionAndScope(stackIdx, pFunctionNode, pFunctionScope, pCurrentNode);
        ui32 fileId = pCurrentNode->GetFileId();
        if (CScriptView *pScriptView = ((CMainFrame*)theApp.m_pMainWnd)->GetScriptView(fileId)) {
            pScriptView->HighlightLine(HIGHLIGHT_STACK_CALL, fileId, line - START_POSITION_OFFSET, INVALID_THREAD_ID, 0);

            ((CMainFrame*)theApp.m_pMainWnd)->m_wndProperties.UpdateActiveVarList(threadId, stackIdx, pFunctionScope);
        }
    }
    *pResult = 0;
}

/*********************************************/

BEGIN_MESSAGE_MAP(CBreakpointList, CListCtrl)
    ON_NOTIFY_REFLECT(NM_CLICK, &CBreakpointList::OnNMClick)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_DELETE_BREAKPOINT, OnDeleteBreakpoint)
END_MESSAGE_MAP()

CBreakpointList::~CBreakpointList() {
}

void CBreakpointList::AddBreakpoint(_bstr_t &line, _bstr_t &fileName, _bstr_t &path) {
    int count = GetItemCount();
    AddData(count, 0, line, 0);
    AddData(count, 1, fileName, 0);
    AddData(count, 2, path, 0);
}

void CBreakpointList::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult) {
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    if (pNMItemActivate->iItem == -1) return;

    CString strLine = GetItemText(pNMItemActivate->iItem, 0);
    CString strFileName = GetItemText(pNMItemActivate->iItem, 1);
    CString strPath = GetItemText(pNMItemActivate->iItem, 2);
    if (strLine.GetLength() && strFileName.GetLength() && strPath.GetLength()) {
        strPath += L"\\";
        strPath += strFileName;

        if (CScriptView *pScriptView = ((CMainFrame*)theApp.m_pMainWnd)->GetScriptView(strPath)) {
            ui32 line = stoul(string(_bstr_t(strLine)));
            pScriptView->HighlightLine(HIGHLIGHT_BREAKPOINT, -1, line - START_POSITION_OFFSET, INVALID_THREAD_ID, 0);
        }
    }

    *pResult = 0;
}

void CBreakpointList::AddData(int row, int col, LPWSTR str, int imageIdx) {
    LVITEM lv;
    lv.iItem = row;
    lv.iSubItem = col;
    lv.pszText = str;
    lv.mask = LVIF_TEXT;
    if (col == 0) {
        lv.iImage = imageIdx;
        lv.mask |= LVIF_IMAGE;
        InsertItem(&lv);
    }
    else {
        SetItem(&lv);
    }
}

void CBreakpointList::Load() {
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMDocument> xmlDoc = theApp.GetDebuggerData();
    CComPtr<IXMLDOMNodeList> spBreakpointList;
    hr = xmlDoc->selectNodes(_bstr_t("//root/breakpoints/path"), &spBreakpointList);
    if (FAILED(hr)) return;
    /*
    <root>
        <breakpoints>
            <path name="file_path">
                <file name="file_name">
                    <bp>line_num</bp>
                    ...
                </file>
                ...
            </path>
            ...
        </breakpoints>
        <script_path></script_path>
        <var_list address="false" viewas="0"></var_list>
        ...
    </root>
    */
    long count = 0;
    spBreakpointList->get_length(&count);
    for (long i = 0; i < count; ++i) {
        _variant_t filePath, fileName;
        CComPtr<IXMLDOMNode> spBreakpointNode;
        hr = spBreakpointList->get_item(i, &spBreakpointNode);
        if (!spBreakpointNode) continue;

        CComPtr<IXMLDOMNode> spAttribute;
        hr = spBreakpointNode->selectSingleNode(_bstr_t("@name"), &spAttribute);
        if (spAttribute) {
            hr = spAttribute->get_nodeValue(&filePath);
        }
        CComPtr<IXMLDOMNodeList> spFileList;
        hr = spBreakpointNode->selectNodes(_bstr_t("file"), &spFileList);
        if (!spFileList) continue;
        
        long fileCount = 0;
        spFileList->get_length(&fileCount);
        for (long k = 0; k < fileCount; ++k) {
            CComPtr<IXMLDOMNode> spFileNode;
            hr = spFileList->get_item(k, &spFileNode);
            if (!spFileNode) continue;
                
            spAttribute = NULL;
            hr = spFileNode->selectSingleNode(_bstr_t("@name"), &spAttribute);
            if (spAttribute) {
                hr = spAttribute->get_nodeValue(&fileName);
            }
            CComPtr<IXMLDOMNodeList> spBpList;
            hr = spFileNode->selectNodes(_bstr_t("bp"), &spBpList);
            if (!spBpList)  continue; 
            
            long bpCount = 0;
            spBpList->get_length(&bpCount);
            for (long m = 0; m < bpCount; ++m) {
                CComPtr<IXMLDOMNode> spBp;
                _bstr_t bstrLine;
                hr = spBpList->get_item(m, &spBp);
                if (spBp) {
                    hr = spBp->get_text(&bstrLine.GetBSTR());
                }
                _bstr_t bstrFilePath(filePath), bstrFileName(fileName);
                if (bstrFilePath.length() && bstrFileName.length() && bstrLine.length()) {
                    AddBreakpoint(bstrLine, bstrFileName, bstrFilePath/*, BREAKPOINT_INVALID*/);
                }
            }
        }
    }
}

void CBreakpointList::Save() {
    if (!UpdateDebuggerData()) return;

    theApp.SaveDebuggerData();
}

bool CBreakpointList::UpdateDebuggerData() {
    CComPtr<IXMLDOMElement> spBreakpoints, spPath, spFile;
    CComPtr<IXMLDOMDocument> xmlDoc = theApp.GetDebuggerData();
    HRESULT hr = xmlDoc->createElement(_bstr_t("breakpoints"), &spBreakpoints);
    if (!spBreakpoints) return false;
    hr = xmlDoc->createElement(_bstr_t("path"), &spPath);
    if (!spPath) return false;

    CString strCurPath, strCurFileName;
    int count = GetItemCount();
    for (int i = 0; i < count; ++i) {
        CString strLine = GetItemText(i, 0);
        CString strFileName = GetItemText(i, 1);
        CString strPath = GetItemText(i, 2);
        if (strPath != strCurPath) {
            spPath.Release();
            strCurPath = strPath;
            hr = xmlDoc->createElement(_bstr_t("path"), &spPath);
            if (!spPath) return false;
            spPath->setAttribute(_bstr_t("name"), _variant_t(_bstr_t(strCurPath)));
            hr = spBreakpoints->appendChild(spPath, 0);
            if (FAILED(hr)) return false;
        }
        if (strFileName != strCurFileName) {
            spFile.Release();
            strCurFileName = strFileName;
            hr = xmlDoc->createElement(_bstr_t("file"), &spFile);
            if (!spFile) return false;
            spFile->setAttribute(_bstr_t("name"), _variant_t(_bstr_t(strCurFileName)));
            hr = spPath->appendChild(spFile, 0);
            if (FAILED(hr)) return false;
        }

        CComPtr<IXMLDOMElement> spBp;
        hr = xmlDoc->createElement(_bstr_t("bp"), &spBp);
        if (!spBp) return false;

        hr = spBp->put_text(_bstr_t(strLine));
        if (FAILED(hr)) return false;
        
        hr = spFile->appendChild(spBp, 0);
        if (FAILED(hr)) return false;
    }

    CComPtr<IXMLDOMNode>    spOldBreakpoints;
    hr = xmlDoc->selectSingleNode(_bstr_t("//root/breakpoints"), &spOldBreakpoints);
    if (FAILED(hr)) return false;

    CComPtr<IXMLDOMNode>    spRoot;
    hr = xmlDoc->selectSingleNode(_bstr_t("//root"), &spRoot);
    if (FAILED(hr)) return false;

    hr = spRoot->replaceChild(spBreakpoints, spOldBreakpoints, 0);
    if (FAILED(hr)) return false;
    return true;
}

BREAKPOINT_MARKER CBreakpointList::InsertBreakpoint(_bstr_t &path, _bstr_t &fileName, _bstr_t &line) {
    Debugger *pDebugger = theApp.GetDebugger();
    int image = 0;
    if (SymbolStore *pSymStore = pDebugger ? pDebugger->GetSymbolStore() : 0) {
        _bstr_t scriptPath = path + "\\" + fileName;
        ui32 fileId = pSymStore->GetFileId((LPCSTR)scriptPath);
        ui32 _line = (ui32)_variant_t(_bstr_t(line)) - START_POSITION_OFFSET;
        image = pDebugger->SetBreakpoint(fileId, _line, true);
    }

    int i = FindUpper(line, fileName, path, GetItemCount());
    if (i != 0) {
        --i;
        CString txt = GetItemText(i, 2);
        txt += GetItemText(i, 1);
        txt += GetItemText(i, 0);
        if (txt == CString((LPCWSTR)(path + fileName + line))) {
            LVITEM lv;
            lv.iItem = i;
            lv.iSubItem = 0;
            lv.iImage = image; // always update as an invalid breakpoint, then it gets updated
            lv.mask = LVIF_IMAGE;// | LVIF_INDENT;
            SetItem(&lv);
            return image ? BREAKPOINT_VALID : BREAKPOINT_INVALID;
        }
        ++i;
    }

    AddData(i, 0, line, image);
    AddData(i, 1, fileName, 0);
    AddData(i, 2, path, 0);

    return image ? BREAKPOINT_VALID : BREAKPOINT_INVALID;
}

void CBreakpointList::DeleteBreakpoint(_bstr_t &path, _bstr_t &fileName, _bstr_t &line) {
    Debugger *pDebugger = theApp.GetDebugger();
    bool isSet = false;
    _bstr_t scriptPath = path + "\\" + fileName;
    ui32 _line = (ui32)_variant_t(_bstr_t(line)) - START_POSITION_OFFSET;
    if (SymbolStore *pSymStore = pDebugger ? pDebugger->GetSymbolStore() : 0) {
        ui32 fileId = pSymStore->GetFileId((LPCSTR)scriptPath);
        isSet = pDebugger->SetBreakpoint(fileId, _line, false);
    }
    if (!isSet) {
        if (CScriptView *pScriptView = ((CMainFrame*)theApp.m_pMainWnd)->GetScriptView(CString((LPWSTR)scriptPath))) {
            pScriptView->SetBreakPointMarker(_line, BREAKPOINT_NONE);
        }
    }
    int i = Find(line, fileName, path);
    if (i != -1) {
        DeleteItem(i);
    }
}

void CBreakpointList::InvalidateAllBreakpoints() {
    int count = GetItemCount();
    for (int i = 0; i < count; ++i) {
        LVITEM lv;
        lv.iItem = i;
        lv.iSubItem = 0;
        lv.iImage = 0;
        lv.mask = LVIF_IMAGE;
        SetItem(&lv);
    }
}

void CBreakpointList::MarkAllValidBreakpoints(_bstr_t &path, _bstr_t &fileName) {
    int count = GetItemCount();
    Debugger *pDebugger = theApp.GetDebugger();
    for (int i = 0; i < count; ++i) {
        CString strPath = GetItemText(i, 2);
        if (path == _bstr_t(strPath)) {
            CString strFileName = GetItemText(i, 1);
            if (fileName == _bstr_t(strFileName)) {
                bool image = 0;
                if (pDebugger) {
                    CString strLine = GetItemText(i, 0);
                    if (SymbolStore *pSymStore = pDebugger->GetSymbolStore()) {
                        _bstr_t scriptPath = path + "\\" + fileName;
                        ui32 fileId = pSymStore->GetFileId((LPCSTR)scriptPath);
                        ui32 line = (ui32)_variant_t(_bstr_t(strLine)) - START_POSITION_OFFSET;
                        image = pDebugger->SetBreakpoint(fileId, line, true);
                    }
                }
                LVITEM lv;
                lv.iItem = i;
                lv.iSubItem = 0;
                lv.iImage = image;
                lv.mask = LVIF_IMAGE;
                SetItem(&lv);
            }
        }
    }

}

int CBreakpointList::FindUpper(_bstr_t &line, _bstr_t &fileName, _bstr_t &path, int count) {
    CString Name((LPCWSTR)fileName), Path((LPCWSTR)path);
    ui32 lineIn = stoul((LPCSTR)line);
    int first = 0;

    while (0 < count) {	// divide and conquer, find half that contains answer
        int count2 = count / 2;
        int mid = first;
        mid += count2;

        int res2 = Path.Compare(GetItemText(mid, 2));

        if (res2 < 0) {
            count = count2;
        } 
        else if (res2 > 0) {
            first = ++mid;
            count -= count2 + 1;
        }
        else {
            int res1 = Name.Compare(GetItemText(mid, 1));
            if (res1 < 0) {
                count = count2;
            }
            else if (res1 > 0) {
                first = ++mid;
                count -= count2 + 1;
            }
            else {
                int res0 = lineIn - stoul((LPCSTR)_bstr_t(GetItemText(mid, 0)));
                if (res0 < 0) {
                    count = count2;
                }
                else {
                    first = ++mid;
                    count -= count2 + 1;
                }
            }
        }
    }
    return first;
}

int CBreakpointList::Find(_bstr_t &line, _bstr_t &fileName, _bstr_t &path) {
    int count = GetItemCount();
    int i = FindUpper(line, fileName, path, count);
    if (i == 0) return -1;
    --i;
    CString txt = GetItemText(i, 2);
    txt += GetItemText(i, 1);
    txt += GetItemText(i, 0);
    if (txt == CString((LPCWSTR)(path + fileName + line))) {
        return i;
    }
    return -1;
}

void CBreakpointList::GetBreakpoints(_bstr_t &path, _bstr_t &fileName, vector<BREAKPOINT_INFO> &breakpoints) {
    breakpoints.clear();
    int count = GetItemCount();
    bool stopPath = false, stopName = false;
    for (int i = 0; i < count; ++i) {
        CString txt = GetItemText(i, 2);
        if (_bstr_t(txt) == path) {
            stopPath = true;
            txt = GetItemText(i, 1);
            if (_bstr_t(txt) == fileName) {
                stopName = true;
                txt = GetItemText(i, 0);
                LVITEM lv = { 0 };
                lv.iItem = i;
                lv.mask = LVIF_IMAGE;
                if (GetItem(&lv)) {
                    breakpoints.push_back(BREAKPOINT_INFO(stoul((LPCSTR)_bstr_t(txt)), lv.iImage ? BREAKPOINT_VALID : BREAKPOINT_INVALID));
                }
            }
            else if (stopName) {
                break;
            }
        }
        else if (stopPath) {
            break;
        }
    }
}

void CBreakpointList::PropagateBreakpoints() {
    Debugger    *pDebugger = theApp.GetDebugger();
    if (!pDebugger) {
        return;
    }
    SymbolStore *pSymStore = pDebugger->GetSymbolStore();
    if (pSymStore) {
        int count = GetItemCount();
        for (int i = 0; i < count; ++i) {
            CString scriptPath = GetItemText(i, 2) + "\\" + GetItemText(i, 1);
            CString txtLine = GetItemText(i, 0);
            ui32 fileId = pSymStore->GetFileId((LPCSTR)_bstr_t(scriptPath));
            ui32 line = (ui32)_variant_t(_bstr_t(txtLine)) - START_POSITION_OFFSET;
            if (pDebugger->SetBreakpoint(fileId, line, true)) {
                LVITEM lv;
                lv.iItem = i;
                lv.iSubItem = 0;
                lv.iImage = 1;
                lv.mask = LVIF_IMAGE;
                SetItem(&lv);
            }
        }
    }
}

void CBreakpointList::UpdateBreakpointLine(_bstr_t &path, _bstr_t &fileName, _bstr_t &oldLine, _bstr_t &newLine) {
    int i = Find(oldLine, fileName, path);
    if (i != -1) {
        SetItemText(i, 0, newLine);
    }
}

void CBreakpointList::RestoreBreakpopints(_bstr_t &path, _bstr_t &fileName, unordered_set<ui32> &breakpointLines) {
    vector<_bstr_t> vecDeleteBreakpoints;
    int count = GetItemCount();
    for (int i = 0; i < count; ++i) {
        if (_wcsicmp(GetItemText(i, 2), path) == 0) {
            if (_wcsicmp(GetItemText(i, 1), fileName) == 0) {
                _variant_t varLine = _bstr_t(GetItemText(i, 0));
                
                unordered_set<ui32>::iterator it = breakpointLines.find((ui32)varLine);
                if (it != breakpointLines.end()) {
                    breakpointLines.erase(it);
                }
                else {
                    vecDeleteBreakpoints.push_back(varLine);
                }
            }
        }
    }
    //delete breakpoints
    for (ui32 i = 0; i < vecDeleteBreakpoints.size(); ++i) {
        DeleteBreakpoint(path, fileName, vecDeleteBreakpoints[i]);
    }
    //now restore breakpoints
    for (unordered_set<ui32>::iterator it = breakpointLines.begin(); it != breakpointLines.end(); ++it) {
        InsertBreakpoint(path, fileName, _bstr_t(_variant_t(*it)));
    }
    breakpointLines.clear();
}

void CPropertiesWnd::OnClose()
{
    // TODO: Add your message handler code here and/or call default
    CDockablePane::OnClose();
}

void CPropertiesWnd::LoadData() {
    CComPtr<IXMLDOMDocument> xmlDoc = theApp.GetDebuggerData();
    CComPtr<IXMLDOMNode> spAttribute;
    HRESULT hr = xmlDoc->selectSingleNode(_bstr_t("//root/var_list/@address"), &spAttribute);
    if (spAttribute) {
        _variant_t var;
        hr = spAttribute->get_nodeValue(&var);
        m_viewAddress = var;
    }
    spAttribute = 0;

    hr = xmlDoc->selectSingleNode(_bstr_t("//root/var_list/@viewas"), &spAttribute);
    if (spAttribute) {
        _variant_t var;
        hr = spAttribute->get_nodeValue(&var);
        m_viewAsType = (CVarList::VIEW_AS_TYPE)(long)var;
    }
}

void CPropertiesWnd::SaveData() {
    CComPtr<IXMLDOMDocument> xmlDoc = theApp.GetDebuggerData();
    CComPtr<IXMLDOMNode> spAttribute;
    HRESULT hr = xmlDoc->selectSingleNode(_bstr_t("//root/var_list/@address"), &spAttribute);
    if (spAttribute) {
        hr = spAttribute->put_nodeValue(_variant_t(m_viewAddress));
    }
    spAttribute = 0;

    hr = xmlDoc->selectSingleNode(_bstr_t("//root/var_list/@viewas"), &spAttribute);
    if (spAttribute) {
        hr = spAttribute->put_nodeValue(_variant_t(m_viewAsType));
    }

    m_wndBreakpointList.Save();
}

void CPropertiesWnd::OnDestroy() {
    SaveData();
    CDockablePane::OnDestroy();
}

/***********************************************************/
//
//BEGIN_MESSAGE_MAP(CWatchList, CVarList)
//    ON_NOTIFY_REFLECT(NM_DBLCLK, &CWatchList::OnNMDblclk)
//    ON_NOTIFY_REFLECT(LVN_GETDISPINFO, &CWatchList::OnLvnGetdispinfo)
//    ON_NOTIFY_REFLECT(LVN_SETDISPINFO, &CWatchList::OnLvnSetdispinfo)
//    ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, &CWatchList::OnLvnEndlabeledit)
//END_MESSAGE_MAP()
//
//
//void CWatchList::Load() {
//    HRESULT hr = S_OK;
//    CComPtr<IXMLDOMDocument> xmlDoc = theApp.GetDebuggerData();
//    CComPtr<IXMLDOMNodeList> spWatchList;
//    hr = xmlDoc->selectNodes(_bstr_t("//root/watch/name"), &spWatchList);
//    if (FAILED(hr)) return;
//
//    long count = 0;
//    spWatchList->get_length(&count);
//    for (long i = 0; i < count; ++i) {
//        _bstr_t name;
//        CComPtr<IXMLDOMNode> spNameNode;
//        hr = spWatchList->get_item(i, &spNameNode);
//        if (!spNameNode) continue;
//
//        hr = spNameNode->get_text(&name.GetBSTR());
//
//        AddSimpleNode(CString((LPWSTR)name), 0, CString());
//    }
//}
//
//void CWatchList::Save() {
//    if (!UpdateDebuggerData()) return;
//
//    theApp.SaveDebuggerData();
//}
//
//bool CWatchList::UpdateDebuggerData() {
//    CComPtr<IXMLDOMDocument> xmlDoc = theApp.GetDebuggerData();
//    CComPtr<IXMLDOMNode>     spWatch;
//    HRESULT hr = xmlDoc->selectSingleNode(_bstr_t("//root/watch"), &spWatch);
//    if (!spWatch) return false;
//
//    int count = GetItemCount();
//    for (int i = 0; i < count; ++i) {
//        CString strName = GetItemText(i, 0);
//        CComPtr<IXMLDOMElement> spName;
//        hr = xmlDoc->createElement(_bstr_t("name"), &spName);
//        if (!spName) continue;
//        
//        hr = spName->put_text(_bstr_t(strName));
//        if (FAILED(hr)) continue;
//
//        hr = spWatch->appendChild(spName, 0);
//    }
//    return true;
//}
//
//void CWatchList::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
//{
//    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
//
//    int idx = pNMItemActivate->iItem;
//    if (idx == -1) {
//        idx = GetItemCount();
//        RECT rect;
//        GetItemRect(idx - 1, &rect, LVIR_BOUNDS);
//        if (!((rect.bottom < pNMItemActivate->ptAction.y) && (pNMItemActivate->ptAction.y < (2 * rect.bottom - rect.top)))) {
//            *pResult = 0;
//            return;
//        }
//        CString str = GetItemText(idx - 1, 0);
//        if (str.GetLength()) {
//            AddSimpleNode(CString(""), (LPARAM)0, CString());
//        }
//        else {
//            --idx;
//            GetItemRect(idx, &rect, LVIR_BOUNDS);
//            if (!((rect.bottom < pNMItemActivate->ptAction.y) && (pNMItemActivate->ptAction.y < rect.top))) {
//                *pResult = 0;
//                return;
//            }
//        }
//    }
//    EditLabel(idx);
//    SetItemState(idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
//    *pResult = 0;
//}
//
//
//void CWatchList::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
//{
//    NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
//    // TODO: Add your control notification handler code here
//    *pResult = 0;
//}
//
//
//void CWatchList::OnLvnSetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
//{
//    NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
//    // TODO: Add your control notification handler code here
//    *pResult = 0;
//}
//
//
//void CWatchList::OnLvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult)
//{
//    NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
//    if (!pDispInfo->item.pszText || (0 == *pDispInfo->item.pszText)) {
//        if (GetItemCount() > 1) {
//            DeleteItem(pDispInfo->item.iItem);
//        }
//    }
//    else {
//        SetItem(&pDispInfo->item);
//    }
//    *pResult = 0;
//}
//
//void CWatchList::GetAllVariables(unordered_map<string, ui32> &vars) {
//    int count = GetItemCount() - 1;
//    for (int i = 0; i < count; ++i) {
//        CString strName = GetItemText(i, 0);
//        vars.insert(pair<string, ui32>(string(_bstr_t(strName)), i));
//    }
//}
//
//void CWatchList::LoadWatchList() {
//    Memory *pMemory = theApp.GetMemory();
//    vector<ThreadId> threadIds;
//    unordered_map<string, ui32> vars;
//    GetAllVariables(vars);
//    pMemory->GetThreadIds(threadIds);
//    for (vector<ThreadId>::iterator it = threadIds.begin(); it != threadIds.end(); ++it) {
//        StackMemory *pStackMemory = pMemory->GetThreadMemory(*it);
//        ui32 topFrameIdx = pStackMemory->GetFrameIndex();
//        if (topFrameIdx || pStackMemory->IsMainThread()) {
//            ui32 globalBit = (topFrameIdx == 0) ? MEMORY_BLOCK_BIT_GLOBAL : 0;
//            if (SymbolStore *pSymStore = theApp.GetSymbolStore()) {
//                vector<LOCAL_VAR_DESCRIPTOR> &locaVarDesc = m_pScope->GetLocalVarDescriptors();
//                int i = 0;
//                //_bstr_t itemPath;
//                for (vector<LOCAL_VAR_DESCRIPTOR>::iterator it = locaVarDesc.begin(); it != locaVarDesc.end(); ++it, ++i) {
//                    string varName, typeName;
//                    pSymStore->GetSymbolName(it->m_desc.m_symId, varName);
//                    unordered_map<string, ui32>::iterator res = vars.find(varName);
//                    if (!(res != vars.end())) continue;
//
//                    Type *pType = (*it);
//                    typeName = CPropertiesWnd::GetTypeString(pSymStore, pType);
//                    MemoryBlockRef *pMemoryBlockRef = pStackMemory->GetMemoryBlockRef(i | globalBit);
//                    pair<unordered_map<string, ItemDesc>::iterator, bool> item = m_varItems.insert(pair<string, ItemDesc>(to_string(i), ItemDesc(pMemoryBlockRef->GetMemoryBlock(), pMemoryBlockRef->GetOffset(), pType, false)));
//
//                    switch (pType->GetType()) {
//                        case TYPE_ARRAY:
//                        case TYPE_AGGREGATE_TYPE:
//                        case TYPE_LIB:
//                        case TYPE_ERROR_TYPE:
//                            AddTreeNode(CString(varName.c_str()), (LPARAM)&item.first->first, CString(typeName.c_str()));
//                            break;
//                        default:
//                            AddSimpleNode(CString(varName.c_str()), (LPARAM)&item.first->first, CString(typeName.c_str()));
//                            break;
//                    }
//                }
//            }
//        }
//    }
//}


void CBreakpointList::OnContextMenu(CWnd* pWnd, CPoint point)
{
    CMenu menu;
    menu.LoadMenu(IDR_POPUP_BREAKPOINTS);

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


void CBreakpointList::OnDeleteBreakpoint()
{
    int i = GetSelectionMark();
    CString path = GetItemText(i, 2),
            script = GetItemText(i, 1),
            line = GetItemText(i, 0);
    DeleteBreakpoint(_bstr_t(path), _bstr_t(script), _bstr_t(line));
}


//void CStackList::OnSize(UINT nType, int cx, int cy)
//{
//    CListCtrl::OnSize(nType, cx - 10, cy - 10);
//    //SetWindowPos(0, 0, 0, cx - 20, cy - 20, nType);
//    // TODO: Add your message handler code here
//}
//BEGIN_MESSAGE_MAP(CTabs, CMFCTabCtrl)
//    ON_WM_SETFOCUS()
//    ON_REGISTERED_MESSAGE(AFX_WM_CHANGE_ACTIVE_TAB, OnTabSetActive)
//END_MESSAGE_MAP()
//
//
//void CTabs::OnSetFocus(CWnd* pOldWnd)
//{
//    CMFCTabCtrl::OnSetFocus(pOldWnd);
//
//    // TODO: Add your message handler code here
//}
//LRESULT CTabs::OnTabSetActive(WPARAM wParam, LPARAM lParam)
//{
//    const int iActiveTab = (int)wParam;
//    CString title;
//    CMFCTabCtrl::GetTabLabel(iActiveTab, title);
//    static_cast<CPropertiesWnd*>(GetParent())->SetWindowText(title);
//    //int iCheckActiveTab = CMFCTabCtrl::GetActiveTab(); //CMFCTabCtrl m_wndTabs;
//    //CMFCTabCtrl::SetActiveTab(iActiveTab); //good idea to also add this depending on usage.
//    return 0;
//}