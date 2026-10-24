#include "stdafx.h"
#include "LottoGui.h"
#include "EurojackpotDlg.h"
#include "LotteryPrices.h"

IMPLEMENT_DYNAMIC(CEurojackpotDlg, CDialogEx)

CEurojackpotDlg::CEurojackpotDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEurojackpotDlg::IDD, pParent)
	, m_nPredictionRows(5)
	, m_bBalanced(FALSE)
	, m_bDelta(FALSE)
	, m_bWheel(FALSE)
	, m_bUnusual(FALSE)
	, m_bConsecutive(FALSE)
	, m_bMagicSum(FALSE)
	, m_bMatrix(FALSE)
	, m_bCoverage(FALSE)
	, m_strWheelNums(_T(""))
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
	DDX_Check(pDX, IDC_SL_BALANCED, m_bBalanced);
	DDX_Check(pDX, IDC_SL_DELTA, m_bDelta);
	DDX_Check(pDX, IDC_SL_WHEEL, m_bWheel);
	DDX_Text(pDX, IDC_SL_WHEEL_NUMS, m_strWheelNums);
	DDX_Check(pDX, IDC_SL_UNUSUAL, m_bUnusual);
	DDX_Check(pDX, IDC_SL_CONSECUTIVE, m_bConsecutive);
	DDX_Check(pDX, IDC_SL_MAGIC_SUM, m_bMagicSum);
	DDX_Check(pDX, IDC_SL_MATRIX, m_bMatrix);
	DDX_Check(pDX, IDC_SL_COVERAGE, m_bCoverage);
}

BEGIN_MESSAGE_MAP(CEurojackpotDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CEurojackpotDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	GetDlgItem(IDC_EUROJACKPOT_PRICE)->SetWindowText(
		LotteryPrices::FormatLinePrice(LotteryPrices::kEurojackpotBaseCents));
	return TRUE;
}
