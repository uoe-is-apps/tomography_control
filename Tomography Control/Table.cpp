#include "stdafx.h"
#include "Tomography Control.h"
#include "Tomography ControlDlg.h"


Table::Table(char* gszPort) 
{
	this -> m_inputEvent.ResetEvent();
	this -> m_inputBuffer = new CString("", 1000);
	this -> m_outputBuffer = new CString("", 5000);

    DCB dcb;
    this -> m_hComm = CreateFile( gszPort,  
                    GENERIC_READ | GENERIC_WRITE, 
                    0, 
                    0, 
                    OPEN_EXISTING,
                    FILE_FLAG_OVERLAPPED,
                    0);

    if (this -> m_hComm == INVALID_HANDLE_VALUE) {
		throw "Could not open serial port.";
    }

	// Set up the port details
    FillMemory(&dcb, sizeof(dcb), 0);
    dcb.DCBlength = sizeof(dcb);
    if (!BuildCommDCB("9600,n,8,1", &dcb)) {   
        // Couldn't build the DCB. Usually a problem
        // with the communications specification string.
        throw "Could not construct serial port configuration.";
    }
    else
	{
		// DCB is ready for use.
	}
}

Table::~Table() 
{
    CloseHandle(this -> m_hComm);

	delete this -> m_inputBuffer;
	delete this -> m_outputBuffer;
}

void Table::SendTableCommand(char* command)
{
    /* CString currentCommand = ((CTomographyControlDlg*)dialog) -> m_tableCommandOutput;

    currentCommand += command;
    currentCommand += "\r\n";

    ((CTomographyControlDlg*)dialog) -> m_tableCommandOutput = currentCommand;

    dialog -> UpdateData(FALSE); */
}