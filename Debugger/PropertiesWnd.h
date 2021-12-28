
#pragma once
#include "stdafx.h"
#include "../Script/Expressions.h"
//#include "ScriptView.h"
#include "smart_ptr.h"
#include <map>

using namespace std;
using namespace script;
using namespace mb_thread_lib;

/*********************************************/

class CPropertiesToolBar : public CMFCToolBar {
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

/*********************************************/

class CSplitterView : public CView {
    protected: // create from serialization only
        CSplitterView() : m_pChild(0) {};
        DECLARE_DYNCREATE(CSplitterView)
public:
    virtual void OnDraw(CDC* pDC);
    DECLARE_MESSAGE_MAP()
    afx_msg void OnSize(UINT nType, int cx, int cy);

public:
    void SetChild(CWnd *pChild) { m_pChild = pChild; }
private:
    CWnd    *m_pChild;
};

/*********************************************/
class CPropertiesWnd;

class CVarList : public CListCtrl {
protected:
    //DECLARE_DYNCREATE(CVarList)
    DECLARE_MESSAGE_MAP()
    typedef enum Image_Type {
        IMG_COLLAPSED = 0,
        IMG_EXPANDED = 1,
        IMG_NONE = 2,
    } IMAGE_TYPE;
public:
    typedef enum ViewAsType {
        VIEW_AS_DEFAULT = 0,
        VIEW_AS_HEX,
        VIEW_AS_BINARY,
    } VIEW_AS_TYPE;
public:
    CVarList() : m_pScope((Scope*)0xffffffff), m_frameIdx(INVALID_STACK_FRAME_IDX), m_pPropWnd(0) {};
    virtual ~CVarList() {

    }
    BOOL SetScope(Scope *pScope, ui32 frameIdx);
    Scope* GetScope() { return m_pScope; }
    void LoadVarList(ThreadId threadId, ui32 frameIdx);
    void LoadFooArgList(ThreadId threadId, ui32 frameIdx);
    void ClearList();

    void AddSimpleNode();
    
    CPropertiesWnd  *m_pPropWnd;

protected:
    void AddTreeNode();void InsertTreeNode(int idxParent, int parentIndent, IMAGE_TYPE img, LPARAM pData);
    void ExpandTreeNode(LVITEM &lv);
    void CollapseTreeNode(LVITEM &lv);
    void AddVarName(int row, IMAGE_TYPE img, int indent);
    void SetVarValue(int row);
    void SetVarTypeName(int row);
    void AppendNode(IMAGE_TYPE img);
    void UpdateImg(int item, int indent, IMAGE_TYPE img);
    //MemoryBlockRef* QueryMemoryBlockRef(string &itemPath);

    Scope           *m_pScope;
    ui32            m_frameIdx;
    
    class ItemDesc {
        ItemDesc();
        ItemDesc& operator=(const ItemDesc& that);
    public:
//#ifdef _DEBUG
        ItemDesc(string& itemPath, string& name, string& type, ThreadId threadId, ui32 stackFrame, ui32 varIdx, Type* pItemType);
//#else // _DEBUG
//        ItemDesc(string& itemPath, MemoryBlockRef *pMemoryBlockRef, ui32 offset, Type* pType);
//#endif // _DEBUG
        ItemDesc(const ItemDesc& that);
        ~ItemDesc();

        MemoryBlockRef* GetStackMemoryBlockRef(Memory &mem);
        MemoryBlockRef* GetMemoryBlockRef(Memory &mem);
        inline ThreadId GetThreadId() { return m_threadId; }
        inline ui32 GetStackFrame() { return m_stackFrame; }
        inline ui32 GetVarIdx() { return m_varIdx; }
        inline string& GetItemPath() { return m_itemPath; }
        _bstr_t         m_name;
        _bstr_t         m_value;
        _bstr_t         m_type;
        _bstr_t         m_address;
    protected:
        void SetType();
    private:
        Type           *m_pItemType;
        ThreadId        m_threadId;
        ui32            m_stackFrame,
                        m_varIdx;
        string          m_itemPath;
//#ifdef _DEBUG
        //string          m_name;
//#endif // _DEBUG
    };
    struct SubscriptCounter {
        struct _ {
            ui32 m_max, m_current;
            _(ui32 v) : m_max(v), m_current(0) {}
        };
        vector<_> digits;

        void PushMax(ui32 maxValue);
        void PushDigit();
        void GetSubscriptString(string &s);
    };

    vector<ItemDesc*>               m_ItemDescVector;

