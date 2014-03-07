// RunProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Tomography Control.h"
#include "Camera.h"
#include "RunProgressDlg.h"
#include "afxdialogex.h"

#define MAX_PROGRESS 1000
#define TABLE_COMMAND_BUFFER_SIZE 128

// CRunProgressDlg dialog

IMPLEMENT_DYNAMIC(CRunProgressDlg, CDialogEx)

CRunProgressDlg::CRunProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRunProgressDlg::IDD, pParent)
	, m_currentPosition(0)
	, m_imageFilename(_T(""))
	, m_framesPerStop(0)
	, m_framesCaptured(0)
	, m_stopsCompleted(_T(0))
	, m_stopsPerRotation(0)
	, m_turnsCompleted(0)
	, m_turnsTotal(0)
	, m_calculatedAngle(_T(""))
	, m_startTime(_T(""))
	, m_estEndTime(_T(""))
	, m_estRunTime(_T(""))
{

}

CRunProgressDlg::~CRunProgressDlg()
{
}

void CRunProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_DISPLAY_CURRENT_POSITION, m_currentPosition);
	DDX_Text(pDX, IDC_DISPLAY_PROCEEDED_IMAGE, m_imageFilename);
	DDX_Text(pDX, IDC_DISPLAY_STOPS_MADE, m_stopsCompleted);
	DDX_Text(pDX, IDC_DISPLAY_STOPS_TOTAL, m_stopsPerRotation);
	DDX_Text(pDX, IDC_DISPLAY_TURNS_MADE, m_turnsCompleted);
	DDX_Text(pDX, IDC_DISPLAY_TURNS_TOTAL, m_turnsTotal);
	DDX_Text(pDX, IDC_DISPLAY_CALC_ANGLE, m_calculatedAngle);
	DDX_Text(pDX, IDC_DISPLAY_START_TIME, m_startTime);
	DDX_Text(pDX, IDC_DISPLAY_EST_END_TIME, m_estEndTime);
	DDX_Text(pDX, IDC_DISPLAY_EST_RUN_TIME, m_estRunTime);
	DDX_Control(pDX, IDC_PROGRESS1, m_progressCtl);
}

BOOL CRunProgressDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	CTime startTime = CTime::GetCurrentTime();
	CTimeSpan estRunTime;

	this -> CalculateTimeRemaining(&estRunTime);

	CTime estEndTime = startTime + estRunTime;
	
	this -> m_startTime.Append(startTime.Format("%H:%M:%S"));
	this -> m_estEndTime.Append(estEndTime.Format("%H:%M:%S"));
	this -> m_estRunTime.Append(estRunTime.Format("%H:%M:%S"));
	this -> m_progressCtl.SetRange(0, (short)MAX_PROGRESS);

	this -> m_task = new RunTask();

	this -> m_task -> m_dialog = this;
	this -> m_task -> m_camera = this -> m_camera;
	this -> m_task -> m_table = this -> m_table;
	this -> m_task -> m_directoryPath = this -> m_directoryPath;
	this -> m_task -> m_turnsTotal = this -> m_turnsTotal;
	this -> m_task -> m_stopsPerTurn = this -> m_stopsPerRotation;
	this -> m_task -> m_framesPerStop = this -> m_framesPerStop;
	this -> m_task -> m_exposureTimeSeconds = this -> m_exposureTimeSeconds;
	this -> m_task -> m_running = TRUE;

	char settingsFilename[FILENAME_BUFFER_SIZE];
	sprintf_s(settingsFilename, FILENAME_BUFFER_SIZE - 1, "%s\\settings.txt", this -> m_directoryPath);
	WriteSettings(settingsFilename);

	this -> m_workerThread = AfxBeginThread(captureRunFrames, this -> m_task, THREAD_PRIORITY_NORMAL, 
		0, CREATE_SUSPENDED);
	this -> m_workerThread -> m_bAutoDelete = FALSE;
	this -> m_workerThread -> ResumeThread();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRunProgressDlg::CalculateTimeRemaining(CTimeSpan* dest) {
	// TODO: Check this maths
	__time64_t exposureTimePerStop = (int)(this -> m_exposureTimeSeconds * this -> m_framesPerStop);
	__time64_t timePerStop = exposureTimePerStop + 1; // Add in rotation time
	unsigned int totalStops = this -> m_stopsPerRotation * this -> m_turnsTotal;
	unsigned int stopsCompletedTotal = (this -> m_stopsPerRotation * this -> m_turnsCompleted) + this -> m_stopsCompleted;

	*dest = exposureTimePerStop * (totalStops - stopsCompletedTotal);
}


BEGIN_MESSAGE_MAP(CRunProgressDlg, CDialogEx)
	ON_MESSAGE(WM_USER_RUN_TURN_COMPLETED, &CRunProgressDlg::OnTurnCompleted)
	ON_MESSAGE(WM_USER_RUN_TABLE_ANGLE_CHANGED, &CRunProgressDlg::OnTableAngleChanged)
	ON_MESSAGE(WM_USER_RUN_STOP_COMPLETED, &CRunProgressDlg::OnStopCompleted)
	ON_MESSAGE(WM_USER_CAPTURING_FRAME, &CRunProgressDlg::OnFrameCaptureStarted)
	ON_MESSAGE(WM_USER_FRAME_CAPTURED, &CRunProgressDlg::OnFrameCaptured)
	ON_MESSAGE(WM_USER_THREAD_FINISHED, &CRunProgressDlg::OnThreadFinished)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &CRunProgressDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CRunProgressDlg message handlers

