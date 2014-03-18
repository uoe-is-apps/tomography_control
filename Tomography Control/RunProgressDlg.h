#pragma once

#include "Camera.h"
#include "Table.h"
#include "afxcmn.h"

// Tracks details of a data collection run task. This also captures current
// state, so that it's guaranteed to exist beyond the lifecycle of the
// thread
struct RunTask
{
	CWnd* m_dialog;

	Camera* m_camera;
	Table* m_table;

	FrameSavingOptions m_frameSavingOptions;
	CString m_directoryPath;
	u_int m_turnsTotal;
	u_int m_stopsPerTurn;
	u_int m_framesPerStop;
	float m_exposureTimeSeconds;

	u_int m_turnCount;
	u_int m_stopCount;
	u_int m_currentPosition;

	BOOL m_running;
};

// CRunProgressDlg dialog

class CRunProgressDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRunProgressDlg)

public:
	CRunProgressDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRunProgressDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_RUN_DIALOG };

	FrameSavingOptions m_frameSavingOptions;

	CString m_directoryPath;
	CString m_imageFilename;
	int m_currentPosition;
	float m_exposureTimeSeconds;
	int m_framesCaptured;
	int m_framesPerStop;
	int m_stopsCompleted;
	int m_stopsPerRotation;
	int m_turnsCompleted;
	int m_turnsTotal;
	CString m_calculatedAngle;

	CString m_startTime;
	CString m_estEndTime;
	CString m_estRunTime;

	CProgressCtrl m_progressCtl;
	
	Camera* m_camera;
	Table* m_table;
	CWinThread* m_workerThread;
	RunTask *m_task;

	void CalculateTimeRemaining(CTimeSpan* dest);

	// Called when the worker thread completes image captures within a rotation of the table
	afx_msg LRESULT OnTurnCompleted(WPARAM wParam, LPARAM lParam);

	// Called when the worker thread completes rotating the table to a new position
	afx_msg LRESULT CRunProgressDlg::OnTableAngleChanged(WPARAM wParam, LPARAM lParam);

	// Called when the worker thread completes a set of image captures
	afx_msg LRESULT OnStopCompleted(WPARAM wParam, LPARAM lParam);

	// Called when the worker thread started the frame capture process
	afx_msg LRESULT OnFrameCaptureStarted(WPARAM wParam, LPARAM lParam);

	// Called when the worker thread captures a single frame
	afx_msg LRESULT OnFrameCaptured(WPARAM wParam, LPARAM lParam);

	// Called when the worker thread finishes
	afx_msg LRESULT OnThreadFinished(WPARAM wParam, LPARAM lParam);

	// Overriden to stop close messages getting through
	afx_msg void OnClose();
	afx_msg void OnBnClickedCancel();

	void WriteSettings(CString dest);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	char m_errorBuffer[ERROR_BUFFER_SIZE];
};

// Function for the manual camera worker thread
UINT captureRunFrames( LPVOID pParam );