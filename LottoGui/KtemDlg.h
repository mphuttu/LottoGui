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
	int m_nStakeIndex;
	BOOL m_bOnnenapila;

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
