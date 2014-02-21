
#pragma once
#include "afxwin.h"

class Table
{
public:
	Table(char* gszPort);
	~Table();
	void SendTableCommand(char* command);

protected:
	CEvent m_inputEvent;
	CString* m_inputBuffer; // Communication waiting to be sent to the table
	CString* m_outputBuffer; // Communication back from the table
	HANDLE m_hComm;
};