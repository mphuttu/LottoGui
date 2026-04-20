#include "stdafx.h"
#include "LottoGui.h"
#include "VikingLottoDlg.h"

IMPLEMENT_DYNAMIC(CVikingLottoDlg, CDialogEx)

CVikingLottoDlg::CVikingLottoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVikingLottoDlg::IDD, pParent)
	, m_nPredictionRows(5)
{
}

CVikingLottoDlg::~CVikingLottoDlg()
{
}

void CVikingLottoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_VIKINGLOTTO_ROWS, m_nPredictionRows);
	DDV_MinMaxInt(pDX, m_nPredictionRows, 1, 50);
}

BEGIN_MESSAGE_MAP(CVikingLottoDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CVikingLottoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	return TRUE;
}
