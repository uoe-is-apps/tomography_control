
// Tomography ControlDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "Camera.h"
#include "Table.h"

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
	afx_msg void OnBnClickedButtonRunLoop();
	afx_msg void OnBnClickedButtonCameraWriteInitial();
	afx_msg void OnBnClickedButtonCameraTakeSingle();
	afx_msg void OnBnClickedButtonCameraTakeDark();
	afx_msg void OnBnClickedButtonCameraTakeFlat();

	// Called when the worker thread completes image captures within a rotation of the table
	afx_msg LRESULT OnTableMessageReceived(WPARAM wParam, LPARAM tablePtr);

	// Called when commands sent to the table have finished being sent
	afx_msg LRESULT OnTableOutputFinished(WPARAM wParam, LPARAM tablePtr);

	// Camera controls
	int m_numImages;
	int m_cameraType;
	
	// Table controls
	CString m_tableCommand;
	CString m_tableCommandOutput;
	CEdit m_tableCommandControl;
	CString m_tableInitialisationFile;
	CEdit m_tableOutputControl;
	CButton m_tableReady;

	// Run controls
	BOOL m_running; // Not set from the UI, just used by the background thread

	// Run inputs
	CString m_researcherName;
	CString m_sampleName;
	CString m_timestamp;
	int m_frameSavingOptions;
	CString m_exposureTimeSeconds;
	int m_perkinElmerMode;
	int m_framesPerStop;
	int m_stopsPerRotation;
	int m_turnsTotal;
	int m_delayBetweenTurnsSeconds;
	int m_tableType;
protected:
	HICON m_hIcon;
	char m_directoryPathBuffer[MAX_PATH];

	FrameSavingOptions GetFrameSavingOptions();
	void UpdateDirectoryPath();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	void RunManualImageTask(FrameSavingOptions frameSavingOptions, FrameType taskType);
	Camera* BuildSelectedCamera(float exposureTimeSeconds);
	DECLARE_MESSAGE_MAP()
};


// Function for the manual camera worker thread
UINT takeManualImages( LPVOID pParam );