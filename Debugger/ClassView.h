
#pragma once

#include "FileTree.h"

class CClassToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CFunctionDefTree : public CTreeCtrl
{
    // Construction
public:
    CFunctionDefTree();

    // Overrides
protected:
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

    // Implementation
public:
    virtual ~CFunctionDefTree();

protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
private:
//    void LoadTree();
//    EXTENTION_TYPE GetExtentionType(CString &fileName);
//    BOOL GetCurrentFile(CString &filePath);
    //CString     m_CurrentDir;
    //CFileFind   m_FileFind;
public:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    //afx_msg void OnSetStartupScript();
  //  void OnClearStartupScript();
   // bool IsEnabledMenuItem(DWORD nID);
   // void ReloadTree();
    //void OpenFile();
    //virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
    //CString     m_StartUpScriptDir, m_StartUpScript;
    //afx_msg void OnCompile();
    //afx_msg void OnDecompile();
   // afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};

class CClassView : public CDockablePane
{
public:
	CClassView();
	virtual ~CClassView();

	void AdjustLayout();
	void OnChangeVisualStyle();

	void FillClassView();
    void ClearClassView() { m_wndClassView.DeleteAllItems(); }
protected:
	CClassToolBar m_wndToolBar;
	/*CFileTree*/CFunctionDefTree m_wndClassView;
	CImageList m_ClassViewImages;
	UINT m_nCurrSort;


// Overrides
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnClassAddMemberFunction();
	afx_msg void OnClassAddMemberVariable();
	afx_msg void OnClassDefinition();
	afx_msg void OnClassProperties();
	afx_msg void OnNewFolder();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnChangeActiveTab(WPARAM, LPARAM);
	afx_msg void OnSort(UINT id);
	afx_msg void OnUpdateSort(CCmdUI* pCmdUI);

	DECLARE_MESSAGE_MAP()
};

