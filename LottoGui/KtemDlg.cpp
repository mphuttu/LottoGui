#include "stdafx.h"
#include "LottoGui.h"
#include "KtemDlg.h"

IMPLEMENT_DYNAMIC(CKtemDlg, CDialogEx)

CKtemDlg::CKtemDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CKtemDlg::IDD, pParent)
	, m_nPredictionRows(5)
{
}

CKtemDlg::~CKtemDlg()
{
}

void CKtemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_KTEM_ROWS, m_nPredictionRows);
	DDV_MinMaxInt(pDX, m_nPredictionRows, 1, 50);
}

BEGIN_MESSAGE_MAP(CKtemDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CKtemDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	return TRUE;
}
