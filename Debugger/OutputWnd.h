
#pragma once
#include <string>
#include "..\Script\Script.h"

#define WM_DEBUGGER_FIRST_MSG           (WM_APP + 1)
#define WM_ON_DEBUGGER_STOP             (WM_DEBUGGER_FIRST_MSG + 0)
#define WM_ON_DEBUGGER_RUN              (WM_DEBUGGER_FIRST_MSG + 1)
#define WM_ON_DEBUGGER_PAUSE            (WM_DEBUGGER_FIRST_MSG + 2)
#define WM_ON_DEBUGGER_STEPIN           (WM_DEBUGGER_FIRST_MSG + 3)
#define WM_ON_DEBUGGER_STEPOVER         (WM_DEBUGGER_FIRST_MSG + 4)
#define WM_ON_DEBUGGER_STEPOUT          (WM_DEBUGGER_FIRST_MSG + 5)
#define WM_ON_DEBUGGER_SET_BREAKPOINT   (WM_DEBUGGER_FIRST_MSG + 6)
#define WM_ON_DEBUGGER_RESET_BREAKPOINT (WM_DEBUGGER_FIRST_MSG + 7)
#define WM_ON_DEBUGGER_BREAK_HIT        (WM_DEBUGGER_FIRST_MSG + 8)
#define WM_ON_SET_STARTUP_SCRIPT        (WM_DEBUGGER_FIRST_MSG + 9)
#define WM_ON_CLEAR_STARTUP_SCRIPT      (WM_DEBUGGER_FIRST_MSG + 10)
//#define WM_ON_SCRIPT_COMPILE_ERROR      (WM_DEBUGGER_FIRST_MSG + 11)
#define WM_ON_SCRIPT_RUNTIME_ERROR      (WM_DEBUGGER_FIRST_MSG + 12)
//#define WM_ON_END_DEBUG_THREAD          (WM_DEBUGGER_FIRST_MSG + 13)
#define WM_ON_BEGIN_THREAD              (WM_DEBUGGER_FIRST_MSG + 14)
#define WM_ON_END_THREAD                (WM_DEBUGGER_FIRST_MSG + 15)
#define WM_ON_END_SCRIPT_THREAD         (WM_DEBUGGER_FIRST_MSG + 16)

using namespace script;
using namespace std;
/////////////////////////////////////////////////////////////////////////////
// COutputList window

class COutputList : public CListBox
{
// Construction
public:
	COutputList();

// Implementation
public:
	virtual ~COutputList();

protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnViewOutput();

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnLbnDblclk();
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
//    virtual void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);
    virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
};

class COutputWnd : public CDockablePane
{
// Construction
public:
	COutputWnd();

	void UpdateFonts();

// Attributes
protected:
    class Output : public StdOutInterface {
    public:
        Output(COutputWnd *pOutputWnd) : m_pOutputWnd(pOutputWnd) {}
        virtual void Print(PRINT_TYPE type, string &text);
        virtual void Clear();
    private:
        COutputWnd *m_pOutputWnd;
    };

	CMFCTabCtrl	m_wndTabs;

	COutputList m_wndOutputBuild;
	COutputList m_wndOutputDebug;
	COutputList m_wndOutputFind;

protected:
	void FillBuildWindow();
	void FillDebugWindow();
	void FillFindWindow();

	void AdjustHorzScroll(CListBox& wndListBox);

// Implementation
public:
	virtual ~COutputWnd();
    void SetActiveTab(int pos) { PostMessage(WM_ON_SCRIPT_RUNTIME_ERROR, pos, 0); }
    Output  m_Output;
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
public:
    //afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
protected:
    afx_msg LRESULT OnOnScriptRuntimeError(WPARAM wParam, LPARAM lParam);
};

