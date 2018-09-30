//----------------------------------------------------------------------------------------------
// StdAfx.h
//----------------------------------------------------------------------------------------------
#ifndef STDAFX_H
#define STDAFX_H

#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

#pragma comment(lib, "mf")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "strmiids")
#pragma comment(lib, "evr")

//----------------------------------------------------------------------------------------------
// Microsoft DirectX SDK (June 2010)
#ifdef _WIN64
//#pragma comment(lib, "C:\\Program Files\\Microsoft DirectX SDK (June 2010)\\Lib\\x64\\d3dx9")
#pragma comment(lib, "C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Lib\\x64\\d3dx9")
#else
//#pragma comment(lib, "C:\\Program Files\\Microsoft DirectX SDK (June 2010)\\Lib\\x86\\d3dx9")
#pragma comment(lib, "C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Lib\\x86\\d3dx9")
#endif

//----------------------------------------------------------------------------------------------
// Microsoft Windows SDK for Windows 7
#include <WinSDKVer.h>
#include <new>
#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include <strsafe.h>
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#include <initguid.h>
#include <Shlwapi.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <Mftransform.h>
#include <evr.h>
#include <uuids.h>
#include <d3d9.h>
#include <Evr9.h>
#include <wmcodecdsp.h>
#include <wmsdkidl.h>
#include <ks.h>
#include <Ksmedia.h>

//----------------------------------------------------------------------------------------------
// Microsoft DirectX SDK (June 2010)
//#include "C:\Program Files\Microsoft DirectX SDK (June 2010)\Include\d3dx9.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx9.h"

//----------------------------------------------------------------------------------------------
// Common Project Files
#ifdef _DEBUG
#define MF_USE_LOGGING 1
//#define MF_USE_LOGREFCOUNT
//#define MF_TRACE_TRANSFORM
#else
#define MF_USE_LOGGING 0
#endif

#include "..\..\Common\MFMacro.h"
#include "..\..\Common\MFTrace.h"
#include "..\..\Common\MFLogging.h"
#include "..\..\Common\MFTExternTrace.h"
#include "..\..\Common\MFGuid.h"
#include "..\..\Common\MFClassFactory.h"
#include "..\..\Common\MFCriticSection.h"
#include "..\..\Common\MFRegistry.h"
#include "..\..\Common\MFTime.h"
#include "..\..\Common\MFLogCommon.h"
#include "..\..\Common\MFLogMediaType.h"
#include "..\..\Common\MFLogSample.h"

//----------------------------------------------------------------------------------------------
// Project Files
#include "resource.h"
#include "Quad.h"
#include "Text2D.h"
#include "MFTVideoShaderEffect.h"

extern HMODULE g_hModule;

#endif