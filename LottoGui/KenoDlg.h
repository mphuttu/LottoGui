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

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
