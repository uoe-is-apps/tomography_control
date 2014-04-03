
// Tomography ControlDlg.cpp : implementation file
//

#include "stdafx.h"

#include <direct.h>
#include <errno.h>
#include <io.h>
#include <windows.h>
#include <string.h>

#include "Tomography Control.h"
#include "Tomography ControlDlg.h"
#include "TakingPhotosDlg.h"
#include "Exceptions.h"
#include "Camera.h"
#include "RunProgressDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ACCESS_MODE_READ_ONLY	4

#define FILE_BUFFER_SIZE		2048

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
	, m_tableCommandOutput(_T(""))
	, m_tableCommand(_T(""))
	, m_exposureTimeSeconds(1.5f)
	, m_framesPerStop(1)
	, m_stopsPerRotation(100)
	, m_turnsTotal(1)
	, m_delayBetweenTurnsSeconds(1)
	, m_tableInitialisationFile(_T(""))
	, m_cameraType(0)
	, m_tableType(0)
	, m_frameSavingOptions(0)
	, m_researcherName(_T(""))
	, m_sampleName(_T(""))
	, m_timestamp(_T(""))
	, m_numImages(1)
	, m_perkinElmerMode(1)
{
	// Fill in a sensible timestamp
	time_t rawtime;
	struct tm timeinfo;
	char timestampBuffer[TIMESTAMP_BUFFER_SIZE];

	time (&rawtime);
	localtime_s(&timeinfo, &rawtime);

	strftime(timestampBuffer, TIMESTAMP_BUFFER_SIZE, "%Y-%m-%d_%H_%M", &timeinfo);
	this -> m_timestamp.Append(timestampBuffer);

	// Guess at location of airtable file
	if (FAILED(SHGetFolderPath(NULL, 
        CSIDL_PERSONAL|CSIDL_FLAG_CREATE, 
        NULL, 
        0, 
        this -> m_directoryPathBuffer)))
	{
		throw bad_directory_error("Could not retrieve user home directory.");
	}
	
	PathAppend(this -> m_directoryPathBuffer, "..");
	PathAppend(this -> m_directoryPathBuffer, "Desktop");
	PathAppend(this -> m_directoryPathBuffer, "airtable.txt");
	this -> m_tableInitialisationFile += this -> m_directoryPathBuffer;

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTomographyControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TABLE_COMMANDS, m_tableCommandOutput);
	DDX_Control(pDX, IDC_EDIT_TABLE_COMMAND, m_tableCommandControl);
	DDX_Text(pDX, IDC_EDIT_TABLE_COMMAND, m_tableCommand);
	DDX_Text(pDX, IDC_EDIT_EXPOSURE_TIME, m_exposureTimeSeconds);
	DDX_Text(pDX, IDC_EDIT_NUM_FRAMES_STOP, m_framesPerStop);
	DDX_Text(pDX, IDC_EDIT_NUM_STOPS_360, m_stopsPerRotation);
	DDX_Text(pDX, IDC_EDIT_NUM_STOPS_361, m_turnsTotal);
	DDX_Text(pDX, IDC_EDIT_TURN_INTERVAL, m_delayBetweenTurnsSeconds);
	DDX_Text(pDX, IDC_BROWSE_TABLE_INI, m_tableInitialisationFile);
	DDX_Radio(pDX, IDC_RADIO_SHAD_O_CAM, m_cameraType);
	DDX_Radio(pDX, IDC_RADIO_SAVE_EACH_FRAME, m_frameSavingOptions);
	DDX_Text(pDX, IDC_EDIT_RESEARCHER_NAME, m_researcherName);
	DDX_Text(pDX, IDC_EDIT_SAMPLE_NAME, m_sampleName);
	DDX_Text(pDX, IDC_EDIT_TIMESTAMP, m_timestamp);
	DDX_Text(pDX, IDC_EDIT_NUM_FRAMES, m_numImages);
	DDX_Control(pDX, IDC_EDIT_TABLE_COMMANDS, m_tableOutputControl);
	DDX_Text(pDX, IDC_EDIT_PE_MODE, m_perkinElmerMode);
	DDX_Control(pDX, IDC_CHECK_TABLE_READY, m_tableReady);
}

