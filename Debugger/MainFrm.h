
// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "FileView.h"
#include "ClassView.h"
#include "ScriptView.h"
#include "OutputWnd.h"
#include "PropertiesWnd.h"
#include "../Script/Basic.h"
#include "../Script/Debugger.h"
#include "../Script/Script.h"

using namespace script;

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:

// Operations
public:
    /*void OnStop();
    void OnRun();
    void OnPause();
    void OnStepIn();
    void OnStep();
    void OnStepOut();
    void OnSetBreakPoint(ui32 fileId, ui32 line, bool set);*/
    //void OnBreakHit(ui32 fileId, ui32 line);
// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;
	CMFCToolBarImages m_UserImages;
	CFileView         m_wndFileTree;
	CClassView        m_wndClassView;
public:
	COutputWnd        m_wndOutput;
	CPropertiesWnd    m_wndProperties;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnViewFileView();
	afx_msg void OnUpdateViewFileView(CCmdUI* pCmdUI);
	afx_msg void OnViewClassView();
	afx_msg void OnUpdateViewClassView(CCmdUI* pCmdUI);
	afx_msg void OnViewOutputWindow();
	afx_msg void OnUpdateViewOutputWindow(CCmdUI* pCmdUI);
	afx_msg void OnViewPropertiesWindow();
	afx_msg void OnUpdateViewPropertiesWindow(CCmdUI* pCmdUI);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
    afx_msg void OnCmdDebugRun();
    afx_msg void OnCmdDebugStop();
    afx_msg void OnCmdDebugPause();
    afx_msg void OnCmdDebugStepin();
    afx_msg void OnCmdDebugStepover();
    afx_msg void OnCmdDebugStepout();

	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);

private:
    void EnableEditing(bool enable);

    class DebuggerEventsImpl : public DebuggerEvents {
    public:
        DebuggerEventsImpl(CMainFrame *pCMainFrame) : m_pCMainFrame(pCMainFrame) {};
        virtual ~DebuggerEventsImpl() {};

        virtual void OnStop(ThreadId threadId) { m_pCMainFrame->SendMessage(WM_ON_DEBUGGER_STOP, threadId, 0); };
        virtual void OnRun(ThreadId threadId) { m_pCMainFrame->SendMessage(WM_ON_DEBUGGER_RUN, threadId, 0); };
        virtual void OnPause(ThreadId threadId) { m_pCMainFrame->SendMessage(WM_ON_DEBUGGER_PAUSE, threadId, 0); };
        virtual void OnStepIn(ThreadId threadId) { m_pCMainFrame->SendMessage(WM_ON_DEBUGGER_STEPIN, threadId, 0); };
        virtual void OnStepOver(ThreadId threadId) { m_pCMainFrame->SendMessage(WM_ON_DEBUGGER_STEPOVER, threadId, 0); };
        virtual void OnStepOut(ThreadId threadId) { m_pCMainFrame->SendMessage(WM_ON_DEBUGGER_STEPOUT, threadId, 0); };
        virtual void OnSetBreakPoint(ui32 fileId, ui32 line, bool set) { m_pCMainFrame->SendMessage(set ? WM_ON_DEBUGGER_SET_BREAKPOINT : WM_ON_DEBUGGER_RESET_BREAKPOINT, fileId, line); };
        virtual void OnBreakHit(ui32 fileId, ui32 line, ThreadId threadId, Scope* pScope) {
            BreakHitData data = { fileId, line, threadId, pScope };
            m_pCMainFrame->SendMessage(WM_ON_DEBUGGER_BREAK_HIT, (WPARAM)&data, 0); 
        }; 
        virtual void OnBeginThread(ThreadId threadId) { m_pCMainFrame->SendMessage(WM_ON_BEGIN_THREAD, threadId, 0); };
        virtual void OnEndThread(ThreadId threadId) { m_pCMainFrame->SendMessage(WM_ON_END_THREAD, threadId, 0); };
    private:
        CMainFrame *m_pCMainFrame;
    } m_DebuggerEventsImpl;

    DEBUGGER_STATES m_debuggerState;
    ScriptRunner   *m_pRunner;
    //SymbolStore *m_pSymbolStore;

public:
    CScriptView *GetScriptView(ui32 fileId);
    CScriptView *GetScriptView(LPCTSTR filePath);
    inline BOOL IsStartUpScriptSet() { return m_wndFileTree.IsStartUpScriptSet(); };
    CString GetStartUpScriptPath() { return m_wndFileTree.GetStartUpScriptPath(); };
    //void SetSymbolStore(SymbolStore* pSymbolStore) { m_pSymbolStore = pSymbolStore; }
    DebuggerEvents* GetDebuggerEvents() { return &m_DebuggerEventsImpl; }
    StdOutInterface* GetStdOutInterface() { return &m_wndOutput.m_Output; }
    afx_msg void OnDestroy();
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
    void PropagateBreakpoints();
protected:
    afx_msg LRESULT OnDebuggerStop(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDebuggerRun(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDebuggerPause(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDebuggerStepIn(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDebuggerStepOver(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDebuggerStepOut(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDebuggerSetBreakPoint(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDebuggerResetBreakPoint(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDebuggerBreakHit(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetStartupScript(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnClearStartupScript(WPARAM wParam, LPARAM lParam);
    //afx_msg LRESULT OnScriptCompileError(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnBeginThread(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnEndThread(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnEndScriptThread(WPARAM wParam, LPARAM lParam);
};


