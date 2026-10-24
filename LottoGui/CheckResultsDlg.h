#pragma once

class CCheckResultsDlg : public CDialog
{
	DECLARE_DYNAMIC(CCheckResultsDlg)

public:
	CCheckResultsDlg(CWnd* pParent = NULL);
	virtual ~CCheckResultsDlg();

	// Dialog Data
	enum { IDD = IDD_CHECK_RESULTS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

public:
	CString m_strActualNumbers;
};
