#pragma once

#include "afxdialogex.h"

class CMilliDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMilliDlg)

public:
	CMilliDlg(CWnd* pParent = NULL);
	virtual ~CMilliDlg();

	enum { IDD = IDD_MILLI_DIALOG };

	int m_nPredictionRows;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
