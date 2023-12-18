
#include "stdafx.h"
#include "FileTree.h"
#include "MFCApplication1.h"
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////

void CTreeNode::Load()
{
	if (m_ImageId == 4) return; // don't for remote drives

	if (!m_Children.empty()) return;

	GetContent();

	for (auto& item : m_Children) {
		item->Add();
	}
}

void CTreeNode::Add()
{
	m_TreeItemHandle = m_TreeCtrl.InsertItem(m_Name, m_ImageId, m_ImageId, m_Parent->GetItemHandle());
	((CFileTree&)m_TreeCtrl).AddItemRef(m_TreeItemHandle, this);
}

void CTreeNode::Delete()
{
	DeleteChildren();
	
	((CFileTree&)m_TreeCtrl).RemoveItemRef(m_TreeItemHandle);
	m_TreeCtrl.DeleteItem(m_TreeItemHandle);
}

void CTreeNode::DeleteChildren()
{
	for (auto& item : m_Children) {
		item->Delete();
	}

	m_Children.clear();
}

void CTreeNode::OnExpand()
{
	for (auto& item : m_Children) {
		item->Load();
	}
}

void CTreeNode::OnCollaps()
{
}

CString CTreeNode::GetPath()
{
	if (m_Parent) {
		CString ParentPath = m_Parent->GetPath();
		return ParentPath + (ParentPath.GetLength() ? _T("\\") : _T("")) + m_Name;
	}
	return CString();
}

CString CTreeNode::GetParentPath()
{
	if (m_Parent) {
		return m_Parent->GetPath();
	}
	return CString();
}

/************************************************************/

void CPcNode::Load()
{
	CString PcName;
	DWORD Size = MAX_COMPUTERNAME_LENGTH + 1;
	GetComputerName(PcName.GetBufferSetLength(MAX_COMPUTERNAME_LENGTH + 1), &Size);
	
	m_Name = PcName;

	m_TreeItemHandle = m_TreeCtrl.InsertItem(PcName, 6, 6);

	((CFileTree&)m_TreeCtrl).AddItemRef(m_TreeItemHandle, this);

	CTreeNode::Load();
}

void CPcNode::Refresh()
{

}

void CPcNode::GetContent()
{
	const int MaxBuffer = 1024;
	CString Buffer;

	DWORD CharCount = GetLogicalDriveStrings(MaxBuffer, Buffer.GetBufferSetLength(MaxBuffer));

	for (int i = 0; i < CharCount;) {
		CString Drive = &Buffer.GetBuffer()[i];
		i += Drive.GetLength() + 1;
		int img = 5;
		Drive.Truncate(2);
		switch (GetDriveType(Drive))
		{
			case DRIVE_FIXED:
				img = 3;
				break;
			case DRIVE_REMOTE:
				img = 4;
				Drive += _T(" - NOT IMPLEMENTED :)");
			default:
				break;
		}
		m_Children.push_back(make_shared<CDirNode>(m_TreeCtrl, this, Drive.MakeUpper(), ENodeType::DRIVE, img));
	}
}

/************************************************************/

void CDirNode::Load()
{
	CTreeNode::Load();
}

void CDirNode::GetContent()
{
	CFileFind fileFind;
	
	if (!fileFind.FindFile(GetPath() + "\\*.*")) return;

	BOOL next = TRUE;
	
	std::map<CString, int> files, dirs;

	while (next) {
		next = fileFind.FindNextFileW();
		if (fileFind.IsDirectory()) {
			if (fileFind.GetFileName() != ".") {
				dirs.insert(std::pair<CString, int>(fileFind.GetFileName(), 0));
			}
		}
		else {
			CString fileName = fileFind.GetFileName();
			switch (CFileTree::GetExtentionType(fileName)) {
			case EXT_TEXT_SCRIPT:
				files.insert(std::pair<CString, int>(fileName, 1));
				break;
			case EXT_BIN_SCRIPT:
				files.insert(std::pair<CString, int>(fileName, 2));
				break;
			default:
				break;

			}
		}
	}

	for (std::map<CString, int>::iterator it = dirs.begin(); it != dirs.end(); ++it) {
		m_Children.push_back(make_shared<CDirNode>(m_TreeCtrl, this, it->first, ENodeType::DIRECTORY, it->second));
	}

	for (std::map<CString, int>::iterator it = files.begin(); it != files.end(); ++it) {
		m_Children.push_back(make_shared<CFileNode>(m_TreeCtrl, this, it->first, it->second));
	}
}

