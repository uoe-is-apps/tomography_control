
// Tomography ControlDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "Table.h"

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

// CTomographyControlDlg dialog
class CTomographyControlDlg : public CDialogEx
{
// Construction
public:
	CTomographyControlDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TOMOGRAPHYCONTROL_DIALOG };

// Table controller
	ITable* table;

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
	void RunManualImageTask(CameraTask* task);
	DECLARE_MESSAGE_MAP()
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
	CString m_mainImageName;
	float m_exposureTimeSeconds;
	int m_framesPerStop;
	int m_stopsPerRotation;
	int m_numberOfTurns;
	int m_delayBetweenTurnsSeconds;
	CString m_cameraName;
	CComboBox m_cameraComboBox;
};


// Function for the manual camera worker thread
UINT takeManualImages( LPVOID pParam );