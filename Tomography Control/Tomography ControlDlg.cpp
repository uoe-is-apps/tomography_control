
// Tomography ControlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Tomography Control.h"
#include "TakingPhotosDlg.h"
#include "Tomography ControlDlg.h"
#include "Table And Camera Control.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CTomographyControlDlg dialog




CTomographyControlDlg::CTomographyControlDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTomographyControlDlg::IDD, pParent)
	, m_TableCommandOutput(_T(""))
	, m_TableCommand(_T(""))
	, m_MainImageName(_T(""))
	, m_ExposureTime(0)
	, m_FramesPerStop(0)
	, m_StopsPerRotation(0)
	, m_NumberOfTurns(0)
	, m_DelayBetweenTurnsSeconds(0)
	, m_TableInitialisationFile(_T(""))
	, m_ManualCameraControl(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTomographyControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TABLE_COMMANDS, m_TableCommandOutput);
	DDX_Control(pDX, IDC_EDIT_TABLE_COMMAND, m_TableCommandControl);
	DDX_Text(pDX, IDC_EDIT_TABLE_COMMAND, m_TableCommand);
	DDX_Text(pDX, IDC_EDIT_MAIN_NAME_IMAGE, m_MainImageName);
	DDX_Text(pDX, IDC_EDIT_EXPOSURE_TIME, m_ExposureTime);
	DDX_Text(pDX, IDC_EDIT_NUM_FRAMES_STOP, m_FramesPerStop);
	DDX_Text(pDX, IDC_EDIT_NUM_STOPS_360, m_StopsPerRotation);
	DDX_Text(pDX, IDC_EDIT_NUM_STOPS_361, m_NumberOfTurns);
	DDX_Text(pDX, IDC_EDIT_TURN_INTERVAL, m_DelayBetweenTurnsSeconds);
	DDX_Control(pDX, IDC_BUTTON_STOP_RUN_LOOP, m_StopRunLoopButton);
	DDX_Control(pDX, IDC_BUTTON_RUN_LOOP, m_RunLoopButton);
	DDX_Text(pDX, IDC_BROWSE_TABLE_INI, m_TableInitialisationFile);
	DDX_Text(pDX, IDC_BROWSE_CAMERA_INI, m_ManualCameraControl);
}

BEGIN_MESSAGE_MAP(CTomographyControlDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_INITIALISE_TABLE, &CTomographyControlDlg::OnBnClickedButtonInitialiseTable)
	ON_BN_CLICKED(IDC_BUTTON_TABLE_NRESET, &CTomographyControlDlg::OnBnClickedButtonTableNreset)
	ON_BN_CLICKED(IDC_BUTTON_TABLE_NCAL, &CTomographyControlDlg::OnBnClickedButtonTableNcal)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_TABLE_DISPLAY, &CTomographyControlDlg::OnBnClickedButtonClearTableDisplay)
	ON_BN_CLICKED(IDC_BUTTON_RESET_TABLE, &CTomographyControlDlg::OnBnClickedButtonResetTable)
	ON_BN_CLICKED(IDC_BUTTON_RUN_LOOP, &CTomographyControlDlg::OnBnClickedButtonRunLoop)
	ON_BN_CLICKED(IDC_BUTTON_STOP_RUN_LOOP, &CTomographyControlDlg::OnBnClickedButtonStopRunLoop)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA_WRITE_INITIAL, &CTomographyControlDlg::OnBnClickedButtonCameraWriteInitial)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA_TAKE_SINGLE, &CTomographyControlDlg::OnBnClickedButtonCameraTakeSingle)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA_TAKE_DARK, &CTomographyControlDlg::OnBnClickedButtonCameraTakeDark)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA_TAKE_FLAT, &CTomographyControlDlg::OnBnClickedButtonCameraTakeFlat)
END_MESSAGE_MAP()


// CTomographyControlDlg message handlers

BOOL CTomographyControlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_StopRunLoopButton.EnableWindow(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTomographyControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTomographyControlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTomographyControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// Catch return keypresses on table command field
BOOL CTomographyControlDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN &&
        pMsg->wParam == VK_RETURN/*  &&
        GetFocus() == this -> m_TableCommandControl */)
    {
		this -> UpdateData(TRUE);

		// Take a copy of the command, then wipe the field
		char *command = (char*)alloca(sizeof(char) * (this -> m_TableCommand.GetLength() + 1));
		strcpy(command, (LPCTSTR)m_TableCommand);
		this -> m_TableCommand.Empty();

		this -> tableAndCameraControl -> SendTableCommand(this, command);
		this -> m_TableCommand.Empty();
        return TRUE; // this doesn't need processing anymore
    }
    return FALSE; // all other cases still need default processing
}


void CTomographyControlDlg::OnBnClickedButtonInitialiseTable()
{
	this -> UpdateData(TRUE);
	// TODO: Add your control notification handler code here
	// Get filename from browser
	// Load file from disk
	// Foreach line, send to table
}


void CTomographyControlDlg::OnBnClickedButtonClearTableDisplay()
{
	this -> UpdateData(TRUE);
	this -> m_TableCommandOutput.Empty();
	this -> UpdateData(FALSE);
}


void CTomographyControlDlg::OnBnClickedButtonResetTable()
{
	this -> UpdateData(TRUE);

	printf("%d\r\n", this -> m_ExposureTime);

	// TODO: Add your control notification handler code here
	this -> m_TableCommandOutput.Empty();
	this -> UpdateData(FALSE);
}


void CTomographyControlDlg::OnBnClickedButtonTableNreset()
{
	this -> UpdateData(TRUE);
	this -> tableAndCameraControl -> SendTableCommand(this, "nreset");
}


void CTomographyControlDlg::OnBnClickedButtonTableNcal()
{
	this -> UpdateData(TRUE);
	this -> tableAndCameraControl -> SendTableCommand(this, "ncal");
}


void CTomographyControlDlg::OnBnClickedButtonRunLoop()
{
	// TODO: Add your control notification handler code here
	m_RunLoopButton.EnableWindow(FALSE);
	m_StopRunLoopButton.EnableWindow(TRUE);
}


void CTomographyControlDlg::OnBnClickedButtonStopRunLoop()
{
	// TODO: Add your control notification handler code here
	m_RunLoopButton.EnableWindow(TRUE);
	m_StopRunLoopButton.EnableWindow(FALSE);
}


void CTomographyControlDlg::OnBnClickedButtonCameraWriteInitial()
{
	// TODO: Add your control notification handler code here
}


void CTomographyControlDlg::OnBnClickedButtonCameraTakeSingle()
{
	// TODO: Add your control notification handler code here
	CTakingPhotosDlg takingPhotosDlg;

	takingPhotosDlg.DoModal();
}


void CTomographyControlDlg::OnBnClickedButtonCameraTakeDark()
{
	// TODO: Add your control notification handler code here
}


void CTomographyControlDlg::OnBnClickedButtonCameraTakeFlat()
{
	// TODO: Add your control notification handler code here
}
