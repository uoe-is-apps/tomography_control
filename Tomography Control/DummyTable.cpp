#include "stdafx.h"
#include "Table.h"
#include "Tomography Control.h"
#include "Tomography ControlDlg.h"

#define BUFFER_SIZE 2000

// Virtually identical to the regular table, except with a loopback
// in place of serial communications
DummyTable::DummyTable(CWnd* wnd) : Table(wnd) 
{
	this -> m_running = TRUE;
	this -> m_inputToSendToTableEvent.ResetEvent();
	this -> m_sendBuffer.Empty();
	this -> m_displayBuffer.Empty();

	this -> Start();
}

DummyTable::~DummyTable() 
{
	this -> Stop();
}


void DummyTable::DoIO()
{
	// Lock the IO buffers while we exchange data between them
	this -> m_bufferLock.Lock();
	
	if (!this -> m_sendBuffer.IsEmpty())
	{
		// Copy the input to the temporary buffer as if we'd just read it in
		this -> m_displayBuffer += this -> m_sendBuffer;

		// Clear the input buffer
		this -> m_sendBuffer.Empty();

		this -> PumpInputReceived();

		// Automatically notify any waiting threads that the table
		// has responded, as the table doesn't produce any status
		// output by itself
		this -> m_outputReceivedFromTableEvent.PulseEvent();

		if (this -> m_sendBuffer.Trim().IsEmpty())
		{
			// Notify the window to refresh the display as we've copied
			// the output to the display
			this -> PumpOutputFinished();
		}
	}

	this -> m_bufferLock.Unlock();
}