void CDirNode::Refresh()
{

}

/************************************************************/

void CFileNode::Load()
{
}

void CFileNode::Refresh()
{

}
/////////////////////////////////////////////////////////////////////////////
// CFileTree

CFileTree::CFileTree()
{
    
}

CFileTree::~CFileTree()
{
}

BEGIN_MESSAGE_MAP(CFileTree, CTreeCtrl)
    ON_WM_LBUTTONDBLCLK()
    ON_WM_CREATE()
    //ON_COMMAND(ID_SET_STARTUP_SCRIPT, &CFileTree::OnSetStartupScript)
    //ON_COMMAND(ID_COMPILE, &CFileTree::OnCompile)
    //ON_COMMAND(ID_DECOMPILE, &CFileTree::OnDecompile)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, &CFileTree::OnTvnItemexpanding)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileTree message handlers

BOOL CFileTree::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
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

void CFileTree::AddItemRef(HTREEITEM ItemHandle, CTreeNode* pNode)
{
	m_Items.insert(pair<HTREEITEM, CTreeNode*>(ItemHandle, pNode));
}

void CFileTree::RemoveItemRef(HTREEITEM ItemHandle)
{
	m_Items.erase(ItemHandle);
}


void CFileTree::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    CTreeCtrl::OnLButtonDblClk(nFlags, point);
    // TODO: Add your message handler code here and/or call default
    OpenFile();
}

void CFileTree::ReloadTree(HTREEITEM ParentItemHandle) {
	
	auto Item = m_Items.find(ParentItemHandle);

	if (Item != m_Items.end()) {
		//clean start up script reference
		ClearStartupScript(m_StartUpScripItemHandle);
		
		Item->second->DeleteChildren();
		Item->second->Load();
	}
}

void CFileTree::OpenFile()
{
	HTREEITEM hItem = GetSelectedItem();
	
	auto Item = m_Items.find(hItem);
	if (Item != m_Items.end()) {
		if (Item->second->GetType() == ENodeType::FILE) {
			CString Path = Item->second->GetPath();
			theApp.OpenDocumentFile(Path);
		}
	}
}

void CFileTree::LoadTree()
{
	m_RootNode = make_shared<CPcNode>(*this, nullptr);
	m_RootNode->Load();
}

int CFileTree::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CTreeCtrl::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  Add your specialized creation code here
    LoadTree();
    return 0;
}


void CFileTree::OnSetStartupScript() {

	HTREEITEM hItem = GetSelectedItem();

	auto Item = m_Items.find(hItem);
	if (Item != m_Items.end()) {
		if (Item->second->GetType() == ENodeType::FILE) {
			
			ClearStartupScript(m_StartUpScripItemHandle);

			TVITEMW tvi;
			tvi.mask = TVIF_STATE | TVIF_HANDLE;
			tvi.hItem = GetSelectedItem();
			tvi.state = TVIS_BOLD;
			tvi.stateMask = TVIS_BOLD;
			SetItem(&tvi);
			CString ScriptDir = Item->second->GetParentPath();
			theApp.SetScriptDir(_bstr_t(ScriptDir));
			m_StartUpScriptDir = ScriptDir;
			m_StartUpScript = Item->second->GetName();
			m_StartUpScripItemHandle = hItem;
			::PostMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_ON_SET_STARTUP_SCRIPT, 0, 0);
		}
	}
}

void CFileTree::OnClearStartupScript() {
    HTREEITEM hItem = GetSelectedItem();
	ClearStartupScript(hItem);
}

void CFileTree::ClearStartupScript(HTREEITEM ItemHandle)
{
	if (ItemHandle != nullptr) {
		TVITEMW tvi;
		tvi.mask = TVIF_STATE | TVIF_HANDLE;
		tvi.hItem = ItemHandle;
		tvi.state = 0;
		tvi.stateMask = TVIS_BOLD;
		SetItem(&tvi);
		m_StartUpScriptDir = "";
		m_StartUpScript = "";
		m_StartUpScripItemHandle = nullptr;
		::PostMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_ON_CLEAR_STARTUP_SCRIPT, 0, 0);
	}
}

