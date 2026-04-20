#pragma once

#include "afxdialogex.h"

class CVikingLottoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CVikingLottoDlg)

public:
	CVikingLottoDlg(CWnd* pParent = NULL);
	virtual ~CVikingLottoDlg();

	enum { IDD = IDD_VIKINGLOTTO_DIALOG };

	int m_nPredictionRows;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
