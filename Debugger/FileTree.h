
#pragma once
#include "../Script/Script.h"

/////////////////////////////////////////////////////////////////////////////
// CFileTree window

using namespace script;

class CFileTree : public CTreeCtrl
{
// Construction
public:
	CFileTree();

// Overrides
protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

// Implementation
public:
	virtual ~CFileTree();

protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
private:
    void LoadTree();
    EXTENTION_TYPE GetExtentionType(CString &fileName);
    BOOL GetCurrentFile(CString &filePath);
public:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSetStartupScript();
    void OnClearStartupScript();
    bool IsEnabledMenuItem(DWORD nID);
    void ReloadTree();
    void OpenFile();
    CString     m_StartUpScriptDir, m_StartUpScript;
    afx_msg void OnCompile();
    afx_msg void OnDecompile();
};
