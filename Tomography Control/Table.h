
#pragma once
#include <string>
#include "afxwin.h"

class Table
{
public:
	virtual void DoIO() = 0;
	virtual void PulseMessageReceived();
	virtual void SendTableCommand(char* command);
	virtual void SetMessageReceiver(CWnd* wnd);
	virtual void Start();
	virtual void Stop();

	CEvent m_inputEvent;
	BOOL m_running;
	CCriticalSection m_bufferLock;
	std::string m_inputBuffer; // Communication waiting to be sent to the table
	std::string m_outputBuffer; // Communication back from the table

protected:

	CWinThread* m_thread;
	CWnd* m_messageReceiver;
};

class SerialTable : public Table
{
public:
	SerialTable(char* gszPort);
	~SerialTable();
	void DoIO();

protected:
	HANDLE m_hComm;
};

class DummyTable : public Table
{
public:
	DummyTable();
	~DummyTable();
	void DoIO();

protected:
};

// Function for the table communication thread
UINT communicateWithTable( LPVOID pParam );