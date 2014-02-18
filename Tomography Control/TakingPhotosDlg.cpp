// TakingPhotosDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Tomography Control.h"
#include "TakingPhotosDlg.h"
#include "afxdialogex.h"


// CTakingPhotosDlg dialog

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

	this -> m_running = TRUE;

	switch (this -> m_taskType)
	{
	case SINGLE:
		this -> m_totalImages = 1;
		break;
	default:
		this -> m_totalImages = 10;
		break;
	}

	this -> m_progress.SetRange(0, m_totalImages - 1);
	this -> m_workerThread = AfxBeginThread(TakeImages, this, THREAD_PRIORITY_NORMAL, 
		0, CREATE_SUSPENDED);
	this -> m_workerThread -> m_bAutoDelete = FALSE;

	this -> m_workerThread -> ResumeThread();

	return retVal;
}

/* void CTakingPhotosDlg::OnCancel( )
{
	this -> m_running = false;
	// TODO: The thread shouldn't be keeping its run/not running
	// state in this class, as it's lost when the dialog closes.
} */

void CTakingPhotosDlg::OnClose( )
{
	// Give the thread 5 seconds to exit
	// ::WaitForSingleObject(this -> m_workerThread -> m_hThread, 5000);
	delete this -> m_workerThread;
}

BEGIN_MESSAGE_MAP(CTakingPhotosDlg, CDialogEx)
END_MESSAGE_MAP()


// CTakingPhotosDlg message handlers

// Worker functions
UINT TakeImages( LPVOID pParam )
{
	CTakingPhotosDlg* dialog = (CTakingPhotosDlg*)pParam;

	if (dialog == NULL ||
		!dialog->IsKindOf(RUNTIME_CLASS(CTakingPhotosDlg)))
	{
		return 1;
	}

	for (short i = 0; i < dialog -> m_totalImages; i++) 
	{
		Sleep(1000);
		if(!(dialog -> m_running))
		{
			break;
		}
		dialog->m_progress.SetPos(i + 1);
	}

	if(dialog -> m_running)
	{
		dialog -> PostMessage(WM_CLOSE);
	}

	return 0;   // thread completed successfully
}
