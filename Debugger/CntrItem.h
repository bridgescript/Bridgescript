
// CntrItem.h : interface of the CRichEditDocTestCntrItem class
//
//#ifdef RICH_EDIT



#pragma once

class CMFCApplication1Doc;
class ScriptView;

class CRichEditDocTestCntrItem : public CRichEditCntrItem
{
	DECLARE_SERIAL(CRichEditDocTestCntrItem)

// Constructors
public:
	CRichEditDocTestCntrItem(REOBJECT* preo = nullptr, CMFCApplication1Doc* pContainer = nullptr);
		// Note: pContainer is allowed to be null to enable IMPLEMENT_SERIALIZE
		//  IMPLEMENT_SERIALIZE requires the class have a constructor with
		//  zero arguments.  Normally, OLE items are constructed with a
		//  non-null document pointer

// Attributes
public:
    CMFCApplication1Doc* GetDocument()
		{ return reinterpret_cast<CMFCApplication1Doc*>(CRichEditCntrItem::GetDocument()); }
    ScriptView* GetActiveView()
		{ return reinterpret_cast<ScriptView*>(CRichEditCntrItem::GetActiveView()); }

// Implementation
public:
	~CRichEditDocTestCntrItem();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

//#endif // RICH_EDIT
