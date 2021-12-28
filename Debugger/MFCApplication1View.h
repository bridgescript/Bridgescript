
// MFCApplication1View.h : interface of the CMFCApplication1View class
//

#pragma once


class CMFCApplication1View : public CView
{
protected: // create from serialization only
	CMFCApplication1View();
	DECLARE_DYNCREATE(CMFCApplication1View)

// Attributes
public:
	CMFCApplication1Doc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CMFCApplication1View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    CDHtmlDialog m_htmlDlg;
    CButton      m_editCheckBox;
// Generated message map functions
protected:
    afx_msg void OnDebugRun();
    afx_msg void OnDebugStop();
    afx_msg void OnDebugPause();
    afx_msg void OnDebugStepin();
    afx_msg void OnDebugStepover();
    afx_msg void OnDebugStepout();

	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnCreateAggregates();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
};

#ifndef _DEBUG  // debug version in MFCApplication1View.cpp
inline CMFCApplication1Doc* CMFCApplication1View::GetDocument() const
   { return reinterpret_cast<CMFCApplication1Doc*>(m_pDocument); }
#endif

