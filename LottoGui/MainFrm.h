
// MainFrm.h : interface of the CMainFrame class
//

#pragma once

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CToolBar          m_wndToolBar;
	CStatusBar        m_wndStatusBar;
	CImageList        m_menuImages;

	struct MenuIconItem
	{
		CString text;
		int imageIndex;
	};

	MenuIconItem       m_menuIconItems[14];

	BOOL InitializeMenuIcons();
	BOOL RebuildMenuImageList(UINT dpi);
	BOOL SetMenuItemOwnerDraw(HMENU hMenu, UINT commandId, MenuIconItem& item);

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg LRESULT OnDpiChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

};