    void ExpandArray(string &itemPath, LVITEM &lv, SymbolStore *pSymStore, ItemDesc &itemDesc);
    void ExpandAggregate(string &itemPath, int row, int indent, SymbolStore *pSymStore, ItemDesc &itemDesc);
    void ExpandLib(string &itemPath, int row, SymbolStore *pSymStore, ItemDesc &itemDesc);
    void SetItemString(LVITEMW &item, _bstr_t &src);
    string EscapeString(string &token);
    string UnEscapeString(string &token);
public:
    afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnIdrPopupVarlist();
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnPopupViewasbinary();
    afx_msg void OnPopupViewasdefault();
    afx_msg void OnPopupViewashex();
    afx_msg void OnPopupViewaddress();
    afx_msg void OnUpdatePopupViewaddress(CCmdUI *pCmdUI);
    afx_msg void OnUpdatePopupViewasbinary(CCmdUI *pCmdUI);
    afx_msg void OnUpdatePopupViewasdefault(CCmdUI *pCmdUI);
    afx_msg void OnUpdatePopupViewashex(CCmdUI *pCmdUI);
    afx_msg void OnPopupCopy();
};

/*********************************************/

class CThreadList : public CListCtrl {
public:
    CThreadList() {};

    void Init();
    void Add(ThreadId threadId);
    void Remove(ThreadId threadId);
    void SetBlocked(ThreadId threadId, bool blocked);
    void SetActive(ThreadId threadId);
    void SetSymbol(ThreadId threadId, _bstr_t location, _bstr_t line, _bstr_t file);

    //void AddData(int row, int col, LPWSTR str, ui32 stackIdx);

    DECLARE_MESSAGE_MAP()
    afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);

private:
    //ui32 GetThreadId();
public:
    //afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
};



//class CWatchList : public CVarList {
//public:
//    CWatchList() {};
//
//    void Load();
//    void Save();
//    void LoadWatchList();
//
//    DECLARE_MESSAGE_MAP()
//    afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
//    afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
//    afx_msg void OnLvnSetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
//    afx_msg void OnLvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult);
//
//protected:
//    bool UpdateDebuggerData();
//    void GetAllVariables(unordered_map<string, ui32> &vars);
//};
//
class CStackList : public CListCtrl {
public:
    CStackList() {};
    
    void AddData(int row, int col, LPWSTR str, ui32 stackIdx);

    DECLARE_MESSAGE_MAP()
    afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);

private:
    ui32 GetThreadId();
public:
    //afx_msg void OnSize(UINT nType, int cx, int cy);
};

/*********************************************/

class CBreakpointList : public CListCtrl {
public:
    CBreakpointList() {};
    ~CBreakpointList();

    void Load();
    void Save();
    void GetBreakpoints(_bstr_t &path, _bstr_t &fileName, vector<BREAKPOINT_INFO> &breakpoints);
    BREAKPOINT_MARKER InsertBreakpoint(_bstr_t &path, _bstr_t &fileName, _bstr_t &line);
    void DeleteBreakpoint(_bstr_t &path, _bstr_t &fileName, _bstr_t &line);
    void InvalidateAllBreakpoints();
    void MarkAllValidBreakpoints(_bstr_t &path, _bstr_t &fileName);
    void PropagateBreakpoints();
    void UpdateBreakpointLine(_bstr_t &path, _bstr_t &fileName, _bstr_t &oldLine, _bstr_t &newLine);
    void RestoreBreakpopints(_bstr_t &path, _bstr_t &fileName, unordered_set<ui32> &breakpointLines);

    DECLARE_MESSAGE_MAP()
    afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);

private:
    void AddData(int row, int col, LPWSTR str, int imageIdx);
    void AddBreakpoint(_bstr_t &line, _bstr_t &fileName, _bstr_t &path);
    int FindUpper(_bstr_t &line, _bstr_t &fileName, _bstr_t &path, int count);
    int Find(_bstr_t &line, _bstr_t &fileName, _bstr_t &path);
    bool UpdateDebuggerData();
public:
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
    afx_msg void OnDeleteBreakpoint();
};

/*********************************************/
class CTabs : public CMFCTabCtrl {
public:
    CTabs():CMFCTabCtrl(), m_defaultBkColor(0xffffffff), m_disabledBkColor(0x009999ff) {};
    virtual BOOL SetActiveTab(int iTab);
    void SetBkColor(int iTab, BOOL disabled);
private:
    COLORREF    m_defaultBkColor,
                m_disabledBkColor;
public:
    //DECLARE_MESSAGE_MAP()
    //afx_msg void OnSetFocus(CWnd* pOldWnd);
    //afx_msg  LRESULT OnTabSetActive(WPARAM wParam, LPARAM lParam);
};

class CPropertiesWnd : public CDockablePane
{
// Construction
public:
	CPropertiesWnd();

