
#pragma once
#include "afxwin.h"
#include "ITable.h"

class Table : public ITable
{
public:
	Table(char* gszPort);
	~Table();
	void DoIO();
	void SendTableCommand(char* command);
	void Start();
	void Stop();

	CEvent m_inputEvent;
	BOOL m_running;

protected:
	CCriticalSection m_bufferLock;
	char* m_inputBuffer; // Communication waiting to be sent to the table
	char* m_outputBuffer; // Communication back from the table
	HANDLE m_hComm;
	CWinThread* m_thread;
};

class DummyTable : public ITable
{
public:
	DummyTable();
	~DummyTable();
	void DoIO();
	void SendTableCommand(char* command);
	void Start();
	void Stop();

	CEvent m_inputEvent;
	BOOL m_running;

protected:
	CCriticalSection m_bufferLock;
	char* m_inputBuffer; // Communication waiting to be sent to the table
	char* m_outputBuffer; // Communication back from the table
	CWinThread* m_thread;
};

// Function for the table communication thread
UINT communicateWithTable( LPVOID pParam );