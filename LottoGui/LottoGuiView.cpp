
// LottoGuiView.cpp : implementation of the CLottoGuiView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "LottoGui.h"
#endif

#include "LottoGuiDoc.h"
#include "LottoGuiView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLottoGuiView

IMPLEMENT_DYNCREATE(CLottoGuiView, CScrollView)

BEGIN_MESSAGE_MAP(CLottoGuiView, CScrollView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CScrollView::OnFilePrintPreview)
END_MESSAGE_MAP()

// CLottoGuiView construction/destruction

CLottoGuiView::CLottoGuiView()
{
	// TODO: add construction code here

}

CLottoGuiView::~CLottoGuiView()
{
}

BOOL CLottoGuiView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CLottoGuiView drawing

void CLottoGuiView::OnDraw(CDC* pDC)
{
	CLottoGuiDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CFont aFont;
	aFont.CreateFont(20, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0,
		FF_MODERN, _T("Consolas"));

	CFont* pOldFont = pDC->SelectObject(&aFont);
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(20, 20, 20));

	CString headerText = pDoc->m_strSampleNums;
	if (headerText.IsEmpty())
	{
		headerText = _T("Use the Lotto Options menu to generate results.");
	}

	CSize textSize = pDC->GetTextExtent(_T("Sample"));
	int lineHeight = textSize.cy;
	if (lineHeight < 24)
	{
		lineHeight = 24;
	}

	const int totalLines = static_cast<int>(pDoc->m_vstrNums.GetSize()) + 8;
	CSize scrollArea(2600, totalLines * (lineHeight + 8));
	SetScrollSizes(MM_LOENGLISH, scrollArea);

	CRect headerRect(10, -20, 2400, -100);
	pDC->DrawText(headerText, headerRect, DT_LEFT | DT_WORDBREAK);

	int linePos = -120;
	for (INT_PTR index = 0; index < pDoc->m_vstrNums.GetSize(); ++index)
	{
		pDC->TextOutW(10, linePos, pDoc->m_vstrNums.GetAt(index));
		linePos -= lineHeight;
	}

	pDC->SelectObject(pOldFont);
}

void CLottoGuiView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;

	// Calculate the total size of this view
	sizeTotal.cx = 850;
	sizeTotal.cy = 1100;
	SetScrollSizes(MM_LOENGLISH, sizeTotal);
}

// CLottoGuiView printing

BOOL CLottoGuiView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CLottoGuiView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CLottoGuiView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CLottoGuiView diagnostics

#ifdef _DEBUG
void CLottoGuiView::AssertValid() const
{
	CView::AssertValid();
}

void CLottoGuiView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CLottoGuiDoc* CLottoGuiView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CLottoGuiDoc)));
	return (CLottoGuiDoc*)m_pDocument;
}
#endif //_DEBUG


// CLottoGuiView message handlers
