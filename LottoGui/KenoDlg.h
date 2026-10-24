#pragma once

#include "afxdialogex.h"

class CKenoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CKenoDlg)

public:
	CKenoDlg(CWnd* pParent = NULL);
	virtual ~CKenoDlg();

	enum { IDD = IDD_KENO_DIALOG };

	int m_nPredictionRows;
	int m_nLevelIndex;
	int m_nStakeIndex;
	BOOL m_bKunkkukerroin;

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
