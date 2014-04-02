#include "stdafx.h"

#include "Tomography Control.h"
#include "Tomography ControlDlg.h"
#include "Exceptions.h"

#define BUFFER_SIZE 2000

Table::Table(CWnd* wnd)
{
	this -> m_messageReceiver = wnd;
	this -> m_thread = NULL;
}

Table::~Table()
{
	if (NULL != this -> m_thread)
	{
		delete this -> m_thread;
	}
}

void Table::ClearDisplay()
{
	this -> m_bufferLock.Lock();
	this -> m_displayBuffer.Empty();
	this -> m_bufferLock.Unlock();
}

void Table::PumpInputReceived()
{
	if (NULL != this -> m_messageReceiver
		&& ::IsWindow(this -> m_messageReceiver -> m_hWnd))
	{
		this -> m_messageReceiver -> PostMessage(WM_USER_TABLE_INPUT_RECEIVED, 0, (LPARAM)this);
	}
}

void Table::PumpOutputFinished()
{
	if (NULL != this -> m_messageReceiver
		&& ::IsWindow(this -> m_messageReceiver -> m_hWnd))
	{
		this -> m_messageReceiver -> PostMessage(WM_USER_TABLE_OUTPUT_FINISHED, 0, (LPARAM)this);
	}
}

void Table::SendToTable(LPCTSTR command)
{
	this -> m_bufferLock.Lock();
	m_sendBuffer += command;
	this -> m_bufferLock.Unlock();
	this -> m_inputToSendToTableEvent.PulseEvent();
}

/* Start the background thread for a table. This is intended to be called
 * at the end of a constructor for a subclass.
 */
void Table::Start()
{
	this -> m_running = TRUE;
	this -> m_thread = AfxBeginThread(communicateWithTable, this, THREAD_PRIORITY_NORMAL, 
		0, CREATE_SUSPENDED);
	this -> m_thread -> m_bAutoDelete = FALSE;
	this -> m_thread -> ResumeThread();
}

/* Stop the background thread for the table. This is intended to be called at the
 * start of the destructor for a subclass.
 */
void Table::Stop()
{
	this -> m_running = FALSE;
	if (NULL != this -> m_thread)
	{
		// Give the thread 5 seconds to exit
		::WaitForSingleObject(this -> m_thread -> m_hThread, 5000);
	}
}

UINT communicateWithTable( LPVOID pParam )
{
	Table* table = (Table*)pParam;

	while (table -> m_running)
	{
		table -> DoIO();

		::WaitForSingleObject(table -> m_inputToSendToTableEvent.m_hObject, TABLE_IO_DELAY_MILLIS);
	}

	return 0;
}