#pragma once
#include "afxcmn.h"

// Function for the worker thread
UINT TakeMultipleImages( LPVOID pParam );
// Function for the worker thread
UINT TakeSingleImage( LPVOID pParam );

// CTakingPhotosDlg dialog

class CTakingPhotosDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTakingPhotosDlg)

public:
	CTakingPhotosDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTakingPhotosDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_TAKING_PHOTOS_DIALOG };

	enum TaskType { SINGLE, DARK, FLAT_FIELD };

	TaskType m_taskType;
	CProgressCtrl m_progress;
	BOOL m_running;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
