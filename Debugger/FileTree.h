
#pragma once
#include "../Script/Script.h"
#include <vector>
#include <memory>
#include <unordered_map>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CFileTree window

using namespace script;

enum class ENodeType : byte {
	NONE,
	PC,
	DRIVE,
	DIRECTORY,
	FILE
};

enum class ENodeState : short {
	EXPANDED,
	COLLAPSED
};

class CTreeNode {
	public:
		CTreeNode(CTreeCtrl& TreeCtrl, CTreeNode *Parent, const CString& Name, ENodeType Type, int ImageId) : 
			m_TreeCtrl(TreeCtrl), m_Parent(Parent), m_Name(Name), m_Type(Type), m_ImageId(ImageId) {}
		virtual ~CTreeNode() {}

		virtual void Load();
		virtual void Refresh() = 0;
		virtual void GetContent() = 0;

		CString GetPath();
		CString GetParentPath();
		void Add();
		void Delete();
		void DeleteChildren();
		void OnExpand();
		void OnCollaps();

		int GetImageId() { return m_ImageId; }
		HTREEITEM GetItemHandle() { return m_TreeItemHandle; }
		ENodeType GetType() { return m_Type; }
		const CString& GetName() { return m_Name; }

	protected:
		CTreeCtrl &m_TreeCtrl;
		CTreeNode *m_Parent = nullptr;
		vector<shared_ptr<CTreeNode>> m_Children;
		CString m_Name;
		HTREEITEM m_TreeItemHandle = nullptr;
		int m_ImageId = 5;
		ENodeType m_Type = ENodeType::NONE;
		ENodeState m_State = ENodeState::COLLAPSED;
};

class CPcNode : public CTreeNode {
	public:
		CPcNode(CTreeCtrl& TreeCtrl, CTreeNode *Parent) : CTreeNode(TreeCtrl, Parent, _T(""), ENodeType::PC, 6) {}
		void Load() override;
		void Refresh() override;
		void GetContent() override;

	private:
};

class CDirNode : public CTreeNode {
	public:
		CDirNode(CTreeCtrl& TreeCtrl, CTreeNode* Parent, const CString& Name, ENodeType Type, int ImageId) : CTreeNode(TreeCtrl, Parent, Name, Type, ImageId) {}
		void Load() override;
		void Refresh() override;
		void GetContent() override;
	private:
};

class CFileNode : public CTreeNode {
	public:
		CFileNode(CTreeCtrl& TreeCtrl, CTreeNode* Parent, const CString& Name, int Image) : CTreeNode(TreeCtrl, Parent, Name, ENodeType::FILE, Image) {}
		void Load() override;
		void Refresh() override;
		void GetContent() override {};
};

class CFileTree : public CTreeCtrl
{
// Construction
public:
	CFileTree();

// Overrides
protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

// Implementation
public:
	virtual ~CFileTree();

protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
private:
    void LoadTree();
    BOOL GetCurrentFile(CString &filePath);

	shared_ptr<CPcNode> m_RootNode;
	unordered_map<HTREEITEM, CTreeNode*>	m_Items;

public:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSetStartupScript();

    static EXTENTION_TYPE GetExtentionType(CString &fileName);
	void AddItemRef(HTREEITEM ItemHandle, CTreeNode *pNode);
	void RemoveItemRef(HTREEITEM ItemHandle);
    void OnClearStartupScript();
	void ClearStartupScript(HTREEITEM ItemHandle);
	bool IsEnabledMenuItem(DWORD nID);
    void ReloadTree(HTREEITEM ParentItemHandle);
    void OpenFile();
    CString     m_StartUpScriptDir, m_StartUpScript;
	HTREEITEM	m_StartUpScripItemHandle = nullptr;
    afx_msg void OnCompile();
    afx_msg void OnDecompile();
	afx_msg void OnTvnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult);
};
