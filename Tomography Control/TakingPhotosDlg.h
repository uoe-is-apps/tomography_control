#pragma once
#include "afxcmn.h"

// Function for the worker thread
UINT TakeImages( LPVOID pParam );

// CTakingPhotosDlg dialog

class CTakingPhotosDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTakingPhotosDlg)

public:
	CTakingPhotosDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTakingPhotosDlg();
	virtual BOOL OnInitDialog();
	BOOL m_IsInit;

// Dialog Data
	enum { IDD = IDD_TAKING_PHOTOS_DIALOG };

	CProgressCtrl m_progress;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// CWinThread *m_workerThread;

	DECLARE_MESSAGE_MAP()
};

/* Tracks details of a request sent to the camera */
class CameraTask
{
public:
	enum TaskType { SINGLE, DARK, FLAT_FIELD };

	CameraTask(TaskType taskType);

	TaskType m_taskType;
	CTakingPhotosDlg* m_dialog;
	short m_currentImages;
	short m_totalImages;
	BOOL m_running;
};