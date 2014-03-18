// TakingPhotosDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Exceptions.h"
#include "TakingPhotosDlg.h"
#include "Tomography Control.h"
#include "Tomography ControlDlg.h"
#include "afxdialogex.h"


// CTakingPhotosDlg dialog - this is the modal dialog shown while a set
// of photos is being taken. This dialog doesn't do anything itself, but
// instead acts as a display while a separate thread feeds in progress

IMPLEMENT_DYNAMIC(CTakingPhotosDlg, CDialogEx)

	CTakingPhotosDlg::CTakingPhotosDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTakingPhotosDlg::IDD, pParent)
{
}

CTakingPhotosDlg::~CTakingPhotosDlg()
{
}

void CTakingPhotosDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_ACQUISITION, m_progress);
}

BOOL CTakingPhotosDlg::OnInitDialog()
{
	BOOL retVal = CDialogEx::OnInitDialog();

	this -> m_task = new ManualCameraTask(this -> m_taskType, this -> m_frameSavingOptions, this -> m_totalImages);
	
	// this -> m_task -> m_directoryPath = this -> m_directoryPath;
	this -> m_task -> m_dialog = this;
	this -> m_task -> m_camera = this -> m_camera;
	this -> m_task -> m_exposureTimeSeconds = this -> m_exposureTimeSeconds;
	this -> m_task -> m_running = TRUE;

	this -> m_progress.SetRange(0, this -> m_task -> m_totalImages - 1);

	this -> m_workerThread = AfxBeginThread(takeManualImages, this -> m_task, THREAD_PRIORITY_NORMAL, 
		0, CREATE_SUSPENDED);
	this -> m_workerThread -> m_bAutoDelete = FALSE;
	this -> m_workerThread -> ResumeThread();

	return retVal;
}

BEGIN_MESSAGE_MAP(CTakingPhotosDlg, CDialogEx)
	ON_MESSAGE(WM_USER_FRAME_CAPTURED, &CTakingPhotosDlg::OnFrameCaptured)
	ON_MESSAGE(WM_USER_THREAD_FINISHED, &CTakingPhotosDlg::OnThreadFinished)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &CTakingPhotosDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CTakingPhotosDlg message handlers

afx_msg void CTakingPhotosDlg::OnClose()
{
	// Tell the background worker we're finished running, but wait for it to exit
	// before closing the window
	this -> m_task -> m_running = FALSE;
	return;
}

void CTakingPhotosDlg::OnBnClickedCancel()
{
	// Tell the background worker we're finished running, but wait for it to exit
	// before closing the window
	this -> m_task -> m_running = FALSE;
	return;
}

afx_msg LRESULT CTakingPhotosDlg::OnFrameCaptured(WPARAM wParam, LPARAM lParam)
{
	u_int position = (u_int)lParam;

	this -> m_progress.SetPos(position + 1);

	return TRUE;
}

afx_msg LRESULT CTakingPhotosDlg::OnThreadFinished(WPARAM wParam, LPARAM lParam)
{
	// Give the thread a second to exit
	::WaitForSingleObject(this -> m_workerThread -> m_hThread, 1000);

	delete this -> m_workerThread;

	EndDialog(IDOK);

	return TRUE;
}

// Helper class for tracking details of the task

ManualCameraTask::ManualCameraTask(FrameType taskType, FrameSavingOptions captureType, unsigned short totalImages)
{
	this -> m_frameSavingOptions = captureType;
	this -> m_taskType = taskType;
	this -> m_totalImages = totalImages;
}

// Worker function for taking a set of manual images from an X-ray camera
UINT takeManualImages( LPVOID pParam )
{
	ManualCameraTask* task = (ManualCameraTask*)pParam;
	CTakingPhotosDlg* dialog = (CTakingPhotosDlg*)task -> m_dialog;
	
	try {
		task -> m_camera -> SetupCamera(task -> m_exposureTimeSeconds);
	}
	catch(camera_init_error error)
	{
		MessageBox(*task -> m_dialog, error.what(), "Tomography Control", MB_ICONERROR);
		dialog -> PostMessage(WM_USER_THREAD_FINISHED);
		return 0;
	}

	u_int frameCount = 1;
	
	try {
		task -> m_camera -> CaptureFrames(task -> m_totalImages, &frameCount, task -> m_frameSavingOptions,
			task -> m_taskType, dialog);
	}
	catch (camera_acquisition_error error)
	{
		MessageBox(*task -> m_dialog, error.what(), "Tomography Control", MB_ICONERROR);
		dialog -> PostMessage(WM_USER_THREAD_FINISHED);
		return 0;
	}

	dialog -> PostMessage(WM_USER_THREAD_FINISHED);

	return 0;   // thread completed successfully
}