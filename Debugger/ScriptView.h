#pragma once
#include "../Script/Basic.h"
#include "../Script/Debugger.h"
#include "../Script/Script.h"
//#include "smart_ptr.h"
#include "SafeThread.h"
#include "MainFrm.h"

using namespace script;
using namespace mb_thread_lib;

typedef enum Highlight_Type {
    HIGHLIGHT_ON_BREAK = 0,
    HIGHLIGHT_COMPILE_ERROR,
    HIGHLIGHT_RUNTIME_ERROR,
    HIGHLIGHT_STACK_CALL,
    HIGHLIGHT_BREAKPOINT,
    HIGHLIGHT_FUNC_DEF,
    HIGHLIGHT_OFF,
} HIGHLIGHT_TYPE;
// CScriptView html view

class CScriptView : public CRichEditView
{
	DECLARE_DYNCREATE(CScriptView)

protected:
	CScriptView();           // protected constructor used by dynamic creation
	virtual ~CScriptView();
    virtual void OnInitialUpdate(); // called first time after construct
    void ParseText();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnEnVscroll();

    void SetScriptFileName(LPCTSTR filePath) { m_ScriptFileName = filePath; };
    void HighlightLine(HIGHLIGHT_TYPE type, ui32 fileId, ui32 line, ThreadId threadId, Scope *pScope, ui32 pos = -1);
    void ClearHighlight(bool invalidateBreakpoints);
    void SetBreakPointMarker(ui32 line, BREAKPOINT_MARKER marker);
    void SetAllBreakpointsMarker(bool invalidate);
    void LoadBreakpointMarkers();

    void EnableEditing(bool enable);
    bool IsModified();
public:
    virtual void OnDocumentComplete(LPCTSTR lpszURL);
    void GetPathAndName(_bstr_t &fileName, _bstr_t &path);
private:

    struct KeyWordData {
        COLORREF color;
        KeyWordData(COLORREF c) : color(c) {}
    };
    KeyWordData* IsKeyWord(CString& s, long pos, long &begin, long &end);
    void MarkKeyWord(KeyWordData* pkw, long start, long end, long origend);
    CMapStringToPtr m_keywords;

    CString                    m_ScriptFileName;
    int                        m_currentLine;
    BREAK_HIT_DATA             m_breakHitData;
    HIGHLIGHT_TYPE             m_highlightType;
    
public:
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
    afx_msg void OnDestroy();
};
