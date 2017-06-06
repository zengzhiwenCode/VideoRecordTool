// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include "duilib.h"

#include "C:/dev/Global/global.h"
#include "C:/dev/Global/win32.h"
using namespace jlib;

#include <DShow.h>

#include <streams.h>
#include <strsafe.h>

#include <d3d9.h>
#include <Vmr9.h>
#include <Evr.h>

#pragma warning(disable:4995)
#include <cstdlib>
#include <Windows.h>
#include <streams.h>
#include <strsafe.h>

#include "DShowUtil.h"
#include "smartptr.h"   // smart pointer class
#include "crossbar.h"
#include "SampleCGB.h"

#include <string>
#include <memory>
#include <list>
#include <map>
#include "DShowPlayer.h"

#define WM_FGNOTIFY (WM_USER+1)
#define WM_GRAPH_EVENT (WM_USER+2)


#pragma warning(push)
#pragma warning(disable:4819)
#include <opencv2/opencv.hpp>
#pragma warning(pop)
