#pragma once

#include "afxdialogex.h"

class CKtemDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CKtemDlg)

public:
	CKtemDlg(CWnd* pParent = NULL);
	virtual ~CKtemDlg();

	enum { IDD = IDD_KTEM_DIALOG };

	int m_nPredictionRows;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
