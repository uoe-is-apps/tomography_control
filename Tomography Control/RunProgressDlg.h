#pragma once


// CRunProgressDlg dialog

class CRunProgressDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRunProgressDlg)

public:
	CRunProgressDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRunProgressDlg();

// Dialog Data
	enum { IDD = IDD_RUN_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

// Function for the manual camera worker thread
UINT takeRunImages( LPVOID pParam );