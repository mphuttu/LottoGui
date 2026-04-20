#pragma once

#include "afxdialogex.h"

class CJokeriDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CJokeriDlg)

public:
	CJokeriDlg(CWnd* pParent = NULL);
	virtual ~CJokeriDlg();

	enum { IDD = IDD_JOKERI_DIALOG };

	int m_nPredictionRows;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
