#include "stdafx.h"
#include "LottoGui.h"
#include "MilliDlg.h"

IMPLEMENT_DYNAMIC(CMilliDlg, CDialogEx)

CMilliDlg::CMilliDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMilliDlg::IDD, pParent)
	, m_nPredictionRows(5)
{
}

CMilliDlg::~CMilliDlg()
{
}

void CMilliDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MILLI_ROWS, m_nPredictionRows);
	DDV_MinMaxInt(pDX, m_nPredictionRows, 1, 50);
}

BEGIN_MESSAGE_MAP(CMilliDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CMilliDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	return TRUE;
}
