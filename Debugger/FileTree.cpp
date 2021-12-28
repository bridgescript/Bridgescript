
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


void CFileTree::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    CTreeCtrl::OnLButtonDblClk(nFlags, point);
    // TODO: Add your message handler code here and/or call default
    OpenFile();
}

void CFileTree::ReloadTree() {
    DeleteAllItems();
    LoadTree();
}

void CFileTree::OpenFile() {
    HTREEITEM hItem = GetSelectedItem();
    CString currentItem = GetItemText(hItem);
    int image, selimage;
    GetItemImage(hItem, image, selimage);
    if (image == 0) { // open directory
        CFileFind fileFind;
        CString findPath = theApp.GetScriptDir() + "*.*";

        fileFind.FindFile(findPath);
        BOOL next = TRUE;
        while (next) {
            next = fileFind.FindNextFileW();
            if (fileFind.IsDirectory()) {
                if (fileFind.GetFileName() == currentItem) {
                    theApp.SetScriptDir(_bstr_t(fileFind.GetFilePath() + "\\"));
                    ReloadTree();
                    break;
                }
            }
        }
    }
    else { // open file
        CString currentPath = theApp.GetScriptDir() + currentItem;
        theApp.OpenDocumentFile(currentPath);
    }
}
void CFileTree::LoadTree() {
    CFileFind fileFind;
    _bstr_t scriptPath = theApp.GetScriptDir();
    if (scriptPath.length()) {
        fileFind.FindFile(scriptPath + "*.*");
    }
    else {
        fileFind.FindFile();
    }
    fileFind.FindNextFileW();

    CString currentDir = fileFind.GetRoot();
    theApp.SetScriptDir(_bstr_t(currentDir));

    bool dir = currentDir == m_StartUpScriptDir;
    if (int len = currentDir.GetLength()) {
        TCHAR lastChar = currentDir.GetAt(len - 1);
        if ((lastChar == _T('\\')) || (lastChar == _T('/'))) {
            currentDir.Delete(len - 1);
        }
        HTREEITEM hSrc = InsertItem(currentDir, 0, 0);
        std::map<CString, int> files, dirs;
        //items.
        BOOL next = TRUE;
        while (next) {
            next = fileFind.FindNextFileW();
            if (fileFind.IsDirectory()) {
                if (fileFind.GetFileName() != ".") {
                    dirs.insert(std::pair<CString, int>(fileFind.GetFileName(), 0));
                }
            }
            else {
                CString fileName = fileFind.GetFileName();
                switch (GetExtentionType(fileName)) {
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
        };
        for (std::map<CString, int>::iterator it = dirs.begin(); it != dirs.end(); ++it) {
            InsertItem(it->first, it->second, it->second, hSrc);
        }

        for (std::map<CString, int>::iterator it = files.begin(); it != files.end(); ++it) {
            HTREEITEM hi = InsertItem(it->first, it->second, it->second, hSrc);
            if (m_StartUpScript.GetLength() && dir && (it->first == m_StartUpScript)) {
                TVITEMW tvi;
                tvi.mask = TVIF_STATE | TVIF_HANDLE;
                tvi.hItem = hi;
                tvi.state = TVIS_BOLD;
                tvi.stateMask = TVIS_BOLD;
                SetItem(&tvi);
            }
        }

        Expand(hSrc, TVE_EXPAND);
    }
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
    // TODO: Add your command handler code here
    HTREEITEM hItem = GetSelectedItem();
    CString currentItem = GetItemText(hItem);
    int nImage, nSelectedImage;
    GetItemImage(hItem, nImage, nSelectedImage);
    if (nImage != 0) { // file
        TVITEMW tvi;
        hItem = GetChildItem(GetRootItem());// GetNextItem(TVI_ROOT, TVGN_CHILD);
        while (hItem) {
            CString itemText = GetItemText(hItem);
            if (itemText == m_StartUpScript) {
                tvi.mask = TVIF_STATE | TVIF_HANDLE;
                tvi.hItem = hItem;
                tvi.state = 0;
                tvi.stateMask = TVIS_BOLD;
                SetItem(&tvi);
                break;
            }
            hItem = GetNextSiblingItem(hItem);// GetNextItem(hItem, TVGN_NEXT);
        }
        tvi.mask = TVIF_STATE | TVIF_HANDLE;
        tvi.hItem = GetSelectedItem();
        tvi.state = TVIS_BOLD;
        tvi.stateMask = TVIS_BOLD;
        SetItem(&tvi);
        m_StartUpScriptDir = (LPWSTR)theApp.GetScriptDir();
        m_StartUpScript = currentItem;
        ::PostMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_ON_SET_STARTUP_SCRIPT, 0, 0);
    }
}

void CFileTree::OnClearStartupScript() {
    HTREEITEM hItem = GetSelectedItem();
    TVITEMW tvi;
    tvi.mask = TVIF_STATE | TVIF_HANDLE;
    tvi.hItem = hItem;
    tvi.state = 0;
    tvi.stateMask = TVIS_BOLD;
    SetItem(&tvi);
    m_StartUpScriptDir = "";
    m_StartUpScript = "";
    ::PostMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_ON_CLEAR_STARTUP_SCRIPT, 0, 0);
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
                    return GetExtentionType(currentItem) == EXT_TEXT_SCRIPT;
                case ID_DECOMPILE:
                    return GetExtentionType(currentItem) == EXT_BIN_SCRIPT;
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
//BOOL CFileTree::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
//{
//    // TODO: Add your specialized code here and/or call the base class
//    bool enabled = true;
//    switch (nID) {
//        case ID_CLEAR_STARTUP_SCRIPT:
//            enabled = false;
//            break;
//        case ID_SET_STARTUP_SCRIPT:
//            enabled = true;
//            break;
//    }
//    return enabled ? CTreeCtrl::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo) : 0;
//}

BOOL CFileTree::GetCurrentFile(CString &filePath) {
    if (HTREEITEM hItem = GetSelectedItem()) {
        filePath = GetItemText(hItem);
        filePath = theApp.GetScriptDir() + filePath;
        return TRUE;
    }
    return FALSE;
}

void CFileTree::OnCompile() {
    CString filePath;
    if (GetCurrentFile(filePath)) {
        theApp.CompileScript(filePath);
        TODO("Compile script in a thread and then ReloadTree");
        ReloadTree();
    }
}


void CFileTree::OnDecompile() {
    CString filePath;
    if (GetCurrentFile(filePath)) {
        theApp.DeCompileScript(filePath);
        TODO("Decompile script in a thread and then ReloadTree");
        ReloadTree();
    }
}
