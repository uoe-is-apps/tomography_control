#include "stdafx.h"
#include "Tomography Control.h"
#include "Tomography ControlDlg.h"

#define BUFFER_SIZE 2000

Table::Table(char* gszPort) 
{
	this -> m_inputEvent.ResetEvent();
	this -> m_inputBuffer = (char*)calloc(BUFFER_SIZE, sizeof(char));
	this -> m_outputBuffer = (char*)calloc(BUFFER_SIZE, sizeof(char));

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

	free(this -> m_inputBuffer);
	free(this -> m_outputBuffer);
}

/* Get pending input from the table, and write new input out to it. */
void Table::DoIO()
{
	DWORD inputBufferSize = sizeof(char) * BUFFER_SIZE;
	DWORD outputBufferSize = sizeof(char) * BUFFER_SIZE;
	DWORD bufferFilled = 0;
	DWORD bytesRead;
	char tempBuffer[BUFFER_SIZE];
	memset(tempBuffer, 0, sizeof(char) * BUFFER_SIZE);

	do {
		if (ReadFile(this -> m_hComm, tempBuffer, sizeof(tempBuffer) - bytesRead - 1, &bytesRead, NULL) == 0)
		{
			// TODO: Indicate failure somehow
		}

		bytesRead += bufferFilled;
	} while(bytesRead > 0
		&& bufferFilled < (sizeof(tempBuffer) - 1));

	// Lock the IO buffers while we exchange data with them
	this -> m_bufferLock.Lock();
	
	// Calculate the overflow on the output buffer
	DWORD spare = outputBufferSize - strlen(this -> m_outputBuffer);

	// If the buffer overflows, drop everything past the overflow point, and
	// move the rest to the top of the buffer.
	if (strlen(tempBuffer) > spare)
	{
		DWORD overflow = 0 - (spare - strlen(tempBuffer));
		DWORD keep = outputBufferSize - overflow; // Number of characters to keep
		// Copy that many characters from the mid to the start
		for (DWORD charIdx = 0; charIdx < keep; charIdx++)
		{
			*(this -> m_outputBuffer + charIdx) = *(this -> m_outputBuffer + charIdx + keep);
		}
		*(this -> m_outputBuffer + keep) = 0;
	}
	
	strcat_s(this -> m_outputBuffer, outputBufferSize, tempBuffer);

	// If we have input to be sent, take a copy then release the locks on the buffers
	if (strlen(this -> m_inputBuffer) > 0)
	{
		memcpy(tempBuffer, this -> m_inputBuffer, inputBufferSize);

		// Clear the input buffer
		m_inputBuffer[0] = NULL;
	}
	else
	{
		tempBuffer[0] = NULL;
	}

	this -> m_bufferLock.Unlock();

	DWORD bytesWritten;

	WriteFile(this -> m_hComm, tempBuffer, strlen(tempBuffer), &bytesWritten, NULL);

	// TODO: Report error if not all bytes have been written
}

void Table::SendTableCommand(char* command)
{
	this -> m_bufferLock.Lock();
	strcat_s(m_inputBuffer, BUFFER_SIZE, command);
	this -> m_bufferLock.Unlock();
	this -> m_inputEvent.PulseEvent();
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

		::WaitForSingleObject(table -> m_inputEvent.m_hObject, 5000);
	}

	return 0;
}