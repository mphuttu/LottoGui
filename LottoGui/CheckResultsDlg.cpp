// CheckResultsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LottoGui.h"
#include "CheckResultsDlg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CCheckResultsDlg, CDialog)

CCheckResultsDlg::CCheckResultsDlg(CWnd* pParent)
	: CDialog(CCheckResultsDlg::IDD, pParent)
	, m_strActualNumbers(_T(""))
{
}

CCheckResultsDlg::~CCheckResultsDlg()
{
}

void CCheckResultsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_CHECK_RESULTS_NUMBERS, m_strActualNumbers);
}

BEGIN_MESSAGE_MAP(CCheckResultsDlg, CDialog)
END_MESSAGE_MAP()

BOOL CCheckResultsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	return TRUE;
}
