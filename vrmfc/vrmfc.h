
// vrmfc.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CvrmfcApp:
// See vrmfc.cpp for the implementation of this class
//

class CvrmfcApp : public CWinApp
{
public:
	CvrmfcApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CvrmfcApp theApp;

CString tr(UINT uid);
