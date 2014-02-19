#include "stdafx.h"
#include "Tomography Control.h"
#include "Tomography ControlDlg.h"
#include "Table And Camera Control.h"


CTableAndCameraControl::CTableAndCameraControl() 
{
}

void CTableAndCameraControl::SendTableCommand(CDialogEx* dialog, char* command)
{
	CString currentCommand = ((CTomographyControlDlg*)dialog) -> m_tableCommandOutput;

	currentCommand += command;
	currentCommand += "\r\n";

	((CTomographyControlDlg*)dialog) -> m_tableCommandOutput = currentCommand;

	dialog -> UpdateData(FALSE);
}