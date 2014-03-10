
// Tomography ControlDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "Camera.h"
#include "Table.h"

/* Tracks details of a request sent to the camera */
class CameraTask
{
public:

	CameraTask(FrameType taskType);

	FrameType m_taskType;
	ICamera* m_camera;
	CString m_directoryPath;
	CWnd* m_dialog;
	short m_currentImages;
	short m_totalImages;
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
	CString m_directoryPath;

	CameraTask *m_task;
	CWinThread* m_workerThread;
	ICamera* m_camera;

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

// CTomographyControlDlg dialog
class CTomographyControlDlg : public CDialogEx
{
// Construction
public:
	CTomographyControlDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TOMOGRAPHYCONTROL_DIALOG };

// Table controller
	Table* m_table;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
public:
	afx_msg void OnBnClickedButtonInitialiseTable();
	afx_msg void OnBnClickedButtonTableNreset();
	afx_msg void OnBnClickedButtonTableNcal();
	afx_msg void OnBnClickedButtonClearTableDisplay();
	afx_msg void OnBnClickedButtonResetTable();
	afx_msg void OnBnClickedButtonRunLoop();
	afx_msg void OnBnClickedButtonCameraWriteInitial();
	afx_msg void OnBnClickedButtonCameraTakeSingle();
	afx_msg void OnBnClickedButtonCameraTakeDark();
	afx_msg void OnBnClickedButtonCameraTakeFlat();

	// Called when the worker thread completes image captures within a rotation of the table
	afx_msg LRESULT OnTableMessageReceived(WPARAM wParam, LPARAM tablePtr);

	// Camera controls
	CString m_manualCameraControl;
	
	// Table controls
	CString m_tableCommand;
	CString m_tableCommandOutput;
	CEdit m_tableCommandControl;
	CString m_tableInitialisationFile;

	// Run controls
	CButton m_runLoopButton;
	BOOL m_running; // Not set from the UI, just used by the background thread

	// Run inputs
	CString m_directoryPath;
	float m_exposureTimeSeconds;
	int m_framesPerStop;
	int m_stopsPerRotation;
	int m_turnsTotal;
	int m_delayBetweenTurnsSeconds;
	int m_cameraType;
	int m_tableType;
	int m_tableComPort;
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	void RunManualImageTask(FrameType taskType);
	ICamera* BuildSelectedCamera();
	DECLARE_MESSAGE_MAP()
};


// Function for the manual camera worker thread
UINT takeManualImages( LPVOID pParam );