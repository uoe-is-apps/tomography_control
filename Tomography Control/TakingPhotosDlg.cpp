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
	case DARK:
	case FLAT_FIELD:
		this -> m_progress.SetRange(0, 9);
		AfxBeginThread(TakeMultipleImages, this);
		break;
	case SINGLE:
		this -> m_progress.SetRange(0, 1);
		AfxBeginThread(TakeSingleImage, this);
		break;
	}

	return retVal;
}

BEGIN_MESSAGE_MAP(CTakingPhotosDlg, CDialogEx)
END_MESSAGE_MAP()


// CTakingPhotosDlg message handlers

// Worker functions
UINT TakeMultipleImages( LPVOID pParam )
{
    CTakingPhotosDlg* dialog = (CTakingPhotosDlg*)pParam;

    if (dialog == NULL ||
        !dialog->IsKindOf(RUNTIME_CLASS(CTakingPhotosDlg)))
    return 1;   // if dialog is not valid

	for (short i = 0; i < 10; i++) 
	{
		Sleep(1000);
		if(!(dialog -> m_running))
		{
			break;
		}
		dialog->m_progress.SetPos(i + 1);
	}

    return 0;   // thread completed successfully
}

UINT TakeSingleImage( LPVOID pParam )
{
    CTakingPhotosDlg* dialog = (CTakingPhotosDlg*)pParam;

    if (dialog == NULL ||
        !dialog->IsKindOf(RUNTIME_CLASS(CTakingPhotosDlg)))
    return 1;   // if dialog is not valid

	for (short i = 0; i < 1; i++) 
	{
		Sleep(1000);
		if(!(dialog -> m_running))
		{
			break;
		}
		dialog->m_progress.SetPos(i + 1);
	}

    return 0;   // thread completed successfully
}