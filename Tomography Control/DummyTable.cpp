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

		this -> PulseMessageReceived();
	}

	this -> m_bufferLock.Unlock();
}