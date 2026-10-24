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
	BOOL m_bPlus;

	BOOL m_bBalanced;
	BOOL m_bDelta;
	BOOL m_bWheel;
	BOOL m_bUnusual;
	BOOL m_bConsecutive;
	BOOL m_bMagicSum;
	BOOL m_bMatrix;
	BOOL m_bCoverage;
	CString m_strWheelNums;
	afx_msg void OnClickedPlus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	void UpdateLinePrice();

	DECLARE_MESSAGE_MAP()
};
