
// LottoGuiView.h : interface of the CLottoGuiView class
//

#pragma once


class CLottoGuiView : public CScrollView
{
protected: // create from serialization only
	CLottoGuiView();
	DECLARE_DYNCREATE(CLottoGuiView)

// Attributes
public:
	CLottoGuiDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // Called for the first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CLottoGuiView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in LottoGuiView.cpp
inline CLottoGuiDoc* CLottoGuiView::GetDocument() const
   { return reinterpret_cast<CLottoGuiDoc*>(m_pDocument); }
#endif