EXTENTION_TYPE CFileTree::GetExtentionType(CString &fileName) {
    if ((fileName.GetLength() >= (sizeof(EXT_TEXT_SCRIPT_STRING) - 1)) && (fileName.Right((sizeof(EXT_TEXT_SCRIPT_STRING) - 1)).CompareNoCase(_T(EXT_TEXT_SCRIPT_STRING)) == 0)) {
        return EXT_TEXT_SCRIPT;
    }
    else if ((fileName.GetLength() >= (sizeof(EXT_BIN_SCRIPT_STRING) - 1)) && (fileName.Right((sizeof(EXT_BIN_SCRIPT_STRING) - 1)).CompareNoCase(_T(EXT_BIN_SCRIPT_STRING)) == 0)) {
        return EXT_BIN_SCRIPT;
    }
    else {
        return EXT_INVALID;
    }
}

bool CFileTree::IsEnabledMenuItem(DWORD nID) {
    if (HTREEITEM hItem = GetSelectedItem()) {
        CString currentItem = GetItemText(hItem);
        int image, selimage;
        GetItemImage(hItem, image, selimage);
        BOOL scriptIsRunning = theApp.IsScriptRunning();
        if (image == 0) { // directory
            switch (nID) {
                case ID_COMPILE:
                case ID_DECOMPILE:
                case ID_RUN:
                case ID_CLEAR_STARTUP_SCRIPT:
                case ID_SET_STARTUP_SCRIPT:
                    return false;
                default:
                    break;
            }
        }
        else {
            switch (nID) {
                case ID_COMPILE:
                    return !scriptIsRunning && (GetExtentionType(currentItem) == EXT_TEXT_SCRIPT);
                case ID_DECOMPILE:
                    return !scriptIsRunning && (GetExtentionType(currentItem) == EXT_BIN_SCRIPT);
                case ID_RUN:
                    break;
                case ID_CLEAR_STARTUP_SCRIPT:
                    return !scriptIsRunning && (m_StartUpScript == currentItem) && (m_StartUpScriptDir == (LPWSTR)theApp.GetScriptDir());
                case ID_SET_STARTUP_SCRIPT:
                    return !scriptIsRunning && !((m_StartUpScript == currentItem) && (m_StartUpScriptDir == (LPWSTR)theApp.GetScriptDir()));
                default:
                    break;
            }
        }
        switch (nID) {
            case ID_COMPILE:
            case ID_DECOMPILE:
            case ID_RUN:
            case ID_CLEAR_STARTUP_SCRIPT:
            case ID_SET_STARTUP_SCRIPT:
                return false;
            default:
                break;
        }
        return true;
    }
    return false;
}

BOOL CFileTree::GetCurrentFile(CString &filePath) {
    if (HTREEITEM hItem = GetSelectedItem()) {
		auto Item = m_Items.find(hItem);
		if (Item != m_Items.end()) {
			filePath = Item->second->GetPath();
	        return TRUE;
		}
    }
    return FALSE;
}

void CFileTree::OnCompile() {
    CString filePath;
    if (GetCurrentFile(filePath)) {
        theApp.CompileScript(filePath);
        TODO("Compile script in a thread and then ReloadTree");
		ReloadTree(GetParentItem(GetSelectedItem()));
    }
}


void CFileTree::OnDecompile() {
    CString filePath;
    if (GetCurrentFile(filePath)) {
        theApp.DeCompileScript(filePath);
        TODO("Decompile script in a thread and then ReloadTree");
		ReloadTree(GetParentItem(GetSelectedItem()));
	}
}


void CFileTree::OnTvnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	 
	auto Item = m_Items.find(pNMTreeView->itemNew.hItem);
	if (Item != m_Items.end()) {
		if(pNMTreeView->action == TVE_EXPAND)
			Item->second->OnExpand();
		else
			Item->second->OnCollaps();
	}
	*pResult = 0;
}