BEGIN_MESSAGE_MAP(CTomographyControlDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_INITIALISE_TABLE, &CTomographyControlDlg::OnBnClickedButtonInitialiseTable)
	ON_BN_CLICKED(IDC_BUTTON_TABLE_NRESET, &CTomographyControlDlg::OnBnClickedButtonTableNreset)
	ON_BN_CLICKED(IDC_BUTTON_TABLE_NCAL, &CTomographyControlDlg::OnBnClickedButtonTableNcal)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_TABLE_DISPLAY, &CTomographyControlDlg::OnBnClickedButtonClearTableDisplay)
	ON_BN_CLICKED(IDC_BUTTON_RUN_LOOP, &CTomographyControlDlg::OnBnClickedButtonRunLoop)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA_TAKE_SINGLE, &CTomographyControlDlg::OnBnClickedButtonCameraTakeSingle)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA_TAKE_DARK, &CTomographyControlDlg::OnBnClickedButtonCameraTakeDark)
	ON_BN_CLICKED(IDC_BUTTON_CAMERA_TAKE_FLAT, &CTomographyControlDlg::OnBnClickedButtonCameraTakeFlat)
	ON_MESSAGE(WM_USER_TABLE_INPUT_RECEIVED, &CTomographyControlDlg::OnTableMessageReceived)
	ON_MESSAGE(WM_USER_TABLE_OUTPUT_FINISHED, &CTomographyControlDlg::OnTableOutputFinished)
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
        pMsg->wParam == VK_RETURN) /* &&
        this -> GetFocus() == &this -> m_tableCommandControl) */
    {
		this -> UpdateData(TRUE);

		// Take a copy of the command, then wipe the field
		CString command = m_tableCommand + "\r\n";

		this -> m_tableCommand.Empty();

		this -> UpdateData(FALSE);
		
		// Check if the command ends with the "nm" command
		if (command.Find(" nm\r\n") > 0)
		{
			float angle;
			int axis;

			if (sscanf(command, "%f %d nm", &angle, &axis) > 0)
			{
				// Command was matched successfully
				if (angle > 360.0
					|| angle < -360.0) {
					MessageBox("Table cannot be rotated to more than 360 degrees.", "Tomography Control", MB_ICONERROR);
					return TRUE;
				}
			}
		}

		this -> m_table -> SendToTable(command);

        return TRUE; // this doesn't need processing anymore
    }
    return FALSE; // all other cases still need default processing
}

/**
 * Send the contents of the table initialisation file to the table.
 */
void CTomographyControlDlg::OnBnClickedButtonInitialiseTable()
{
	this -> UpdateData(TRUE);

	if (this -> m_tableInitialisationFile.GetLength() == 0)
	{
		MessageBox("You must specify a table initialisation file.", "Tomography Control", MB_ICONERROR);
		return;
	}

	HANDLE initialisationFileHandle = CreateFile(m_tableInitialisationFile,
		GENERIC_READ, FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (initialisationFileHandle == INVALID_HANDLE_VALUE)
	{
		DWORD dw = GetLastError();
		char buffer[ERROR_BUFFER_SIZE];

		FormatMessage(FORMAT_MESSAGE_FROM_HMODULE,
			initialisationFileHandle,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)buffer,
			ERROR_BUFFER_SIZE - 1, NULL );

		MessageBox(buffer, "Tomography Control", MB_ICONERROR);

		return;
	}

	DWORD fileSize = GetFileSize(initialisationFileHandle, NULL);
	char *fileBuffer;

	if (fileSize == INVALID_FILE_SIZE)
	{
		DWORD dw = GetLastError();
		char buffer[ERROR_BUFFER_SIZE];

		FormatMessage(FORMAT_MESSAGE_FROM_HMODULE,
			initialisationFileHandle,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)buffer,
			ERROR_BUFFER_SIZE - 1, NULL );

		MessageBox(buffer, "Tomography Control", MB_ICONERROR);

		return;
	}

	fileBuffer = (char *)malloc(sizeof(char) * (fileSize + 3)); // 3 spare characters for "\r\n\0"
	if (NULL == fileBuffer)
	{
		MessageBox("Could not allocate memory for table initialisation file.", "Tomography Control", MB_ICONERROR);
		return;
	}
	
	DWORD bytesRead;
	if (!ReadFile(initialisationFileHandle, fileBuffer,
		fileSize, &bytesRead,
		NULL
		))
	{
		DWORD dw = GetLastError();
		char errorBuffer[ERROR_BUFFER_SIZE];

		FormatMessage(FORMAT_MESSAGE_FROM_HMODULE,
			initialisationFileHandle,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)errorBuffer,
			ERROR_BUFFER_SIZE - 1, NULL );

		MessageBox(errorBuffer, "Tomography Control", MB_ICONERROR);

		return;
	}

	CloseHandle(initialisationFileHandle);

	char *endOfBuffer = (fileBuffer + fileSize);

	// Ensure the buffer ends with CRLF
	*(endOfBuffer++) = '\r';
	*(endOfBuffer++) = '\n';
	// Null terminate the buffer
	*(endOfBuffer) = NULL;
	this -> m_table -> SendToTable(fileBuffer);

	free(fileBuffer);
}


