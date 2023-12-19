
// MFCApplication1.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "MFCApplication1.h"
#include "MainFrm.h"
#include "..\Script\build.h"
#include "ChildFrm.h"
#include "MFCApplication1Doc.h"
#include "MFCApplication1View.h"
#include "ScriptView.h"
#include <initguid.h>
//#include "MFCApplication1_i.c"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCApplication1App


class CMFCApplication1Module :
    public ATL::CAtlMfcModule
{
public:
    //DECLARE_LIBID(LIBID_MFCApplication1Lib);
    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_MFCAPPLICATION1, "{04210E0A-FE98-4E5B-979F-58D9C1ECF7B4}");
};

CMFCApplication1Module _AtlModule;

BEGIN_MESSAGE_MAP(CMFCApplication1App, CWinAppEx)
    ON_COMMAND(ID_DEBUG_RUN, &CMFCApplication1App::OnCmdDebugRun)
    ON_COMMAND(ID_DEBUG_STOP, &CMFCApplication1App::OnCmdDebugStop)
    ON_COMMAND(ID_DEBUG_PAUSE, &CMFCApplication1App::OnCmdDebugPause)
    ON_COMMAND(ID_DEBUG_STEPIN, &CMFCApplication1App::OnCmdDebugStepin)
    ON_COMMAND(ID_DEBUG_STEPOVER, &CMFCApplication1App::OnCmdDebugStepover)
    ON_COMMAND(ID_DEBUG_STEPOUT, &CMFCApplication1App::OnCmdDebugStepout)

	ON_COMMAND(ID_APP_ABOUT, &CMFCApplication1App::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
    ON_COMMAND(ID_DEBUG_DUMPMEMORY, &CMFCApplication1App::OnDebugDumpMemory)
END_MESSAGE_MAP()


// CMFCApplication1App construction

CMFCApplication1App::CMFCApplication1App()
{
	m_bHiColorIcons = TRUE;

	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("BridgeDebugger.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CMFCApplication1App object

CMFCApplication1App theApp;


// CMFCApplication1App initialization

BOOL CMFCApplication1App::InitInstance()
{
    // InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
    TODO("modify this string to be something appropriate");
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(10);  // Load standard INI file options (including MRU)
    
    LoadDebuggerData();

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;

	pDocTemplate = new CMultiDocTemplate(IDR_MFCApplication1TYPE,
		RUNTIME_CLASS(CMFCApplication1Doc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
        RUNTIME_CLASS(CScriptView));
		//RUNTIME_CLASS(CMFCApplication1View));

	if (!pDocTemplate)
		return FALSE;

    pDocTemplate->SetContainerInfo(IDR_RichEditDocTestTYPE_CNTR_IP);

	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;

	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	#if !defined(_WIN32_WCE) || defined(_CE_DCOM)
	// Register class factories via CoRegisterClassObject().
//	if (FAILED(_AtlModule.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE)))
//		return FALSE;
	#endif // !defined(_WIN32_WCE) || defined(_CE_DCOM)
	// App was launched with /Embedding or /Automation switch.
	// Run app as automation server.
	if (cmdInfo.m_bRunEmbedded || cmdInfo.m_bRunAutomated)
	{
		// Don't show the main window
		return TRUE;
	}
	// App was launched with /Unregserver or /Unregister switch.
	//if (cmdInfo.m_nShellCommand == CCommandLineInfo::AppUnregister)
	//{
	//	_AtlModule.UpdateRegistryAppId(FALSE);
	//	_AtlModule.UnregisterServer(TRUE);
	//	return FALSE;
	//}
	//// App was launched with /Register or /Regserver switch.
	//if (cmdInfo.m_nShellCommand == CCommandLineInfo::AppRegister)
	//{
	//	_AtlModule.UpdateRegistryAppId(TRUE);
	//	_AtlModule.RegisterServer(TRUE);
	//	return FALSE;
	//}

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);


	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.

	///uncomment this line to process command line!
    //*if (!ProcessShellCommand(cmdInfo))
	//	return FALSE;*/

	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

int CMFCApplication1App::ExitInstance()
{
#if !defined(_WIN32_WCE) || defined(_CE_DCOM)
	_AtlModule.RevokeClassObjects();
#endif
    m_xmlDebuggerData.Release();
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CMFCApplication1App message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    CString m_version;

	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
    m_version = BUILD_DATE;
    m_version += "\n" BRIDGE_SCRIPT_VER();
    m_version += RELEASE_TYPE;
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_VERSION, m_version);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CMFCApplication1App::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CMFCApplication1App customization load/save methods

void CMFCApplication1App::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CMFCApplication1App::LoadCustomState()
{
}

void CMFCApplication1App::SaveCustomState()
{
}

Debugger* CMFCApplication1App::GetDebugger() { 
    if (m_spThreadParameter) {
        if (m_spThreadParameter->m_pRunner) {
            return &m_spThreadParameter->m_pRunner->GetInterpreter().GetDebugger();
        }
    }
    return 0;
}

SymbolStore* CMFCApplication1App::GetSymbolStore() {
    Debugger *pDebugger = GetDebugger();
    return pDebugger ? pDebugger->GetSymbolStore() : 0;
}

Memory* CMFCApplication1App::GetMemory() {
    Debugger *pDebugger = GetDebugger();
    return pDebugger ? pDebugger->GetMemory() : 0;
}

// CMFCApplication1App message handlers
void CMFCApplication1App::OnCmdDebugRun() {
    if (!m_ScriptRunnerThread.GetThreadId()) {
        if (CanRunEditedScript() && LoadScript()) {
            m_ScriptRunnerThread.Start(m_spThreadParameter);
            m_spThreadParameter->m_pRunner->GetInterpreter().GetDebuggerHandler().Wait();
            theApp.GetDebugger()->Run();
            return;
        }
        else {
            return;
        }
    }
    Debugger *pDebugger = theApp.GetDebugger();
    if (pDebugger) {
        pDebugger->Run();
    }
}


void CMFCApplication1App::OnCmdDebugStop() {
    Debugger *pDebugger = theApp.GetDebugger();
    if (pDebugger) {
        pDebugger->Stop();
    }
    m_ScriptRunnerThread.Stop();
}


void CMFCApplication1App::OnCmdDebugPause() {
    Debugger *pDebugger = theApp.GetDebugger();
    if (pDebugger) {
        pDebugger->Pause(PauseType(PAUSE_TYPE::PAUSE_ALL));
    }
}


void CMFCApplication1App::OnCmdDebugStepin() {
    if (!m_ScriptRunnerThread.GetThreadId()) {
        if (CanRunEditedScript() && LoadScript()) {
            m_ScriptRunnerThread.Start(m_spThreadParameter);
            m_spThreadParameter->m_pRunner->GetInterpreter().GetDebuggerHandler().Wait();
        }
        else {
            return;
        }
    }
    Debugger *pDebugger = theApp.GetDebugger();
    if (pDebugger) {
        pDebugger->StepIn();
    }
}


void CMFCApplication1App::OnCmdDebugStepover() {
    if (!m_ScriptRunnerThread.GetThreadId()) {
        if (CanRunEditedScript() && LoadScript()) {
            m_ScriptRunnerThread.Start(m_spThreadParameter);
            m_spThreadParameter->m_pRunner->GetInterpreter().GetDebuggerHandler().Wait();
        }
        else {
            return;
        }
    }
    Debugger *pDebugger = theApp.GetDebugger();
    if (pDebugger) {
        pDebugger->StepOver();
    }
}


void CMFCApplication1App::OnCmdDebugStepout() {
    Debugger *pDebugger = theApp.GetDebugger();
    if (pDebugger) {
        pDebugger->StepOut();
    }
}

BOOL CMFCApplication1App::LoadScript() {
    if (!m_ScriptRunnerThread.GetThreadId()) {
        CMainFrame *pMainFrame = (CMainFrame*)m_pMainWnd;
		if (!pMainFrame->IsStartUpScriptSet()) return FALSE;
        string scriptPath = (LPCSTR)_bstr_t(pMainFrame->GetStartUpScriptPath());
		if (scriptPath.empty()) return FALSE;
        StdOutInterface *pStdOutInterface = pMainFrame->GetStdOutInterface();
        pStdOutInterface->Clear();
        smart_ptr<ScriptRunner> spRunner(new ScriptRunner(scriptPath, pStdOutInterface));
        m_spThreadParameter = smart_ptr<Parameter>(new Parameter(spRunner, this));
        if (spRunner->Load()) {
            spRunner->GetInterpreter().PreRun(pMainFrame->GetDebuggerEvents());
            ((CMainFrame*)m_pMainWnd)->PropagateBreakpoints();
            return TRUE;
        }
        else {
            theApp.UnLoadScript(); // free resources
        }
    }
    return FALSE;
}

void CMFCApplication1App::UnLoadScript() {
    if (!m_ScriptRunnerThread.GetThreadId()) {
        m_spThreadParameter = smart_ptr<Parameter>(0);
    }
}

void CMFCApplication1App::CompileScript(CString &scriptPath) {
    if (!m_ScriptRunnerThread.GetThreadId()) {
        CMainFrame *pMainFrame = (CMainFrame*)m_pMainWnd;
        StdOutInterface *pStdOutInterface = pMainFrame->GetStdOutInterface();
        pStdOutInterface->Clear();
        string path = (LPCSTR)_bstr_t(scriptPath);
        smart_ptr<ScriptRunner> spRunner(new CompilerRunner(path, pStdOutInterface));
        m_spThreadParameter = smart_ptr<Parameter>(new Parameter(spRunner, this));
        if (spRunner->Load()) {
            spRunner->GetInterpreter().PreRun(pMainFrame->GetDebuggerEvents());
            spRunner->Exec();
        }
        m_spThreadParameter = smart_ptr<Parameter>(0);
    }
}

void CMFCApplication1App::DeCompileScript(CString &scriptPath) {
    if (!m_ScriptRunnerThread.GetThreadId()) {
        CMainFrame *pMainFrame = (CMainFrame*)m_pMainWnd;
        StdOutInterface *pStdOutInterface = pMainFrame->GetStdOutInterface();
        pStdOutInterface->Clear();
        string path = (LPCSTR)_bstr_t(scriptPath);
        smart_ptr<ScriptRunner> spRunner(new DecompilerRunner(path, pStdOutInterface));
        m_spThreadParameter = smart_ptr<Parameter>(new Parameter(spRunner, this));
        if (spRunner->Load()) {
            spRunner->GetInterpreter().PreRun(pMainFrame->GetDebuggerEvents());
            spRunner->Exec();
        }
        m_spThreadParameter = smart_ptr<Parameter>(0);
    }
}

bool CMFCApplication1App::CanRunEditedScript() {
    vector<CDocument*> docToSave;
    CString paths(L"\'");
    POSITION pos = m_pDocManager->GetFirstDocTemplatePosition();
    if (CDocTemplate *pDocTemplate = theApp.m_pDocManager->GetNextDocTemplate(pos)) {
        CDocument* pOpenDocument = NULL;
        POSITION docPos = pDocTemplate->GetFirstDocPosition();
        while (docPos) {
            pOpenDocument = pDocTemplate->GetNextDoc(docPos);
            CString path = pOpenDocument->GetPathName();
            POSITION posFV = pOpenDocument->GetFirstViewPosition();
            CScriptView *pScriptView = static_cast<CScriptView*>(pOpenDocument->GetNextView(posFV));
            pScriptView->ClearHighlight(false);
            if (pScriptView->IsModified()) {
                docToSave.push_back(pOpenDocument);
                paths += path + L", ";
            }
        }
    }
    if (docToSave.size()) {
        CString prompt;
        paths.Truncate(paths.GetLength() - 2);
        paths += L"\'";
        AfxFormatString1(prompt, AFX_IDP_ASK_TO_SAVE, paths);

        switch (AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE))
        {
            case IDCANCEL:
                return false;       // don't continue
            case IDYES:
                // If so, either Save or Update, as appropriate
                for (vector<CDocument*>::iterator it = docToSave.begin(); it != docToSave.end(); ++it) {
                    if (!(*it)->DoFileSave()) {
                        return FALSE;       // don't continue
                    }
                }
                break;
            default:
                ASSERT(FALSE);
            case IDNO:
                return false;
        }
    }
    return true;
}

void CMFCApplication1App::OnClosingMainFrame(CFrameImpl* pFrameImpl)
{
    OnCmdDebugStop();
    TODO("Wait for the script thread to finish and then continue!");
    return CWinAppEx::OnClosingMainFrame(pFrameImpl);
}


void CMFCApplication1App::OnDebugDumpMemory() {
#if defined(TRACE_MEMORY) 
    Memory::GetInstance().DumpMemoryBlocks();
#endif // defined(TRACE_MEMORY) 
}

void CMFCApplication1App::LoadDebuggerData() {
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
        <script_path><script_path>
        ...
        <watch>
            <name>var_name</name>
            ...
        </watch>
    </root>
    */

    if (!m_xmlDebuggerDataPath.length()) {
        WCHAR dir[MAX_PATH + 1];
        //GetCurrentDirectory(sizeof(dir) / sizeof(dir[0]), dir);
        ui32 size = GetModuleFileName(0, dir, sizeof(dir) / sizeof(dir[0]));
        //if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        //}
        
        CString path(dir);
        path = path.Left(path.ReverseFind(L'\\'));

        m_xmlDebuggerDataPath = _bstr_t(path) + L"\\debugger.data";
        HRESULT hr = m_xmlDebuggerData.CoCreateInstance(L"Msxml2.DOMDocument.3.0");

        if (!m_xmlDebuggerData) return;

        _variant_t docPath(m_xmlDebuggerDataPath);

        m_xmlDebuggerData->put_validateOnParse(VARIANT_TRUE);
        m_xmlDebuggerData->put_preserveWhiteSpace(VARIANT_TRUE);

        m_xmlDebuggerData->put_async(VARIANT_FALSE);

        VARIANT_BOOL res;
        hr = m_xmlDebuggerData->load(docPath, &res);

        if (FAILED(hr) || res == VARIANT_FALSE) {
            CString strXmlDoc;
            BOOL bNameValid = strXmlDoc.LoadString(IDS_XML_DATA);
            ASSERT(bNameValid);
            hr = m_xmlDebuggerData->loadXML(_bstr_t(strXmlDoc), &res);
            hr = m_xmlDebuggerData->save(docPath);
        }
    }
}

void CMFCApplication1App::SaveDebuggerData() {
    HRESULT hr = m_xmlDebuggerData->save(_variant_t(m_xmlDebuggerDataPath));
}

_bstr_t CMFCApplication1App::GetScriptDir() {
    CComPtr<IXMLDOMNode> spScriptPath;
    HRESULT hr = m_xmlDebuggerData->selectSingleNode(_bstr_t("//root/script_path"), &spScriptPath);
    if (S_OK != hr) return "";

    _bstr_t value;
    hr = spScriptPath->get_text(&value.GetBSTR());
    return value;
}

void CMFCApplication1App::SetScriptDir(_bstr_t &dir) {
    CComPtr<IXMLDOMNode> spScriptPath;
    HRESULT hr = m_xmlDebuggerData->selectSingleNode(_bstr_t("//root/script_path"), &spScriptPath);
    if (S_OK != hr) return;

    hr = spScriptPath->put_text(dir);
}
