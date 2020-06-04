// LottoDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LottoGui.h"
#include "LottoDialog.h"
#include "afxdialogex.h"


// CLottoDialog dialog

IMPLEMENT_DYNAMIC(CLottoDialog, CDialog)

CLottoDialog::CLottoDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CLottoDialog::IDD, pParent)
	, m_nMinNum(0)
	, m_nMaxNum(0)
	, m_nAmount(0)
	, m_nRounds(0)
	, m_strNums(_T(""))
	, m_bGivenNumber(FALSE)
{
	
}

CLottoDialog::~CLottoDialog()
{
}

void CLottoDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FROMEDIT, m_nMinNum);
	DDV_MinMaxInt(pDX, m_nMinNum, 1, 50);
	DDX_Text(pDX, IDC_TOEDIT, m_nMaxNum);
	DDV_MinMaxInt(pDX, m_nMaxNum, 2, 50);
	DDX_Text(pDX, IDC_AMOUNTEDIT, m_nAmount);
	DDV_MinMaxInt(pDX, m_nAmount, 1, 10000);
	DDX_Text(pDX, IDC_ROUNDSEDIT, m_nRounds);
	DDV_MinMaxInt(pDX, m_nRounds, 1, 20);
	DDX_Text(pDX, IDC_NUMSEDIT, m_strNums);
	DDX_Check(pDX, IDC_GIVENUMCHECK, m_bGivenNumber);
}


BEGIN_MESSAGE_MAP(CLottoDialog, CDialog)
	ON_BN_CLICKED(IDC_GIVENUMCHECK, &CLottoDialog::OnClickedGivenumcheck)
	ON_BN_CLICKED(IDC_USERANGECHECK, &CLottoDialog::OnClickedUserangecheck)
END_MESSAGE_MAP()


// CLottoDialog message handlers


void CLottoDialog::OnClickedGivenumcheck()
{
	// TODO: Add your control notification handler code here
	// Get a pointer to each of the check box objects
	CButton* pRange = (CButton*) GetDlgItem( IDC_USERANGECHECK);
	CButton* pGiveNum = (CButton*) GetDlgItem (IDC_GIVENUMCHECK);

	// If give own numbers is checked, uncheck number range
	if ( pGiveNum->GetCheck() ){
		pRange->SetCheck(0);
		m_bGivenNumber = TRUE;
	}
}


void CLottoDialog::OnClickedUserangecheck()
{
	// TODO: Add your control notification handler code here
	// Get a pointer to each of the check box objects
	CButton* pRange = (CButton*) GetDlgItem( IDC_USERANGECHECK);
	CButton* pGiveNum = (CButton*) GetDlgItem (IDC_GIVENUMCHECK);

	// If give own numbers is checked, uncheck number range
	if ( pRange->GetCheck() ) {
		pGiveNum->SetCheck(0);
		m_bGivenNumber = FALSE;
	}
}


BOOL CLottoDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	CButton* pGiveNum = (CButton*) GetDlgItem(IDC_GIVENUMCHECK);
	pGiveNum->SetCheck(1);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
