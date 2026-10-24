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
	BOOL m_bPlusNumber;

	BOOL m_bBalanced;
	BOOL m_bDelta;
	BOOL m_bWheel;
	BOOL m_bUnusual;
	BOOL m_bConsecutive;
	BOOL m_bMagicSum;
	BOOL m_bMatrix;
	BOOL m_bCoverage;
	CString m_strWheelNums;
	afx_msg void OnClickedPlusNumber();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	void UpdateLinePrice();

	DECLARE_MESSAGE_MAP()
};
