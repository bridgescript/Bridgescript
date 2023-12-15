
// MFCApplication1Doc.h : interface of the CMFCApplication1Doc class
//


#pragma once

#ifdef RICH_EDIT
#define CDocument   CRichEditDoc
#endif // RICH_EDIT

class CMFCApplication1Doc : public CDocument
{
protected: // create from serialization only
	CMFCApplication1Doc();
	DECLARE_DYNCREATE(CMFCApplication1Doc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef RICH_EDIT
    virtual CRichEditCntrItem* CreateClientItem(REOBJECT* preo) const;
#endif // RICH_EDIT

#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CMFCApplication1Doc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
    virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
    virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
};

#ifdef CDocument
#undef CDocument
#endif // CDocument

