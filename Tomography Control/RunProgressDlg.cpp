// RunProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Tomography Control.h"
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
	, m_stopsMade(_T(0))
	, m_stopsPerRotation(0)
	, m_turnsMade(0)
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
	DDX_Text(pDX, IDC_DISPLAY_STOPS_MADE, m_stopsMade);
	DDX_Text(pDX, IDC_DISPLAY_STOPS_TOTAL, m_stopsPerRotation);
	DDX_Text(pDX, IDC_DISPLAY_TURNS_MADE, m_turnsMade);
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
	
	__time64_t exposureTimePerTurn = this -> m_exposureTimeSeconds * this -> m_stopsPerRotation * this -> m_framesPerStop;
	// TODO: Calculate realistic time for turning the table
	__time64_t rotationTimePerTurn = this -> m_stopsPerRotation;
	__time64_t timePerTurn = exposureTimePerTurn + rotationTimePerTurn;

	
	CTimeSpan estRunTime(timePerTurn * this -> m_turnsTotal);
	CTime estEndTime = startTime + estRunTime;
	
	this -> m_startTime.Append(startTime.Format("%H:%M:%S"));
	this -> m_estEndTime.Append(estEndTime.Format("%H:%M:%S"));
	this -> m_estRunTime.Append(estRunTime.Format("%H:%M:%S"));
	this -> m_progressCtl.SetRange(0, (short)MAX_PROGRESS);

	return TRUE;  // return TRUE  unless you set the focus to a control
}


BEGIN_MESSAGE_MAP(CRunProgressDlg, CDialogEx)
	ON_MESSAGE(WM_USER_RUN_TURN_COMPLETED, &CRunProgressDlg::OnTurnCompleted)
	ON_MESSAGE(WM_USER_RUN_TABLE_ANGLE_CHANGED, &CRunProgressDlg::OnTableAngleChanged)
	ON_MESSAGE(WM_USER_RUN_STOP_COMPLETED, &CRunProgressDlg::OnStopCompleted)
	ON_MESSAGE(WM_USER_RUN_IMAGE_CAPTURED, &CRunProgressDlg::OnImageCaptured)
END_MESSAGE_MAP()


// CRunProgressDlg message handlers


afx_msg LRESULT CRunProgressDlg::OnTurnCompleted(WPARAM wParam, LPARAM lParam)
{
	int* turnCount = (int*)lParam;

	this -> m_turnsMade = *turnCount;
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

	this -> m_stopsMade = *stopCount;
	this -> UpdateData(FALSE);

	int totalStops = m_turnsTotal * m_stopsPerRotation;
	short progress = MAX_PROGRESS * (*stopCount) / totalStops;

	this -> m_progressCtl.SetPos(progress);

	return TRUE;
}

afx_msg LRESULT CRunProgressDlg::OnImageCaptured(WPARAM wParam, LPARAM lParam)
{
	int* currentPosition = (int*)lParam;

	this -> m_currentPosition = *currentPosition;
	this -> UpdateData(FALSE);

	return TRUE;
}


// Worker functions
UINT takeRunImages( LPVOID pParam )
{
	RunTask* task = (RunTask*)pParam;
	CRunProgressDlg* dialog = (CRunProgressDlg*)task -> m_dialog;

	char tableCommandBuffer[TABLE_COMMAND_BUFFER_SIZE];
	const float tableResolution = 0.0005f; // Degrees
	int stepsPerStop = (int)((360.0f / task -> m_stopsPerTurn) / tableResolution);

	task -> m_currentPosition = 0;
	
	for (task -> m_turnCount = 0; task -> m_turnCount < task -> m_turnsTotal; task -> m_turnCount++)
	{
		for (task -> m_stopCount = 0; task -> m_stopCount < task -> m_stopsPerTurn; task -> m_stopCount++)
		{
			float calculatedAngle = task -> m_stopCount * stepsPerStop * tableResolution;
			sprintf_s(tableCommandBuffer, TABLE_COMMAND_BUFFER_SIZE, "deg %.2f nm\r\n", calculatedAngle);

			task -> m_table -> SendTableCommand(tableCommandBuffer);
			// TODO: See if we can get a verification of state from the table instead of just waiting blindly
			Sleep(1000);

			if (::IsWindow(dialog -> m_hWnd))
			{
				dialog -> PostMessage(WM_USER_RUN_TABLE_ANGLE_CHANGED, 0, (LPARAM)&calculatedAngle);
			}

			for (task -> m_frameCount = 0; task -> m_frameCount < task -> m_framesPerStop; task -> m_frameCount++)
			{
				task -> m_currentPosition++;

				if (task -> m_running)
				{
					// Should input filename here, too
					task -> m_camera -> TakeImage("");
					if (::IsWindow(dialog -> m_hWnd))
					{
						dialog -> PostMessage(WM_USER_RUN_IMAGE_CAPTURED, 0, (LPARAM)&task -> m_currentPosition);
					}
				}
				else
				{
					break;
				}
			}

			if (task -> m_running)
			{
				if (::IsWindow(dialog -> m_hWnd))
				{
					dialog -> PostMessage(WM_USER_RUN_STOP_COMPLETED, 0, (LPARAM)&task -> m_stopCount);
				}
			}
			else
			{
				break;
			}

		}

		if (task -> m_running)
		{
			if (::IsWindow(dialog -> m_hWnd))
			{
				dialog -> PostMessage(WM_USER_RUN_TURN_COMPLETED, 0, (LPARAM)&task -> m_turnCount);
			}
		}
		else
		{
			break;
		}
	}

	if (task -> m_running == TRUE
		&& ::IsWindow(dialog -> m_hWnd))
	{
		dialog -> PostMessage(WM_CLOSE);
	}

	return 0;   // thread completed successfully
}
