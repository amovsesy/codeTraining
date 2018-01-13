//-----------------------------------------------------------------------------
// File: Main.h
//
// Desc: Main application entry / handling header file.
//
// Copyright (c) 1997-2002 Adam Hoult & Gary Simmons. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _MAIN_H_
#define _MAIN_H_

//-----------------------------------------------------------------------------
// Main Application Includes
//-----------------------------------------------------------------------------
#include "..\\Res\\resource.h"
#include <windows.h>
#include <tchar.h>
#include <D3DX9.h>

//-----------------------------------------------------------------------------
// Miscellaneous Macros
//-----------------------------------------------------------------------------
#define RGB2BGR( Color ) (Color & 0xFF000000) | ((Color & 0xFF0000) >> 16) | (Color & 0x00FF00) | ((Color & 0x0000FF) << 16)

#endif // _MAIN_H_