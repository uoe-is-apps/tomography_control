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
	this -> m_inputEvent.ResetEvent();
	this -> m_inputBuffer.Empty();
	this -> m_outputBuffer.Empty();

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
	
	if (!this -> m_inputBuffer.IsEmpty())
	{
		// Copy the input to the temporary buffer as if we'd just read it in
		this -> m_outputBuffer += this -> m_inputBuffer;

		// Clear the input buffer
		this -> m_inputBuffer.IsEmpty();

		this -> PulseMessageReceived();
	}

	this -> m_bufferLock.Unlock();
}