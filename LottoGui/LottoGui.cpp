
// LottoGui.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "LottoGui.h"
#include "MainFrm.h"

#include "LottoGuiDoc.h"
#include "LottoGuiView.h"
#include "LottoDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLottoGuiApp

BEGIN_MESSAGE_MAP(CLottoGuiApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CLottoGuiApp::OnAppAbout)
	ON_COMMAND(ID_HELP_USAGE, &CLottoGuiApp::OnHelpUsage)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)
//	ON_COMMAND(ID_LOTTOOPTIONS, &CLottoGuiApp::OnLottooptions)
END_MESSAGE_MAP()


// CLottoGuiApp construction

CLottoGuiApp::CLottoGuiApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("LottoGui.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CLottoGuiApp object

CLottoGuiApp theApp;


// CLottoGuiApp initialization

BOOL CLottoGuiApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)


	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CLottoGuiDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CLottoGuiView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);


	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();
	return TRUE;
}

int CLottoGuiApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	return CWinApp::ExitInstance();
}

// CLottoGuiApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

class CUsageHelpDlg : public CDialogEx
{
public:
	CUsageHelpDlg() : CDialogEx(IDD_HELPDIALOG)
	{
	}

protected:
	virtual BOOL OnInitDialog()
	{
		CDialogEx::OnInitDialog();

		CString helpText =
			L"LottoGui - Usage Instructions\r\n\r\n"
			L"1. Classic Lotto Options\r\n"
			L"   - Open Lotto Options from the menu.\r\n"
			L"   - Choose either a number range or your own input numbers.\r\n"
			L"   - Set how many numbers and rows you want to generate.\r\n\r\n"
			L"2. Suomen Lotto CSV\r\n"
			L"   - Reads historical data from data\\SuomenLottoData.csv.\r\n"
			L"   - Shows frequency-based predictions and column statistics.\r\n\r\n"
			L"3. Milli CSV\r\n"
			L"   - Reads historical data from data\\MilliData.csv.\r\n"
			L"   - Uses the six main numbers in the data for predictions and statistics.\r\n\r\n"
			L"4. Eurojackpot CSV\r\n"
			L"   - Reads main numbers and stars from data\\EurojackpotData.csv.\r\n"
			L"   - Shows predictions and statistical analysis for both sets.\r\n\r\n"
			L"5. Viking Lotto CSV\r\n"
			L"   - Reads historical main numbers from data\\VikingData.csv.\r\n"
			L"   - The Viking number is added randomly from 1 to 5 because no historical Viking-number data is stored.\r\n\r\n"
			L"6. Jokeri CSV\r\n"
			L"   - Reads historical Jokeri digits from data\\JokeriData.csv.\r\n"
			L"   - Repeated digits are allowed in a prediction row, just like in the real draw.\r\n"
			L"   - Harmonic mean is shown as N/A for columns containing zero values.\r\n\r\n"
			L"7. Viewing results\r\n"
			L"   - Predictions and statistics are shown in the main client area.\r\n"
			L"   - Results can be saved and printed with the standard File menu commands.";

		SetDlgItemText(IDC_HELP_TEXT, helpText);
		return TRUE;
	}

	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CUsageHelpDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CLottoGuiApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void CLottoGuiApp::OnHelpUsage()
{
	CUsageHelpDlg helpDlg;
	helpDlg.DoModal();
}

// CLottoGuiApp message handlers





//void CLottoGuiApp::OnLottooptions()
//{
//	// TODO: Add your command handler code here
//	CLottoDialog aDlg;
//	aDlg.DoModal();
//
//}
