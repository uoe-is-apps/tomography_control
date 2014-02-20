
// Tomography Control.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"

#include "ShadOCam.h"
#include "Tomography Control.h"
#include "Tomography ControlDlg.h"
#include "Table And Camera Control.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTomographyControlApp

BEGIN_MESSAGE_MAP(CTomographyControlApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CTomographyControlApp construction

CTomographyControlApp::CTomographyControlApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CTomographyControlApp object

CTomographyControlApp theApp;


// CTomographyControlApp initialization

BOOL CTomographyControlApp::InitInstance()
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


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("University of Edinburgh"));
	
	ShadOCam* shadOCam = NULL;
	CTomographyControlDlg controlDialogue;

	controlDialogue.tableAndCameraControl = new CTableAndCameraControl();

	try {
		shadOCam = new ShadOCam("C:\\ShadoCam\\IniFile.txt");
	}
	catch(char* message)
	{
		MessageBox(NULL, message, "Tomography Control", MB_ICONERROR);
		// TODO: Fail
	}

	m_pMainWnd = &controlDialogue;
	INT_PTR nResponse = controlDialogue.DoModal();

	if (NULL != shadOCam)
	{
		delete shadOCam;
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}