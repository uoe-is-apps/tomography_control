#include "stdafx.h"
#include "Table.h"
#include "Tomography Control.h"
#include "Tomography ControlDlg.h"

#define BUFFER_SIZE 2000

// Virtually identical to the regular table, except with a loopback
// in place of serial communications
DummyTable::DummyTable() 
{
	this -> m_running = TRUE;
	this -> m_inputEvent.ResetEvent();
	this -> m_inputBuffer = (char*)calloc(BUFFER_SIZE, sizeof(char));
	this -> m_outputBuffer = (char*)calloc(BUFFER_SIZE, sizeof(char));
}

DummyTable::~DummyTable() 
{
	free(this -> m_inputBuffer);
	free(this -> m_outputBuffer);
}


void DummyTable::DoIO()
{
	DWORD inputBufferSize = sizeof(char) * BUFFER_SIZE;
	DWORD outputBufferSize = sizeof(char) * BUFFER_SIZE;
	DWORD bufferFilled = 0;
	DWORD bytesRead;

	char tempBuffer[BUFFER_SIZE];

	// Lock the IO buffers while we exchange data between them
	this -> m_bufferLock.Lock();
	
	if (strlen(this -> m_inputBuffer) > 0)
	{
		// Copy the input to the temporary buffer as if we'd just read it in
		memcpy(tempBuffer, this -> m_inputBuffer, inputBufferSize);

		// Clear the input buffer
		m_inputBuffer[0] = NULL;
	
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

		// TODO: Send message to notify dialog that the output buffer has changed
	}

	this -> m_bufferLock.Unlock();
}

void DummyTable::SendTableCommand(char* command)
{
	int inputBufferSize = sizeof(*m_inputBuffer);
	int commandSize = strlen(command);

	this -> m_bufferLock.Lock();
	TODO: strcat_s(m_inputBuffer, sizeof(char) * BUFFER_SIZE, command);
	this -> m_bufferLock.Unlock();
	this -> m_inputEvent.PulseEvent();
}

void DummyTable::Start()
{
	this -> m_running = TRUE;

	// Note that communicateWithTable is defined in Table.cpp
	this -> m_thread = AfxBeginThread(communicateWithTable, this, THREAD_PRIORITY_NORMAL, 
		0, CREATE_SUSPENDED);
	this -> m_thread -> m_bAutoDelete = FALSE;
	this -> m_thread -> ResumeThread();
}

void DummyTable::Stop()
{
	this -> m_running = FALSE;
	// Give the thread 5 seconds to exit
	::WaitForSingleObject(this -> m_thread -> m_hThread, 5000);
	delete this -> m_thread;
}