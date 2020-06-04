
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

	// TODO: add draw code for native data here
	CFont aFont;
	aFont.CreateFont(22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		FF_ROMAN, _T("Times New Roman"));

	CFont * pOldFont = pDC->SelectObject(&aFont);

	CSize TextSize = pDC->GetTextExtent(pDoc->m_strAllotedNums);

	CSize scrollArea = CSize(TextSize.cx * pDoc->m_nMaxNum, TextSize.cy * pDoc->m_nLines);

	// Allow a margin
	scrollArea += CSize(20, 20); 

	SetScrollSizes(MM_LOENGLISH, scrollArea);

	
	// GetCharWidth32
	TEXTMETRIC tm;
	pDC->GetTextMetricsW(&tm);
	DWORD dwCharX = tm.tmAveCharWidth; 
    DWORD dwCharY = tm.tmHeight; 
	CString strSampleNums = _T("Sample Numbers: ") + pDoc->m_strSampleNums;
	int nSampleLen = strSampleNums.GetLength() * dwCharX * 1.2;
	const int nXMaxTextLen = 500;
	int nLineHeight = -TextSize.cy -5;
	CRect rcSmall (CPoint(20, -20), CPoint(700,-200));

	if ( nSampleLen < nXMaxTextLen ){
		rcSmall.SetRect(20, -20, nSampleLen + 5, nLineHeight -20 );
		
	}
	else {
		rcSmall.SetRect(20, -20, nXMaxTextLen, (nLineHeight -20 )* nSampleLen / nXMaxTextLen);
	}
	

	// pDC->TextOutW(10, -3, _T("Sample Numbers: ") + pDoc->m_strSampleNums);
	
	pDC->Rectangle(rcSmall);
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(strSampleNums, rcSmall, 
		DT_LEFT + DT_WORDBREAK);
	pDoc->m_nLines += 2;

	int nLinePos = - 200 - TextSize.cy;
	pDC->SetTextColor(RGB(0, 125, 250));
	// for ( int index = 0; index < pDoc->m_nLines; index++)
	//for ( vector<CString>::const_iterator p = pDoc->m_vstrNums.begin();
	//	p != pDoc->m_vstrNums.end(); ++p)
	for ( INT_PTR index = 0; index < pDoc->m_vstrNums.GetSize(); ++index)
	{

	// pDC->TextOutW(10, nLinePos, *p);
		pDC->TextOutW(10, nLinePos, pDoc->m_vstrNums.GetAt(index)); // 
	nLinePos -= TextSize.cy;
	pDoc->m_nLines += 1;
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
