
// MFCApplication1.h : main header file for the MFCApplication1 application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include "../Script/Basic.h"
#include "../Script/Debugger.h"
#include "../Script/Script.h"
#include "SafeThread.h"
#include "MainFrm.h"
//#include "MFCApplication1_i.h"

//using namespace script;
using namespace mb_thread_lib;

// CMFCApplication1App:
// See MFCApplication1.cpp for the implementation of this class
//

class CMFCApplication1App : public CWinAppEx
{
    class Parameter {
        Parameter();
        Parameter(const Parameter&);
        Parameter& operator= (const Parameter&);
    public:
        Parameter(smart_ptr<ScriptRunner> &pRunner, CMFCApplication1App *pApp) : m_pRunner(pRunner), m_pApp(pApp){}
        void Clear() { 
            CMainFrame* pMainFrame = (CMainFrame*)m_pApp->GetMainWnd(); //theApp.GetMainWnd
            pMainFrame->SendMessage(WM_ON_END_SCRIPT_THREAD, 0, 0);
            //pMainFrame->m_wndProperties.ClearGlobalVarList();
            //m_pApp->OnDebugDumpMemory();
            //m_pApp->UnLoadScript();
            //TODO("Must release Runner in the MFCApp thread!");
            //m_pRunner = smart_ptr<ScriptRunner>(0); 
        }
        smart_ptr<ScriptRunner>   m_pRunner;
        CMFCApplication1App      *m_pApp;
    };

    typedef smart_ptr< Parameter, sp_free< Parameter > > _Param;

    class Runner : public CThreadInterface<Parameter> {
    protected:
        virtual void Thread(_Param param) {
            param->m_pRunner->Exec();
       };
        virtual void Notify(mb_thread_lib::NOTIFY_EVENT_TYPE evt, _Param& param) {
            switch (evt) {
                case mb_thread_lib::BEGIN_THREAD_EVENT:
                    //Init(param);
                    break;
                case mb_thread_lib::TERMINATE_THREAD_EVENT:
                    Term(param);
                    break;
                case END_THREAD_EVENT:
                    End(param);
                    break;
                default:
                    break;
            }
        }
        void Init(_Param &param) {
            //param->m_pRunner->Load();
            //param->m_pRunner->BeforeExec(param->m_pDebuggerNotify);
        }
        void End(_Param &param) {
            //param->m_pApp->m_pMainWnd->SendMessage(WM_ON_END_DEBUG_THREAD, 0, 0);
            param->m_pRunner->GetInterpreter().PostRun();
            param->Clear();
            //param = smart_ptr<Parameter>(0);
        };
        void Term(_Param &param) {
            //param->Clear();
            //End(); 
        };
    public:
//        Runner() {};
//        virtual ~Runner(void) {};
    };

    typedef CSafeThread<Runner, Parameter> SCRIPT_RUNNER_THREAD;

    smart_ptr<Parameter>    m_spThreadParameter;
    SCRIPT_RUNNER_THREAD    m_ScriptRunnerThread;

public:
	CMFCApplication1App();

    inline BOOL IsScriptRunning() { return m_ScriptRunnerThread.GetThreadId() != 0; };
    Debugger* GetDebugger();
    SymbolStore* GetSymbolStore();
    Memory* GetMemory();
    CComPtr<IXMLDOMDocument> GetDebuggerData() { return m_xmlDebuggerData; };
    void SaveDebuggerData();
    void ReleaseScriptThreadRunner() { m_spThreadParameter->m_pRunner = smart_ptr<ScriptRunner>(0); };
// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
    afx_msg void OnDebugDumpMemory();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

    BOOL LoadScript();
    void UnLoadScript();
    void CompileScript(CString &scriptPath);
    void DeCompileScript(CString &scriptPath);
    _bstr_t GetScriptDir();
    void SetScriptDir(_bstr_t &dir);
    bool CanRunEditedScript();

    afx_msg void OnCmdDebugRun();
    afx_msg void OnCmdDebugStop();
    afx_msg void OnCmdDebugPause();
    afx_msg void OnCmdDebugStepin();
    afx_msg void OnCmdDebugStepover();
    afx_msg void OnCmdDebugStepout();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
    virtual void OnClosingMainFrame(CFrameImpl* pFrameImpl);

    private:
        void LoadDebuggerData();
        //void SaveDebuggerData();

        _bstr_t                     m_xmlDebuggerDataPath;
        CComPtr<IXMLDOMDocument>    m_xmlDebuggerData;
};

extern CMFCApplication1App theApp;
