
// Tomography ControlDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "Camera.h"
#include "Table.h"

#define DIRECTORY_PATH_BUFFER_SIZE 512
#define TIMESTAMP_BUFFER_SIZE 256


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
	int m_numImages;
	int m_numAvSumImages;
	int m_cameraType;
	
	// Table controls
	CString m_tableCommand;
	CString m_tableCommandOutput;
	CEdit m_tableCommandControl;
	CString m_tableInitialisationFile;

	// Run controls
	CButton m_runLoopButton;
	BOOL m_running; // Not set from the UI, just used by the background thread

	// Run inputs
	CString m_researcherName;
	CString m_sampleName;
	CString m_timestamp;
	int m_frameSavingOptions;
	float m_exposureTimeSeconds;
	int m_framesPerStop;
	int m_stopsPerRotation;
	int m_turnsTotal;
	int m_delayBetweenTurnsSeconds;
	int m_tableType;
	int m_tableComPort;
protected:
	HICON m_hIcon;
	char m_directoryPathBuffer[DIRECTORY_PATH_BUFFER_SIZE];

	FrameSavingOptions GetFrameSavingOptions();
	void UpdateDirectoryPath();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	void RunManualImageTask(FrameType taskType);
	Camera* BuildSelectedCamera();
	DECLARE_MESSAGE_MAP()
};


// Function for the manual camera worker thread
UINT takeManualImages( LPVOID pParam );