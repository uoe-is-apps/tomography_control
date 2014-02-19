// TakingPhotosDlg.cpp : implementation file
//

#include "stdafx.h"
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
	this -> m_IsInit = FALSE;
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
	this -> m_IsInit = TRUE;

	return retVal;
}

BEGIN_MESSAGE_MAP(CTakingPhotosDlg, CDialogEx)
END_MESSAGE_MAP()


// CTakingPhotosDlg message handlers

// Helper class for tracking details of the task

CameraTask::CameraTask(TaskType taskType)
{
	this -> m_taskType = taskType;

	switch (taskType)
	{
	case SINGLE:
		this -> m_totalImages = 1;
		break;
	default:
		this -> m_totalImages = 10;
		break;
	}
}