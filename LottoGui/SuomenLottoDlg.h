#pragma once

#include "afxdialogex.h"

class CSuomenLottoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSuomenLottoDlg)

public:
	CSuomenLottoDlg(CWnd* pParent = NULL);
	virtual ~CSuomenLottoDlg();

	enum { IDD = IDD_SUOMENLOTTO_DIALOG };

	int m_nPredictionRows;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
