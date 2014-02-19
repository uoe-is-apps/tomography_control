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
{

}

CRunProgressDlg::~CRunProgressDlg()
{
}

void CRunProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CRunProgressDlg, CDialogEx)
END_MESSAGE_MAP()


// CRunProgressDlg message handlers



// Worker functions
UINT takeRunImages( LPVOID pParam )
{
	CRunProgressDlg* dialog = (CRunProgressDlg*)pParam;

	// Display start time
	CString startTime = CTime::GetCurrentTime().Format("%H:%M:%S");
	/* dialog -> m_startTimeDisplay.Append(startTime);

	float degreesPerStop = 360.0f / dialog -> m_stopsPerRotation;
	
	
	for (dialog -> m_turnsMadeDisplay = 0; dialog -> m_turnsMadeDisplay < dialog -> m_numberOfTurns; dialog -> m_turnsMadeDisplay++)
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
	
	Sleep(5000); // Leave the window open for a bit

	return 0;   // thread completed successfully
}
