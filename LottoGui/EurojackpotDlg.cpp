#include "stdafx.h"
#include "LottoGui.h"
#include "EurojackpotDlg.h"

IMPLEMENT_DYNAMIC(CEurojackpotDlg, CDialogEx)

CEurojackpotDlg::CEurojackpotDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEurojackpotDlg::IDD, pParent)
	, m_nPredictionRows(5)
{
}

CEurojackpotDlg::~CEurojackpotDlg()
{
}

void CEurojackpotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EUROJACKPOT_ROWS, m_nPredictionRows);
	DDV_MinMaxInt(pDX, m_nPredictionRows, 1, 50);
}

BEGIN_MESSAGE_MAP(CEurojackpotDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CEurojackpotDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	return TRUE;
}
