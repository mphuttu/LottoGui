#include "stdafx.h"
#include "LottoGui.h"
#include "JokeriDlg.h"

IMPLEMENT_DYNAMIC(CJokeriDlg, CDialogEx)

CJokeriDlg::CJokeriDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CJokeriDlg::IDD, pParent)
	, m_nPredictionRows(5)
{
}

CJokeriDlg::~CJokeriDlg()
{
}

void CJokeriDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_JOKERI_ROWS, m_nPredictionRows);
	DDV_MinMaxInt(pDX, m_nPredictionRows, 1, 50);
}

BEGIN_MESSAGE_MAP(CJokeriDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CJokeriDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	return TRUE;
}