void CTomographyControlDlg::OnBnClickedButtonClearTableDisplay()
{
	this -> m_table -> ClearDisplay();
	
	UpdateData(TRUE);
	this -> m_tableCommandOutput.Empty();
	UpdateData(FALSE);
}


void CTomographyControlDlg::OnBnClickedButtonTableNreset()
{
	this -> m_table -> SendToTable("nreset\r\n");
}


void CTomographyControlDlg::OnBnClickedButtonTableNcal()
{
	this -> m_table -> SendToTable("ncal\r\n");
}

LRESULT CTomographyControlDlg::OnTableMessageReceived(WPARAM wParam, LPARAM tablePtr)
{
	Table* table = (Table*)tablePtr;

	UpdateData(TRUE);
	table -> m_bufferLock.Lock();
	this -> m_tableCommandOutput += table -> m_displayBuffer;
	table -> m_displayBuffer.Empty();
	table -> m_bufferLock.Unlock();
	UpdateData(FALSE);

	// Scroll to the end of the buffer
	this -> m_tableOutputControl.LineScroll(m_tableOutputControl.GetLineCount());

	return TRUE;
}

LRESULT CTomographyControlDlg::OnTableOutputFinished(WPARAM wParam, LPARAM tablePtr)
{
	this -> m_tableReady.SetCheck(BST_CHECKED);
	UpdateData(FALSE);

	return TRUE;
}

/* Get a camera object, depending on the currently selected type.
 *
 * Throws bad_camera_type_error in case of a problem with the camera type.
 * Throws camera_init_error if there's an error with the camera.
 * Throws camera_settings_error if there's a problem with the values sent to the camera.
 */
Camera* CTomographyControlDlg::BuildSelectedCamera()
{
	Camera *camera;

	switch (this -> m_cameraType)
	{
	case 0:
		char pixelMapFilename[MAX_PATH];

		// Guess at location of pixel map file
		if (FAILED(SHGetFolderPath(NULL, 
			CSIDL_PERSONAL|CSIDL_FLAG_CREATE, 
			NULL, 
			0, 
			pixelMapFilename)))
		{
			throw bad_directory_error("Could not retrieve user home directory.");
		}
	
		PathAppend(pixelMapFilename, "Pixel Map");

		camera = new ShadOCam(this -> m_directoryPathBuffer, this -> m_exposureTimeSeconds, SHAD_O_CAM_CONFIG_FILE, pixelMapFilename);
		break;
	case 1:
		camera = new PerkinElmerXrd(this -> m_directoryPathBuffer, this -> m_perkinElmerMode);
		break;
	case 2:
		camera = new DummyCamera(this -> m_directoryPathBuffer, this -> m_exposureTimeSeconds);
		break;
	default:
		throw bad_camera_type_error("Unrecognised camera type.");
	}

	return camera;
}

FrameSavingOptions CTomographyControlDlg::GetFrameSavingOptions()
{
	switch (this -> m_frameSavingOptions)
	{
	case 0:
		return INDIVIDUAL;
	case 1:
		return AVERAGE;
	case 2:
		return SUM;
	default:
		throw bad_frame_saving_options_error("Unknown frame saving option type.");
	}
}

void CTomographyControlDlg::OnBnClickedButtonRunLoop()
{
	UpdateData(TRUE);
	
	try {
		// Copy directory path into a buffer in the object for later reference.
		UpdateDirectoryPath();
	}
	catch(bad_directory_error error)
	{
		MessageBox(error.what(), "Tomography Control", MB_ICONERROR);

		return;
	}
	
	if (this -> m_exposureTimeSeconds < 0.000)
	{
		MessageBox("Exposure time must be a positive number of seconds.", "Tomography Control", MB_ICONERROR);
		return;
	}
	if (this -> m_perkinElmerMode < 0
		|| this -> m_perkinElmerMode > 7)
	{
		MessageBox("Perkin-Elmer camera capture mode must be between 0 and 7.", "Tomography Control", MB_ICONERROR);
		return;
	}
	if (this -> m_framesPerStop < 1)
	{
		MessageBox("Frames per stop must be at least 1.", "Tomography Control", MB_ICONERROR);
		return;
	}
	if (this -> m_stopsPerRotation < 1)
	{
		MessageBox("Stops per rotation must be at least 1.", "Tomography Control", MB_ICONERROR);
		return;
	}
	if (this -> m_turnsTotal < 1)
	{
		MessageBox("Rotations to be performed must be at least 1.", "Tomography Control", MB_ICONERROR);
		return;
	}

	CRunProgressDlg runProgressDlg;
	
	runProgressDlg.m_table = this -> m_table;
	
	try {
		runProgressDlg.m_camera = BuildSelectedCamera();
	}
	catch(camera_init_error error)
	{
		MessageBox(error.what(), "Tomography Control", MB_ICONERROR);
		return;
	}
	
	runProgressDlg.m_frameSavingOptions = GetFrameSavingOptions();
	runProgressDlg.m_directoryPath = this -> m_directoryPathBuffer;
	runProgressDlg.m_exposureTimeSeconds = this -> m_exposureTimeSeconds;
	runProgressDlg.m_framesPerStop = this -> m_framesPerStop; // What about m_numImages?
	runProgressDlg.m_stopsPerRotation = this -> m_stopsPerRotation;
	runProgressDlg.m_turnsTotal = this -> m_turnsTotal;

	// The dialog handles waiting for the thread to exit
	runProgressDlg.DoModal();

	delete runProgressDlg.m_camera;
}

