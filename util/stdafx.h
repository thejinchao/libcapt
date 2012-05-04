#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500		//for Windows2000
#endif						

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410	//for Windows98
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0400		//for IE4.0
#endif

#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include <atldlgs.h>
#include <atlscrl.h>

#include <string>
#include <vector>
#include <algorithm>

#include <captGenerator.h>
#include <captFontFile.h>
#include <captRLE.h>

#ifdef UNICODE
typedef std::wstring String;
#else
typedef std::string String;
#endif
