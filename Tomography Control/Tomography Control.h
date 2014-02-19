
// Tomography Control.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

// Message for the worker threads to notify of data collection
// progress
#define WM_USER_TURN_COMPLETED (WM_USER + 1) 
#define WM_USER_STOP_COMPLETED (WM_USER_TURN_COMPLETED + 1) 
#define WM_USER_IMAGE_CAPTURED (WM_USER_STOP_COMPLETED + 1) 
#define WM_USER_RUN_FINISHED (WM_USER_IMAGE_CAPTURED + 1) 


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