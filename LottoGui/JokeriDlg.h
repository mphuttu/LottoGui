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
	BOOL m_bRepeatProfile;
	BOOL m_bDouble;
	afx_msg void OnClickedDouble();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	void UpdateLinePrice();

	DECLARE_MESSAGE_MAP()
};
