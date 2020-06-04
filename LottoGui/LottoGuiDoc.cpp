
// LottoGuiDoc.cpp : implementation of the CLottoGuiDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "LottoGui.h"
#endif

#include "LottoGuiDoc.h"
#include "LottoDialog.h"

#include <propkey.h>

#include <vector>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <unordered_set>
#include <algorithm>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CLottoGuiDoc

IMPLEMENT_DYNCREATE(CLottoGuiDoc, CDocument)

BEGIN_MESSAGE_MAP(CLottoGuiDoc, CDocument)
	ON_COMMAND(ID_LOTTOOPTIONS, &CLottoGuiDoc::OnLottooptions)
END_MESSAGE_MAP()


// CLottoGuiDoc construction/destruction

CLottoGuiDoc::CLottoGuiDoc()
{
	// TODO: add one-time construction code here
	m_nAmount = 7;
	m_nRounds = 1;
	m_strNums = _T("1 3 12 14 16 17 18 23 24 26 30 37");
	m_strAllotedNums = _T("1 3 12 17 23 26 37");
	m_nMinNum = 1;
	m_nMaxNum = 40;
	m_strSampleNums = _T("");
	m_nLines = 0;
}

CLottoGuiDoc::~CLottoGuiDoc()
{
}

BOOL CLottoGuiDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CLottoGuiDoc serialization

void CLottoGuiDoc::Serialize(CArchive& ar)
{
	 m_vstrNums.Serialize(ar);

	if (ar.IsStoring())
	{
		// TODO: add storing code here
		ar << m_strSampleNums;
		ar << m_nLines;
		// ar << m_vstrNums;
	}
	else
	{
		// TODO: add loading code here
		ar >> m_strSampleNums;
		ar >> m_nLines;
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CLottoGuiDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
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
void CLottoGuiDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CLottoGuiDoc::SetSearchContent(const CString& value)
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

// CLottoGuiDoc diagnostics

#ifdef _DEBUG
void CLottoGuiDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CLottoGuiDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CLottoGuiDoc commands


void CLottoGuiDoc::OnLottooptions()
{
	// TODO: Add your command handler code here
	CLottoDialog aDlg; 

	aDlg.m_nMinNum = m_nMinNum;
	aDlg.m_nMaxNum = m_nMaxNum;
	aDlg.m_nAmount = m_nAmount;
	aDlg.m_nRounds = m_nRounds;
	aDlg.m_strNums = m_strNums;

	int i=0, j=0, k=0;
	int n = 0;

	// Open the dialog
	if ( aDlg.DoModal() )
	{
		m_nMinNum = aDlg.m_nMinNum;
		m_nMaxNum = aDlg.m_nMaxNum;
		m_nAmount = aDlg.m_nAmount;
		m_nRounds = aDlg.m_nRounds;
		m_strNums = aDlg.m_strNums;
		m_strSampleNums = aDlg.m_strNums;

		m_nLines = m_nRounds;

		vector<int> input;
		
		
		if (aDlg.m_bGivenNumber == TRUE )
		{	
			extractIntegerWords(m_strNums, input);			 
			
		}
		
		
		if (aDlg.m_bGivenNumber == FALSE )
		{
			m_strSampleNums = L"";

			n = (m_nMaxNum - m_nMinNum) + 1;

			
			for (j = m_nMinNum; j < m_nMaxNum + 1; j++){
				input.push_back(j);
				CString strTmp;
				strTmp.Format(L"%d ", j);
				m_strSampleNums += strTmp;
			}
			
		}
		n = input.size();
		int* arr = new int[n];

			 for ( i=0; i < n; i++)
			arr[i] = input[i];

		
		
		// Array for lotto numbers
		vector<int> lottoline;

		srand( time(0) );

		time_t now = time(NULL);
		CString strTemp;

		m_strAllotedNums =L"";
		for ( k =0; k < m_nRounds; k++)
		{
			m_strAllotedNums = "";
			strTemp.Format(L"%d", k+1);
			m_strAllotedNums += _T("Line ") + strTemp + _T(": ");
			// lottoline.clear();

			for ( i = 0; i < m_nAmount; i++)
			{
				// Allot an index
				int number = rand() % n;

				// The first round
				if ( i == 0 ) 
				{
					lottoline.push_back(arr[number]);
					// strTemp.Format(L"%d", arr[number] );
					// m_strAllotedNums += strTemp;
				}

				// Rounds after the first one
				if ( i > 0 )
				{
					vector <int>::iterator it = find( lottoline.begin(), lottoline.end(), arr[number] );
					if ( it == lottoline.end() ){
						lottoline.push_back(arr[number]);
						// strTemp.Format(L" %d", arr[number] );
					// m_strAllotedNums += strTemp;
					}
					else
					{
						do {
					number = rand() % n;
					it = find( lottoline.begin(), lottoline.end(), arr[number] );
						} while( it != lottoline.end() );
						lottoline.push_back(arr[number]);
						// strTemp.Format(L" %d", arr[number] );
					// m_strAllotedNums += strTemp;
					} // end of else
				}

				if ( i == m_nAmount - 1 ) {
					// Sort the numbers in ascending order
					sort (lottoline.begin(), lottoline.end() );
				
				ostringstream vts;

				if ( !lottoline.empty() )
					copy (lottoline.begin(), lottoline.end() -1, 
					ostream_iterator<int>(vts, ", " ) );

				// Now add the last element with no delimimter
				vts << lottoline.back();

				 m_strAllotedNums +=  vts.str().c_str();
				 // m_vstrNums.push_back(m_strAllotedNums);
				 m_vstrNums.Add(m_strAllotedNums);
				// m_strAllotedNums = L"";
				lottoline.clear();
				} // end of if ( i == m_nAmount -1 ) ...
			} // end of i = 0...

		} // end of k = 0...


		delete[] arr;
		UpdateAllViews(NULL);
		SetModifiedFlag();
	}
}

void extractIntegerWords(CString str, vector <int> &vec)
{
	stringstream ss;
	// Storing the whow string into string stream
	CT2CA pszConvertedAnsiString (str);
	string strStd (pszConvertedAnsiString); 
	ss << strStd;

	// running loop till the end of the strawm
	string temp;
	int found;
	while (!ss.eof() )
	{
		// extracting word by word from stream
		ss >> temp;
		// checking if the given word is integer or not
		if (stringstream(temp) >> found)
			vec.push_back(found);
	}

}

void CLottoGuiDoc::DeleteContents()
{
	// TODO: Add your specialized code here and/or call the base class
	m_nLines = 0;
	m_strSampleNums = L"";
	// m_vstrNums.clear();
	m_vstrNums.RemoveAll();

	CDocument::DeleteContents();
}
