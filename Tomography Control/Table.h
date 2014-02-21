
#pragma once
#include "afxwin.h"

class Table
{
public:
	Table(char* gszPort);
	~Table();
	void SendTableCommand(char* command);

protected:
	HANDLE m_hComm;
};