    virtual void AdjustLayout();

// Attributes
public:
	void SetVSDotNetLook(BOOL bSet)
	{
		//m_wndLocalVarList.SetVSDotNetLook(bSet);
		//m_wndLocalVarsList.SetGroupNameFullWidth(bSet);
	}

protected:
    //CMFCTabCtrl	m_tabsWnd;
    CTabs m_tabsWnd;

	//CFont m_fntPropList;
	//CComboBox m_wndObjectCombo;
	//CPropertiesToolBar m_wndToolBar;
    /*CMFCPropertyGridCtrl*///CValueGrid m_wndLocalVarsList;
    //CListCtrl      m_wndStackList;
    CVarList        m_wndGlobalVarList;
    CThreadList     m_wndThreadList;
    //CWatchList      m_wndWatchList;
    CBreakpointList m_wndBreakpointList;
    
    //CDialog        m_dlg;
    //CSplitterWnd   m_splitter;
    typedef struct TabHolder {
        CMFCTabCtrl	   *m_pTabsWnd;
        CSplitterWnd   *m_pSplitterWnd;
        CVarList       *m_pVarListWnd;
        CStackList     *m_pStackListWnd;
        ThreadId        m_threadId;
        TabHolder() : m_pTabsWnd(0), m_pSplitterWnd(0), m_pVarListWnd(0), m_pStackListWnd(0), m_threadId(0) {}
        ~TabHolder() {
            m_pSplitterWnd->DestroyWindow();
            if (m_pSplitterWnd) delete m_pSplitterWnd;
            if (m_pVarListWnd) delete m_pVarListWnd;
            if (m_pStackListWnd) delete m_pStackListWnd; 
        }
        void Create(CMFCTabCtrl *pTabsWnd, CRect &rect, int idx, ThreadId threadId);
        CString GetName();
    private:
        TabHolder(const TabHolder&);
        TabHolder& operator= (const TabHolder&);
    } TAB_HOLDER;

    typedef map<ThreadId, TAB_HOLDER*>  TAB_HOLDER_MAP;
    
    TAB_HOLDER_MAP m_tabHolders;

    CImageList     m_imageList;
    CImageList     m_imageBpList;

// Implementation
public:
	virtual ~CPropertiesWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//afx_msg void OnExpandAllProperties();
	//afx_msg void OnUpdateExpandAllProperties(CCmdUI* pCmdUI);
	//afx_msg void OnSortProperties();
	//afx_msg void OnUpdateSortProperties(CCmdUI* pCmdUI);
	//afx_msg void OnProperties1();
	//afx_msg void OnUpdateProperties1(CCmdUI* pCmdUI);
	//afx_msg void OnProperties2();
	//afx_msg void OnUpdateProperties2(CCmdUI* pCmdUI);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);

	DECLARE_MESSAGE_MAP()

	//void InitPropList();
	//void SetPropListFont();
public:
    void InitStackListHeader(CListCtrl &listCtrl);
    void ClearGlobalVarList();
    void UpdateStackList(ThreadId threadId);
    void UpdateVarList(ThreadId threadId, ui32 frameIdx, Scope *pScope, CVarList &varlList);
    void UpdateActiveVarList(ThreadId threadId, ui32 frameIdx, Scope *pFunctionScope);
    void InitVarListHeader(CVarList &varList);
    void InitBreakpointListHeader();
    void OnBeginThread(ThreadId threadId);
    void OnEndThread(ThreadId threadId);
    void DisableTab(BOOL disable, ThreadId threadId);

    BREAKPOINT_MARKER InsertBreakpoint(_bstr_t &path, _bstr_t &fileName, _bstr_t &line);
    void DeleteBreakpoint(_bstr_t &path, _bstr_t &fileName, _bstr_t &line);
    void GetBreakpoints(_bstr_t &path, _bstr_t &fileName, vector<BREAKPOINT_INFO> &breakpoints);
    void PropagateBreakpoints();
    void InvalidateAllBreakpoints();
    void MarkAllValidBreakpoints(_bstr_t &path, _bstr_t &fileName);
    void UpdateBreakpointLine(_bstr_t &path, _bstr_t &fileName, _bstr_t &oldLine, _bstr_t &newLine);
    void RestoreBreakpopints(_bstr_t &path, _bstr_t &fileName, unordered_set<ui32> &breakpointLines);
    CVarList::VIEW_AS_TYPE GetViewAs();
    void SetViewAs(CVarList::VIEW_AS_TYPE viewAs);
    bool GetViewAddress();
    void SetViewAddress(bool viewAddress);
    void ToggleAddress();
private:
    void LoadData();
    void SaveData();

    CVarList::VIEW_AS_TYPE    m_viewAsType;
    bool                      m_viewAddress;
    //int m_nComboHeight;
    //afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
public:
    afx_msg void OnClose();
    afx_msg void OnDestroy();
};

