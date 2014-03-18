
#pragma once
#include <string>
#include "afxwin.h"

#define TABLE_IO_DELAY_MILLIS 40

class Table
{
public:
	Table(CWnd* wnd);
	virtual ~Table();

	virtual void DoIO() = 0;
	virtual void PumpOutputUpdated();
	virtual void SendToTable(LPCTSTR command);
	
	CEvent m_inputEvent;
	BOOL m_running;
	CCriticalSection m_bufferLock;
	CString m_inputBuffer; // Communication waiting to be sent to the table
	CString m_outputBuffer; // Communication back from the table

protected:

	CWinThread* m_thread;
	CWnd* m_messageReceiver;
	
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