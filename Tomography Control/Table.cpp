#include "stdafx.h"

#include "Tomography Control.h"
#include "Tomography ControlDlg.h"
#include "Exceptions.h"

#define BUFFER_SIZE 2000

SerialTable::SerialTable(char* gszPort) 
{
	this -> m_inputEvent.ResetEvent();
	this -> m_inputBuffer.clear();
	this -> m_outputBuffer.clear();
	
    FillMemory(&this -> m_dcb, sizeof(this -> m_dcb), 0);
    FillMemory(&this -> m_commTimeouts, sizeof(this -> m_commTimeouts), 0);
    this -> m_dcb.DCBlength = sizeof(this -> m_dcb);

    this -> m_hComm = CreateFile( gszPort,  
                    GENERIC_READ | GENERIC_WRITE, 
                    0, 
                    NULL, 
                    OPEN_EXISTING,
                    0,
                    NULL);

    if (this -> m_hComm == INVALID_HANDLE_VALUE) {
		throw new bad_serial_port_error("Could not open serial port.");
    }

	// Set up the port details
    if (!BuildCommDCB("baud=19200 parity=N data=8 stop=1", &this -> m_dcb)) {   
        // Couldn't build the DCB. Usually a problem
        // with the communications specification string.
        throw new bad_serial_port_error("Could not construct serial port configuration.");
    }
    else
	{
		// DCB is ready for use.
	}

	if (SetCommState(this -> m_hComm, &this -> m_dcb) == 0)
	{
        throw new bad_serial_port_error("Could not set port state.");
	}
	
	this -> m_commTimeouts.ReadIntervalTimeout = MAXDWORD;
	this -> m_commTimeouts.ReadTotalTimeoutMultiplier = 0;
	this -> m_commTimeouts.ReadTotalTimeoutConstant = 0;
	this -> m_commTimeouts.WriteTotalTimeoutMultiplier = 0;
	this -> m_commTimeouts.WriteTotalTimeoutConstant = 0;

	SetCommTimeouts(this -> m_hComm, &this -> m_commTimeouts);

}

SerialTable::~SerialTable() 
{
    CloseHandle(this -> m_hComm);
}

/* Get pending input from the table, and write new input out to it. */
void SerialTable::DoIO()
{
	DoWrite();
	DoRead();
}
	
void SerialTable::DoRead()
{
	DWORD bytesRead = 0;
	char tempBuffer[BUFFER_SIZE];
	
	tempBuffer[0] = NULL;
	
	if (!ReadFile(this -> m_hComm, tempBuffer, BUFFER_SIZE - 1, &bytesRead, NULL))
	{
		char errorBuffer[ERROR_BUFFER_SIZE];

		sprintf_s(errorBuffer, ERROR_BUFFER_SIZE - 1, "Error reading from table: %d", GetLastError());

		MessageBox(NULL, errorBuffer, "Tomography Control", MB_ICONERROR);
		return;
	}

	// Copy the incoming string to the output buffer
	if (bytesRead > 0)
	{
		// Lock the IO buffers while we exchange data with them
		this -> m_bufferLock.Lock();

		tempBuffer[bytesRead] = NULL;
		this -> m_outputBuffer += tempBuffer;
		
		this -> PulseMessageReceived();

		this -> m_bufferLock.Unlock();
	}
}

void SerialTable::DoWrite()
{
	// Lock the IO buffers while we exchange data with them
	this -> m_bufferLock.Lock();

	// If we have input to be sent, take a copy then release the locks on the buffers
	if (!this -> m_inputBuffer.empty())
	{
		const char *input = this -> m_inputBuffer.c_str();
		DWORD bytesWritten;

		// Write the contents of the temp buffer out to serial
		WriteFile(this -> m_hComm, input, strlen(input), &bytesWritten, NULL);

		// TODO: Report error if not all bytes have been written

		// Copy the input to the temporary buffer as if we'd just read it in
		this -> m_outputBuffer += this -> m_inputBuffer;
		this -> PulseMessageReceived();

		// Clear the input buffer
		this -> m_inputBuffer.clear();
	}

	this -> m_bufferLock.Unlock();
}

void Table::PulseMessageReceived()
{
	if (NULL != this -> m_messageReceiver
		&& ::IsWindow(this -> m_messageReceiver -> m_hWnd))
	{
		this -> m_messageReceiver -> PostMessage(WM_USER_TABLE_MESSAGE_RECEIVED, 0, (LPARAM)this);
	}
}

void Table::SendTableCommand(char* command)
{
	this -> m_bufferLock.Lock();
	m_inputBuffer += command;
	this -> m_bufferLock.Unlock();
	this -> m_inputEvent.PulseEvent();
}

void Table::SetMessageReceiver(CWnd* wnd)
{
	this -> m_messageReceiver = wnd;
}

void Table::Start()
{
	this -> m_running = TRUE;
	this -> m_thread = AfxBeginThread(communicateWithTable, this, THREAD_PRIORITY_NORMAL, 
		0, CREATE_SUSPENDED);
	this -> m_thread -> m_bAutoDelete = FALSE;
	this -> m_thread -> ResumeThread();
}

void Table::Stop()
{
	this -> m_running = FALSE;
	// Give the thread 5 seconds to exit
	::WaitForSingleObject(this -> m_thread -> m_hThread, 5000);
	delete this -> m_thread;
}

UINT communicateWithTable( LPVOID pParam )
{
	Table* table = (Table*)pParam;

	while (table -> m_running)
	{
		table -> DoIO();

		::WaitForSingleObject(table -> m_inputEvent.m_hObject, 50);
	}

	return 0;
}