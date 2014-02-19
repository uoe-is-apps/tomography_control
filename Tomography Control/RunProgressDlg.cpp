// RunProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Tomography Control.h"
#include "RunProgressDlg.h"
#include "afxdialogex.h"


// CRunProgressDlg dialog

IMPLEMENT_DYNAMIC(CRunProgressDlg, CDialogEx)

CRunProgressDlg::CRunProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRunProgressDlg::IDD, pParent)
	, m_currentPosition(0)
	, m_imageFilename(_T(""))
	, m_stopsMade(_T(0))
	, m_stopsTotal(0)
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
	DDX_Text(pDX, IDC_DISPLAY_STOPS_TOTAL, m_stopsTotal);
	DDX_Text(pDX, IDC_DISPLAY_TURNS_MADE, m_turnsMade);
	DDX_Text(pDX, IDC_DISPLAY_TURNS_TOTAL, m_turnsTotal);
	DDX_Text(pDX, IDC_DISPLAY_CALC_ANGLE, m_calculatedAngle);
	DDX_Text(pDX, IDC_DISPLAY_START_TIME, m_startTime);
	DDX_Text(pDX, IDC_DISPLAY_EST_END_TIME, m_estEndTime);
	DDX_Text(pDX, IDC_DISPLAY_EST_RUN_TIME, m_estRunTime);
}

BOOL CRunProgressDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	CString startTime = CTime::GetCurrentTime().Format("%H:%M:%S");
	this -> m_startTime.Append(startTime);

	return TRUE;  // return TRUE  unless you set the focus to a control
}


BEGIN_MESSAGE_MAP(CRunProgressDlg, CDialogEx)
	ON_MESSAGE(WM_USER_TURN_COMPLETED, &CRunProgressDlg::OnTurnCompleted)
END_MESSAGE_MAP()


// CRunProgressDlg message handlers


afx_msg LRESULT CRunProgressDlg::OnTurnCompleted(WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

afx_msg LRESULT CRunProgressDlg::OnStopCompleted(WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

afx_msg LRESULT CRunProgressDlg::OnImageCaptured(WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}


// Worker functions
UINT takeRunImages( LPVOID pParam )
{
	RunTask* task = (RunTask*)pParam;

	const float tableResolution = 0.0005; // Degrees
	int stepsPerStop = (360.0f / task -> m_stopsPerTurn) / tableResolution;
	
	/* for (dialog -> m_turnsMadeDisplay = 0; dialog -> m_turnsMadeDisplay < dialog -> m_numberOfTurns; dialog -> m_turnsMadeDisplay++)
	{
		for (dialog -> m_stopsMadeDisplay = 0; dialog -> m_stopsMadeDisplay < dialog -> m_stopsPerRotation; dialog -> m_stopsMadeDisplay++)
		{
			// float m_calculatedAngle;
			// CString m_estimatedRunTimeDisplay;
			// CString m_estimatedEndTimeDisplay;

			for (int frameCount = 0; frameCount < dialog -> m_framesPerStop; frameCount++)
			{
				dialog -> PostMessage(WM_USER_DATA_CHANGED, 0, NULL);
				Sleep(10);

				if (!dialog -> m_running)
				{
					break;
				}
			}

			if (!dialog -> m_running)
			{
				break;
			}
		}

		if (!dialog -> m_running)
		{
			break;
		}
	} */

	if (task -> m_running)
	{
		Sleep(5000); // Leave the window open for a bit
		task -> m_dialog -> PostMessage(WM_CLOSE);
	}

	return 0;   // thread completed successfully
}
