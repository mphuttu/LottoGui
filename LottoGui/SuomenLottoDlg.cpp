#include "stdafx.h"
#include "LottoGui.h"
#include "SuomenLottoDlg.h"
#include "LotteryPrices.h"

IMPLEMENT_DYNAMIC(CSuomenLottoDlg, CDialogEx)

CSuomenLottoDlg::CSuomenLottoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSuomenLottoDlg::IDD, pParent)
	, m_nPredictionRows(5)
	, m_bPlus(FALSE)
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

CSuomenLottoDlg::~CSuomenLottoDlg()
{
}

void CSuomenLottoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SUOMENLOTTO_ROWS, m_nPredictionRows);
	DDV_MinMaxInt(pDX, m_nPredictionRows, 1, 50);
	DDX_Check(pDX, IDC_SUOMENLOTTO_PLUS, m_bPlus);
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

BEGIN_MESSAGE_MAP(CSuomenLottoDlg, CDialogEx)
	ON_BN_CLICKED(IDC_SUOMENLOTTO_PLUS, &CSuomenLottoDlg::OnClickedPlus)
END_MESSAGE_MAP()

BOOL CSuomenLottoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	UpdateLinePrice();
	return TRUE;
}

void CSuomenLottoDlg::UpdateLinePrice()
{
	const BOOL plus = IsDlgButtonChecked(IDC_SUOMENLOTTO_PLUS);
	GetDlgItem(IDC_SUOMENLOTTO_PLUS)->SetWindowText(
		LotteryPrices::FormatOptionPrice(L"Plus", LotteryPrices::kSuomenLottoPlusCents));
	const int cents = LotteryPrices::kSuomenLottoBaseCents +
		(plus ? LotteryPrices::kSuomenLottoPlusCents : 0);
	GetDlgItem(IDC_SUOMENLOTTO_PRICE)->SetWindowText(
		LotteryPrices::FormatLinePrice(cents));
}

void CSuomenLottoDlg::OnClickedPlus()
{
	UpdateLinePrice();
}
