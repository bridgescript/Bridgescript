
#pragma once

#include "FileTree.h"

class CFileViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
public:
    /*DECLARE_MESSAGE_MAP()
    afx_msg void OnClearStartupScript();
    afx_msg void OnSetStartupScript();*/
};

class CFileView : public CDockablePane
{
// Construction
public:
	CFileView();

	void AdjustLayout();
	void OnChangeVisualStyle();

// Attributes
protected:

	CFileTree m_wndFileTree;
	CImageList m_FileViewImages;
	CFileViewToolBar m_wndToolBar;
    CString m_CurrentDir;
    CFileFind   m_FileFind;

protected:
	void FillFileView();

// Implementation
public:
	virtual ~CFileView();
    inline BOOL IsStartUpScriptSet() { return m_wndFileTree.m_StartUpScript.GetLength() != 0; };
    inline CString GetStartUpScriptPath() { return m_wndFileTree.m_StartUpScriptDir + m_wndFileTree.m_StartUpScript; };
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnProperties();
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenWith();
	afx_msg void OnCompile();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
    afx_msg void OnSetStartupScript();
    afx_msg void OnClearStartupScript();
    afx_msg void OnDecompile();
    afx_msg void OnRun();
};

