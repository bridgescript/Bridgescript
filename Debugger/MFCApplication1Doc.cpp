
// MFCApplication1Doc.cpp : implementation of the CMFCApplication1Doc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "MFCApplication1.h"
#endif

#include "MFCApplication1Doc.h"
#include "ScriptView.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef RICH_EDIT
#include "CntrItem.h"
#define CDocument   CRichEditDoc
#endif // RICH_EDIT

// CMFCApplication1Doc

IMPLEMENT_DYNCREATE(CMFCApplication1Doc, CDocument)

BEGIN_MESSAGE_MAP(CMFCApplication1Doc, CDocument)
END_MESSAGE_MAP()


// CMFCApplication1Doc construction/destruction

CMFCApplication1Doc::CMFCApplication1Doc()
{
	// TODO: add one-time construction code here

}

CMFCApplication1Doc::~CMFCApplication1Doc()
{
}

BOOL CMFCApplication1Doc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

#ifdef RICH_EDIT
IMPLEMENT_SERIAL(CRichEditDocTestCntrItem, CRichEditCntrItem, 0)

CRichEditDocTestCntrItem::CRichEditDocTestCntrItem(REOBJECT* preo, CMFCApplication1Doc* pContainer)
    : CRichEditCntrItem(preo, pContainer)
{
    // TODO: add one-time construction code here
}

CRichEditDocTestCntrItem::~CRichEditDocTestCntrItem()
{
    // TODO: add cleanup code here
}


// CRichEditDocTestCntrItem diagnostics

#ifdef _DEBUG
void CRichEditDocTestCntrItem::AssertValid() const
{
    CRichEditCntrItem::AssertValid();
}

void CRichEditDocTestCntrItem::Dump(CDumpContext& dc) const
{
    CRichEditCntrItem::Dump(dc);
}
#endif



CRichEditCntrItem* CMFCApplication1Doc::CreateClientItem(REOBJECT* preo) const
{
    return new CRichEditDocTestCntrItem(preo, const_cast<CMFCApplication1Doc*>(this));
}
#endif // RICH_EDIT



// CMFCApplication1Doc serialization

void CMFCApplication1Doc::Serialize(CArchive& ar)
{
#ifdef RICH_EDIT
    if (ar.IsStoring())
    {
        // TODO: add storing code here
        
    }
    else
    {
        // TODO: add loading code here
    }
    CRichEditDoc::m_bRTF = FALSE;
    CRichEditDoc::Serialize(ar);
#else // RICH_EDIT
	if (ar.IsStoring())
	{
		// TODO: add storing code here
        POSITION pos = GetFirstViewPosition();
        if (CScriptView *pScriptView = static_cast<CScriptView*>(GetNextView(pos))) {
            //pScriptView->SetScriptFileName(lpszPathName);
            pScriptView->EnableEditing(false);
            if (!pScriptView->Save(ar)) {
                pScriptView->EnableEditing(true);
                throw new CFileException(CFileException::genericException);
            }
            pScriptView->EnableEditing(true);
        }
	}
	else
	{
		// TODO: add loading code here
	}
#endif // RICHEDIT
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CMFCApplication1Doc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CMFCApplication1Doc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CMFCApplication1Doc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CMFCApplication1Doc diagnostics

#ifdef _DEBUG
void CMFCApplication1Doc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMFCApplication1Doc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CMFCApplication1Doc commands


BOOL CMFCApplication1Doc::OnOpenDocument(LPCTSTR lpszPathName)
{
#ifdef RICH_EDIT

    if (!CDocument::OnOpenDocument(lpszPathName))
        return FALSE;
#endif // RICH_EDIT

    // TODO:  Add your specialized creation code here
    POSITION pos = GetFirstViewPosition();
    if (CScriptView *pScriptView = static_cast<CScriptView*>(GetNextView(pos))) {
        pScriptView->SetScriptFileName(lpszPathName);
#ifdef RICH_EDIT
        pScriptView->OnDocumentComplete(NULL);
#endif // RICH_EDIT

        return TRUE;
    }
    
    return FALSE;
}


BOOL CMFCApplication1Doc::OnSaveDocument(LPCTSTR lpszPathName)
{
    // TODO: Add your specialized code here and/or call the base class
    POSITION pos = GetFirstViewPosition();
    if (CScriptView *pScriptView = static_cast<CScriptView*>(GetNextView(pos))) {

        pScriptView->SetScriptFileName(lpszPathName);// ar.GetFile()->GetFileName());
                                                         //pScriptView->EnableEditing(false);
    }
    return CDocument::OnSaveDocument(lpszPathName);
}


BOOL CMFCApplication1Doc::CanCloseFrame(CFrameWnd* pFrame)
{
    // TODO: Add your specialized code here and/or call the base class

    POSITION pos = GetFirstViewPosition();
    if (CScriptView *pScriptView = static_cast<CScriptView*>(GetNextView(pos))) {
        if (pScriptView->IsModified()) {
            SetModifiedFlag();
        }
    }

    return CDocument::CanCloseFrame(pFrame);
}
