#include "stdafx.h"
#include "LottoGui.h"
#include "JokeriDlg.h"
#include "LotteryPrices.h"

IMPLEMENT_DYNAMIC(CJokeriDlg, CDialogEx)

CJokeriDlg::CJokeriDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CJokeriDlg::IDD, pParent)
	, m_nPredictionRows(5)
	, m_bRepeatProfile(FALSE)
	, m_bDouble(FALSE)
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
	DDX_Check(pDX, IDC_JOKERI_REPEAT_PROFILE, m_bRepeatProfile);
	DDX_Check(pDX, IDC_JOKERI_DOUBLE, m_bDouble);
}

BEGIN_MESSAGE_MAP(CJokeriDlg, CDialogEx)
	ON_BN_CLICKED(IDC_JOKERI_DOUBLE, &CJokeriDlg::OnClickedDouble)
END_MESSAGE_MAP()

BOOL CJokeriDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	UpdateLinePrice();
	return TRUE;
}

void CJokeriDlg::UpdateLinePrice()
{
	const BOOL doubled = IsDlgButtonChecked(IDC_JOKERI_DOUBLE);
	const LPCTSTR option = (PRIMARYLANGID(::GetThreadLocale()) == LANG_FINNISH)
		? L"Tuplaus" : L"Double";
	GetDlgItem(IDC_JOKERI_DOUBLE)->SetWindowText(
		LotteryPrices::FormatOptionPrice(option, LotteryPrices::kJokeriDoubleCents));
	const int cents = LotteryPrices::kJokeriBaseCents +
		(doubled ? LotteryPrices::kJokeriDoubleCents : 0);
	GetDlgItem(IDC_JOKERI_PRICE)->SetWindowText(
		LotteryPrices::FormatLinePrice(cents));
}

void CJokeriDlg::OnClickedDouble()
{
	UpdateLinePrice();
}
