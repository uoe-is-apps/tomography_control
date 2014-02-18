#pragma once


// CTakingPhotosDlg dialog

class CTakingPhotosDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTakingPhotosDlg)

public:
	CTakingPhotosDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTakingPhotosDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_TAKING_PHOTOS_DIALOG };

	enum TaskType { SINGLE, DARK, FLAT_FIELD };

	TaskType m_taskType;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
