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
	// virtual void OnCancel( );
	virtual void OnClose( );

// Dialog Data
	enum { IDD = IDD_TAKING_PHOTOS_DIALOG };

	enum TaskType { SINGLE, DARK, FLAT_FIELD };

	TaskType m_taskType;
	CProgressCtrl m_progress;
	short m_currentImages;
	short m_totalImages;
	BOOL m_running;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CWinThread *m_workerThread;

	DECLARE_MESSAGE_MAP()
};
