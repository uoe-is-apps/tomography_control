
// Tomography Control.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

// Message for the worker threads to notify of data collection
// progress
#define WM_USER_THREAD_FINISHED (WM_USER + 1) 
#define WM_USER_CAPTURING_FRAME (WM_USER_THREAD_FINISHED + 1) 
#define WM_USER_FRAME_CAPTURED (WM_USER_CAPTURING_FRAME + 1) 
#define WM_USER_TABLE_OUTPUT_UPDATED (WM_USER_FRAME_CAPTURED + 1) 
#define WM_USER_RUN_TURN_COMPLETED (WM_USER_TABLE_OUTPUT_UPDATED + 1) 
#define WM_USER_RUN_TABLE_ANGLE_CHANGED (WM_USER_RUN_TURN_COMPLETED + 1) 
#define WM_USER_RUN_STOP_COMPLETED (WM_USER_RUN_TABLE_ANGLE_CHANGED + 1) 
#define WM_USER_RUN_FINISHED (WM_USER_RUN_STOP_COMPLETED + 1) 

#define ERROR_BUFFER_SIZE 1024

// CTomographyControlApp:
// See Tomography Control.cpp for the implementation of this class
//

class CTomographyControlApp : public CWinApp
{
public:
	CTomographyControlApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CTomographyControlApp theApp;
