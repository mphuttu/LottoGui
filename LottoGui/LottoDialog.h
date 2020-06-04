#pragma once


// CLottoDialog dialog

class CLottoDialog : public CDialog
{
	DECLARE_DYNAMIC(CLottoDialog)

public:
	CLottoDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLottoDialog();

// Dialog Data
	enum { IDD = IDD_LOTTODIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_nMinNum;
	int m_nMaxNum;
	int m_nAmount;
	int m_nRounds;
	CString m_strNums;
	afx_msg void OnClickedGivenumcheck();
	afx_msg void OnClickedUserangecheck();
	virtual BOOL OnInitDialog();
	BOOL m_bGivenNumber;
};
