
// LottoGuiDoc.h : interface of the CLottoGuiDoc class
//


#pragma once

#include <vector>
using namespace std;

class CLottoGuiDoc : public CDocument
{
protected: // create from serialization only
	CLottoGuiDoc();
	DECLARE_DYNCREATE(CLottoGuiDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CLottoGuiDoc();
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
	afx_msg void OnLottooptions();
	int m_nAmount;
	int m_nRounds;
	CString m_strNums;
	CString m_strAllotedNums;
	int m_nMinNum;
	int m_nMaxNum;
	CString m_strSampleNums;
	int m_nLines;
	// vector<CString> m_vstrNums;
	CStringArray m_vstrNums;
	virtual void DeleteContents();
};

void extractIntegerWords(CString str, vector <int> &vec);