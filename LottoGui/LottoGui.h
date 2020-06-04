
// LottoGui.h : main header file for the LottoGui application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CLottoGuiApp:
// See LottoGui.cpp for the implementation of this class
//

class CLottoGuiApp : public CWinApp
{
public:
	CLottoGuiApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
//	afx_msg void OnLottooptions();
};

extern CLottoGuiApp theApp;
