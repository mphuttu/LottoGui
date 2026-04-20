#include "stdafx.h"
#include "LottoGui.h"
#include "SuomenLottoDlg.h"

IMPLEMENT_DYNAMIC(CSuomenLottoDlg, CDialogEx)

CSuomenLottoDlg::CSuomenLottoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSuomenLottoDlg::IDD, pParent)
	, m_nPredictionRows(5)
{
}

CSuomenLottoDlg::~CSuomenLottoDlg()
{
}

void CSuomenLottoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SUOMENLOTTO_ROWS, m_nPredictionRows);
	DDV_MinMaxInt(pDX, m_nPredictionRows, 1, 50);
}

BEGIN_MESSAGE_MAP(CSuomenLottoDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CSuomenLottoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	return TRUE;
}
