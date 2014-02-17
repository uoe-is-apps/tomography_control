#include "stdafx.h"
#include "Tomography Control.h"
#include "Tomography ControlDlg.h"
#include "Table And Camera Control.h"


CTableAndCameraControl::CTableAndCameraControl() 
{
}

void CTableAndCameraControl::SendTableCommand(CDialogEx* dialog, char* command)
{
	CString currentCommand = ((CTomographyControlDlg*)dialog) -> m_TableCommandOutput;

	currentCommand += command;
	currentCommand += "\r\n";

	((CTomographyControlDlg*)dialog) -> m_TableCommandOutput = currentCommand;

	dialog -> UpdateData(FALSE);
}