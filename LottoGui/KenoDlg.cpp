#include "stdafx.h"
#include "LottoGui.h"
#include "KenoDlg.h"

IMPLEMENT_DYNAMIC(CKenoDlg, CDialogEx)

CKenoDlg::CKenoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CKenoDlg::IDD, pParent)
	, m_nPredictionRows(5)
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
}

BEGIN_MESSAGE_MAP(CKenoDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CKenoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	return TRUE;
}
