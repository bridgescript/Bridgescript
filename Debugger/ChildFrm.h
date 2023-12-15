
// ChildFrm.h : interface of the CChildFrame class
//

#pragma once

#define IMG_X_OFFSET 3
#define IMG_Y_OFFSET 3

class CChildFrame : public CMDIChildWndEx
{

	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame();

// Attributes
public:
#ifdef RICH_EDIT
    void SetPos(int pos);
    int GetImgSizeY() const { return m_imgSizeY; }
protected:
    CSplitterWndEx  m_wndSplitter;
    CImageList      m_imageBpList;
    CBrush          m_brushBkg;
    CPoint			m_bp;
    typedef struct BREAKPOINT {
        int m_y;
        BREAKPOINT_MARKER m_marker;
    } BREAKPOINT;
    int m_pos;
    int m_imgSizeY;
	int m_imgSizeX;
    map<int, BREAKPOINT>        m_breakpoints;
    int                         m_highlightLine;
    HIGHLIGHT_TYPE              m_highlightType;
#endif // RICH_EDIT

// Operations
public:
    void Invalidate();
    void SetBreakpointMarker(int line, BREAKPOINT_MARKER marker);
    void SetHighlight(int line, HIGHLIGHT_TYPE type);
// Overrides
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:

	DECLARE_MESSAGE_MAP()
#ifdef RICH_EDIT
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPaint();
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
#endif // RICH_EDIT

public:
    virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW, const RECT& rect = rectDefault, CMDIFrameWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
    afx_msg void OnNcPaint();
};
