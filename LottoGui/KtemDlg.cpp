#include "stdafx.h"
#include "LottoGui.h"
#include "KtemDlg.h"
#include "LotteryPrices.h"

IMPLEMENT_DYNAMIC(CKtemDlg, CDialogEx)

CKtemDlg::CKtemDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CKtemDlg::IDD, pParent)
	, m_nPredictionRows(5)
	, m_nStakeIndex(0)
	, m_bOnnenapila(FALSE)
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

CKtemDlg::~CKtemDlg()
{
}

void CKtemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_KTEM_ROWS, m_nPredictionRows);
	DDV_MinMaxInt(pDX, m_nPredictionRows, 1, 50);
	DDX_CBIndex(pDX, IDC_KTEM_STAKE, m_nStakeIndex);
	DDX_Check(pDX, IDC_KTEM_ONNENAPILA, m_bOnnenapila);
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

BEGIN_MESSAGE_MAP(CKtemDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CKtemDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CComboBox* stake = static_cast<CComboBox*>(GetDlgItem(IDC_KTEM_STAKE));
	LotteryPrices::AddStakeOptions(stake,
		LotteryPrices::kKtemMinimumStakeCents, LotteryPrices::kKtemMaximumStakeCents);
	UpdateData(FALSE);
	return TRUE;
}
