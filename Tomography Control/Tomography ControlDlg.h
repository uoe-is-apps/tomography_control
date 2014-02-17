
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
	
	CString m_TableCommand;
	CString m_TableCommandOutput;
	CEdit m_TableCommandControl;
};