afx_msg void CRunProgressDlg::OnClose()
{
	// Tell the background worker we're finished running, but wait for it to exit
	// before closing the window
	this -> m_task -> m_running = FALSE;
	return;
}

void CRunProgressDlg::OnBnClickedCancel()
{
	// Tell the background worker we're finished running, but wait for it to exit
	// before closing the window
	this -> m_task -> m_running = FALSE;
	return;
}

afx_msg LRESULT CRunProgressDlg::OnFrameCaptured(WPARAM wParam, LPARAM lParam)
{
	this -> m_currentPosition = (int)lParam;
	this -> m_framesCaptured++;
	this -> UpdateData(FALSE);

	return TRUE;
}

afx_msg LRESULT CRunProgressDlg::OnFrameCaptureStarted(WPARAM wParam, LPARAM lParam)
{
	char* filename = (char*)lParam;

	this -> m_imageFilename.Empty();
	this -> m_imageFilename.Append(filename);
	this -> UpdateData(FALSE);

	return TRUE;
}

afx_msg LRESULT CRunProgressDlg::OnThreadFinished(WPARAM wParam, LPARAM lParam)
{
	// Give the thread 10 seconds to exit
	::WaitForSingleObject(this -> m_workerThread -> m_hThread, 10000);

	delete this -> m_workerThread;

	EndDialog(IDOK);

	return TRUE;
}

afx_msg LRESULT CRunProgressDlg::OnTurnCompleted(WPARAM wParam, LPARAM lParam)
{
	int* turnCount = (int*)lParam;

	this -> m_turnsCompleted = *turnCount;
	this -> UpdateData(FALSE);

	return TRUE;
}

afx_msg LRESULT CRunProgressDlg::OnTableAngleChanged(WPARAM wParam, LPARAM lParam)
{
	float* angle = (float*)lParam;

	this -> m_calculatedAngle.Format("%.2f", *angle);
	this -> UpdateData(FALSE);

	return TRUE;
}

afx_msg LRESULT CRunProgressDlg::OnStopCompleted(WPARAM wParam, LPARAM lParam)
{
	int* stopCount = (int*)lParam;

	this -> m_stopsCompleted = *stopCount;
	this -> UpdateData(FALSE);

	int totalStops = m_turnsTotal * m_stopsPerRotation;
	short progress = MAX_PROGRESS * (*stopCount) / totalStops;

	this -> m_progressCtl.SetPos(progress);

	return TRUE;
}

void CRunProgressDlg::WriteSettings(char* dest)
{
	FILE* fileHandle;
	errno_t fileErrno = fopen_s(&fileHandle, dest, "w");

	if (0 != fileErrno)
	{
		//TODO: Handle problems opening the file
		return;
	}

	fclose(fileHandle);
}

// Worker functions
UINT captureRunFrames( LPVOID pParam )
{
	RunTask* task = (RunTask*)pParam;
	CRunProgressDlg* dialog = (CRunProgressDlg*)task -> m_dialog;

	// Wait for the dialog to open before we proceed
	while (!::IsWindow(dialog -> m_hWnd))
	{
		Sleep(200);
	}
	
	char tableCommandBuffer[TABLE_COMMAND_BUFFER_SIZE];
	const float tableResolution = 0.0005f; // Degrees
	int stepsPerStop = (int)((360.0f / task -> m_stopsPerTurn) / tableResolution);

	task -> m_currentPosition = 0;
	
	for (task -> m_turnCount = 0; task -> m_turnCount < task -> m_turnsTotal && task -> m_running; task -> m_turnCount++)
	{
		for (task -> m_stopCount = 0; task -> m_stopCount < task -> m_stopsPerTurn && task -> m_running; task -> m_stopCount++)
		{
			float calculatedAngle = task -> m_stopCount * stepsPerStop * tableResolution;
			sprintf_s(tableCommandBuffer, TABLE_COMMAND_BUFFER_SIZE, "deg %.2f nm\r\n", calculatedAngle);

			task -> m_table -> SendTableCommand(tableCommandBuffer);
			// TODO: See if we can get a verification of state from the table instead of just waiting blindly
			Sleep(1000);

			dialog -> PostMessage(WM_USER_RUN_TABLE_ANGLE_CHANGED, 0, (LPARAM)&calculatedAngle);

			task -> m_camera -> CaptureFrames(task -> m_framesPerStop, SINGLE, dialog);
			
			dialog -> PostMessage(WM_USER_RUN_STOP_COMPLETED, 0, (LPARAM)&task -> m_stopCount);
		}
		
		dialog -> PostMessage(WM_USER_RUN_TURN_COMPLETED, 0, (LPARAM)&task -> m_turnCount);
	}

	dialog -> PostMessage(WM_USER_THREAD_FINISHED);

	return 0;   // thread completed successfully
}
