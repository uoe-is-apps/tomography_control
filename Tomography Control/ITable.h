
#pragma once
#include "afxwin.h"

class ITable
{
public:
	virtual void DoIO() = 0;
	virtual void SendTableCommand(char* command) = 0;
	virtual void Start() = 0;
	virtual void SetMessageReceiver(CWnd* wnd) = 0;
	virtual void Stop() = 0;

protected:
	virtual void PulseMessageReceived() = 0;
};
