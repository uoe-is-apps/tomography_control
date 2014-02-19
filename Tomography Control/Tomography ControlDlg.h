
// Tomography ControlDlg.h : header file
//

#pragma once
#include "afxwin.h"


class CTableAndCameraControl
{
public:
	CTableAndCameraControl();
	void SendTableCommand(CDialogEx* dialog, char* command);
};

// CTomographyControlDlg dialog
class CTomographyControlDlg : public CDialogEx
{
// Construction
public:
	CTomographyControlDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TOMOGRAPHYCONTROL_DIALOG };

// Table and camera controller
	CTableAndCameraControl* tableAndCameraControl;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual BOOL CTomographyControlDlg::PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonInitialiseTable();
	afx_msg void OnBnClickedButtonTableNreset();
	afx_msg void OnBnClickedButtonTableNcal();
	afx_msg void OnBnClickedButtonClearTableDisplay();
	afx_msg void OnBnClickedButtonResetTable();
	afx_msg void OnBnClickedButtonRunLoop();
	afx_msg void OnBnClickedButtonStopRunLoop();
	afx_msg void OnBnClickedButtonCameraWriteInitial();
	afx_msg void OnBnClickedButtonCameraTakeSingle();
	afx_msg void OnBnClickedButtonCameraTakeDark();
	afx_msg void OnBnClickedButtonCameraTakeFlat();
	
	// Table controls
	CString m_TableCommand;
	CString m_TableCommandOutput;
	CEdit m_TableCommandControl;
	CString m_TableInitialisationFile;

	// Run controls
	CButton m_StopRunLoopButton;
	CButton m_RunLoopButton;

	// Camera controls
	CString m_MainImageName;
	int m_ExposureTime;
	int m_FramesPerStop;
	int m_StopsPerRotation;
	int m_NumberOfTurns;
	int m_DelayBetweenTurnsSeconds;
	CString m_ManualCameraControl;
};

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

// Function for the manual camera worker thread
UINT TakeImages( LPVOID pParam );