/* The following methods deal with manually taking images from the camera, on
 * request.
 */
void CTomographyControlDlg::OnBnClickedButtonCameraTakeSingle()
{
	this -> UpdateData(TRUE);

	this -> RunManualImageTask(GetFrameSavingOptions(), SINGLE);
}

void CTomographyControlDlg::OnBnClickedButtonCameraTakeDark()
{
	this -> UpdateData(TRUE);

	this -> RunManualImageTask(GetFrameSavingOptions(), DARK);
}

void CTomographyControlDlg::OnBnClickedButtonCameraTakeFlat()
{
	this -> UpdateData(TRUE);

	this -> RunManualImageTask(GetFrameSavingOptions(), FLAT_FIELD);
}

void CTomographyControlDlg::RunManualImageTask(FrameSavingOptions frameSavingOptions, FrameType taskType)
{
	CTakingPhotosDlg takingPhotosDlg;
	
	try {
		UpdateDirectoryPath();
	}
	catch(bad_directory_error error)
	{
		MessageBox(error.what(), "Tomography Control", MB_ICONERROR);
		return;
	}

	try {
		takingPhotosDlg.m_camera = BuildSelectedCamera();
	}
	catch(bad_camera_type_error error)
	{
		MessageBox(error.what(), "Tomography Control", MB_ICONERROR);
		return;
	}

	takingPhotosDlg.m_totalImages = this -> m_numImages;
	takingPhotosDlg.m_frameSavingOptions = frameSavingOptions;
	takingPhotosDlg.m_taskType = taskType;
	takingPhotosDlg.m_directoryPath = this -> m_directoryPathBuffer;
	takingPhotosDlg.m_exposureTimeSeconds = this -> m_exposureTimeSeconds;
	takingPhotosDlg.DoModal();

	delete takingPhotosDlg.m_camera;
}

/*
 * Updates the path stored in this class, for where images should be written
 * out to. Also ensures the directory exists.
 *
 * Throws bad_directory_error in case of problems accessing/creating the
 * directory.
 */
void CTomographyControlDlg::UpdateDirectoryPath()
{
	int mkdir_res = 0;
	CString directoryName;

	if (FAILED(SHGetFolderPath(NULL, 
        CSIDL_PERSONAL|CSIDL_FLAG_CREATE, 
        NULL, 
        0, 
        this -> m_directoryPathBuffer)))
	{
		throw bad_directory_error("Could not retrieve user home directory.");
	}

	PathAppend(this -> m_directoryPathBuffer, "CT_Scans");

	// Ensure the outer directory exists
	if (GetFileAttributesA(this -> m_directoryPathBuffer) == INVALID_FILE_ATTRIBUTES)
	{
		if (FAILED(CreateDirectory(this -> m_directoryPathBuffer, NULL)))
		{
			throw bad_directory_error("Could not create outer directory.");
		}
	}

	
	if (!this -> m_researcherName.IsEmpty())
	{
		directoryName.Append(m_researcherName);
		directoryName.Append("_");
	}
	if (!this -> m_sampleName.IsEmpty())
	{
		directoryName.Append(m_sampleName);
		directoryName.Append("_");
	}
	directoryName.Append(this -> m_timestamp);
	PathAppend(this -> m_directoryPathBuffer, (LPCSTR)directoryName);

	if (GetFileAttributesA(this -> m_directoryPathBuffer) == INVALID_FILE_ATTRIBUTES)
	{
		if (FAILED(CreateDirectory(this -> m_directoryPathBuffer, NULL)))
		{
			throw bad_directory_error("Could not create directory to write images to.");
		}
	}
}