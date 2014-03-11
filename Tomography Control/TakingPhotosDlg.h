#pragma once
#include "afxwin.h"
#include "Camera.h"
#include "Table.h"

/* Tracks details of a request sent to the camera */
class ManualCameraTask
{
public:

	ManualCameraTask(FrameType taskType, FrameSavingOptions captureType, unsigned short totalImages);
	
	FrameSavingOptions m_frameSavingOptions;
	FrameType m_taskType;
	Camera* m_camera;

	char *m_directoryPath;

	CWnd* m_dialog;

	float m_exposureTimeSeconds;
	unsigned short m_currentImages;
	unsigned short m_totalImages;
	BOOL m_running;
};
// CTakingPhotosDlg dialog

class CTakingPhotosDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTakingPhotosDlg)

public:
	CTakingPhotosDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTakingPhotosDlg();
	virtual BOOL OnInitDialog();
	
	FrameType m_taskType;
	FrameSavingOptions m_frameSavingOptions;

	ManualCameraTask *m_task;
	CWinThread* m_workerThread;
	unsigned short m_totalImages;
	float m_exposureTimeSeconds;
	char *m_directoryPath;
	Camera* m_camera;

// Dialog Data
	enum { IDD = IDD_TAKING_PHOTOS_DIALOG };

	afx_msg void OnClose();
	void OnBnClickedCancel();
	afx_msg LRESULT OnFrameCaptured(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnThreadFinished(WPARAM wParam, LPARAM lParam);

protected:
	CProgressCtrl m_progress;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// CWinThread *m_workerThread;

	DECLARE_MESSAGE_MAP()
};