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

	BOOL m_bBalanced;
	BOOL m_bDelta;
	BOOL m_bWheel;
	BOOL m_bUnusual;
	BOOL m_bConsecutive;
	BOOL m_bMagicSum;
	BOOL m_bMatrix;
	BOOL m_bCoverage;
	CString m_strWheelNums;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
