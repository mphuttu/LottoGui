#pragma once

#include "afxdialogex.h"

class CEurojackpotDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEurojackpotDlg)

public:
	CEurojackpotDlg(CWnd* pParent = NULL);
	virtual ~CEurojackpotDlg();

	enum { IDD = IDD_EUROJACKPOT_DIALOG };

	int m_nPredictionRows;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
