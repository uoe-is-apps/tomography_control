
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

