#include "stdafx.h"
#include "LottoGui.h"
#include "KenoDlg.h"
#include "LotteryPrices.h"

IMPLEMENT_DYNAMIC(CKenoDlg, CDialogEx)

CKenoDlg::CKenoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CKenoDlg::IDD, pParent)
	, m_nPredictionRows(5)
	, m_nLevelIndex(LotteryPrices::kKenoMaximumLevel - LotteryPrices::kKenoMinimumLevel)
	, m_nStakeIndex((LotteryPrices::kKenoDefaultStakeCents - LotteryPrices::kKenoMinimumStakeCents) /
		LotteryPrices::kStakeStepCents)
	, m_bKunkkukerroin(FALSE)
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

CKenoDlg::~CKenoDlg()
{
}

void CKenoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_KENO_ROWS, m_nPredictionRows);
	DDV_MinMaxInt(pDX, m_nPredictionRows, 1, 50);
	DDX_CBIndex(pDX, IDC_KENO_LEVEL, m_nLevelIndex);
	DDX_CBIndex(pDX, IDC_KENO_STAKE, m_nStakeIndex);
	DDX_Check(pDX, IDC_KENO_KUNKKUKERROIN, m_bKunkkukerroin);
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

BEGIN_MESSAGE_MAP(CKenoDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CKenoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CComboBox* level = static_cast<CComboBox*>(GetDlgItem(IDC_KENO_LEVEL));
	for (int picks = LotteryPrices::kKenoMinimumLevel; picks <= LotteryPrices::kKenoMaximumLevel; ++picks)
	{
		CString label;
		label.Format(L"%d", picks);
		level->AddString(label);
	}
	level->SetCurSel(m_nLevelIndex);

	CComboBox* stake = static_cast<CComboBox*>(GetDlgItem(IDC_KENO_STAKE));
	LotteryPrices::AddStakeOptions(stake,
		LotteryPrices::kKenoMinimumStakeCents, LotteryPrices::kKenoMaximumStakeCents);
	stake->SetCurSel(m_nStakeIndex);
	return TRUE;
}
