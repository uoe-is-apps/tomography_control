
#pragma once
#include "afxwin.h"

class Table
{
public:
	Table(char* gszPort);
	~Table();
	void DoIO();
	void SendTableCommand(char* command);

	BOOL m_running;
	CEvent m_inputEvent;

protected:
	CCriticalSection m_bufferLock;
	char* m_inputBuffer; // Communication waiting to be sent to the table
	char* m_outputBuffer; // Communication back from the table
	HANDLE m_hComm;
};

// Function for the table communication thread
UINT communicateWithTable( LPVOID pParam );