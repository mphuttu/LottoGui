#include "stdafx.h"
#include "LottoGui.h"
#include "VikingLottoDlg.h"
#include "LotteryPrices.h"

IMPLEMENT_DYNAMIC(CVikingLottoDlg, CDialogEx)

CVikingLottoDlg::CVikingLottoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVikingLottoDlg::IDD, pParent)
	, m_nPredictionRows(5)
	, m_bPlusNumber(FALSE)
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

CVikingLottoDlg::~CVikingLottoDlg()
{
}

void CVikingLottoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_VIKINGLOTTO_ROWS, m_nPredictionRows);
	DDV_MinMaxInt(pDX, m_nPredictionRows, 1, 50);
	DDX_Check(pDX, IDC_VIKINGLOTTO_PLUS, m_bPlusNumber);
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

BEGIN_MESSAGE_MAP(CVikingLottoDlg, CDialogEx)
	ON_BN_CLICKED(IDC_VIKINGLOTTO_PLUS, &CVikingLottoDlg::OnClickedPlusNumber)
END_MESSAGE_MAP()

BOOL CVikingLottoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	UpdateLinePrice();
	return TRUE;
}

void CVikingLottoDlg::UpdateLinePrice()
{
	const BOOL plusNumber = IsDlgButtonChecked(IDC_VIKINGLOTTO_PLUS);
	const LPCTSTR option = (PRIMARYLANGID(::GetThreadLocale()) == LANG_FINNISH)
		? L"Plusnumero" : L"Plus number";
	GetDlgItem(IDC_VIKINGLOTTO_PLUS)->SetWindowText(
		LotteryPrices::FormatOptionPrice(option, LotteryPrices::kVikingLottoPlusCents));
	const int cents = LotteryPrices::kVikingLottoBaseCents +
		(plusNumber ? LotteryPrices::kVikingLottoPlusCents : 0);
	GetDlgItem(IDC_VIKINGLOTTO_PRICE)->SetWindowText(
		LotteryPrices::FormatLinePrice(cents));
}

void CVikingLottoDlg::OnClickedPlusNumber()
{
	UpdateLinePrice();
}
