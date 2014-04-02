
#pragma once
#include <string>
#include "afxwin.h"

#define TABLE_IO_DELAY_MILLIS 40

class Table
{
public:
	Table(CWnd* wnd);
	virtual ~Table();
	
	BOOL m_running;
	CString m_displayBuffer; // Input sent to the table, plus anything received from the table
	CEvent m_inputToSendToTableEvent;
	CEvent m_outputReceivedFromTableEvent;
	CCriticalSection m_bufferLock;

	virtual void DoIO() = 0;
	virtual void ClearDisplay();
	virtual void PumpInputReceived();
	virtual void PumpOutputFinished();
	virtual void SendToTable(LPCTSTR command);

protected:

	CWinThread* m_thread;
	CWnd* m_messageReceiver;
	
	CString m_sendBuffer; // Communication waiting to be sent to the table
	
	void Start();
	void Stop();
};

class SerialTable : public Table
{
public:
	SerialTable(CWnd* wnd, LPCTSTR gszPort);
	~SerialTable();
	void DoIO();

protected:
	void DoRead();
	void DoWrite();

	HANDLE m_hComm;
    DCB m_dcb;
	COMMTIMEOUTS m_commTimeouts;

	// Time when a line was last sent to the table, for flow-control purposes
	DWORD m_lastWriteTicks;
};

class DummyTable : public Table
{
public:
	DummyTable(CWnd* wnd);
	~DummyTable();
	void DoIO();


protected:
};

// Function for the table communication thread
UINT communicateWithTable( LPVOID pParam );