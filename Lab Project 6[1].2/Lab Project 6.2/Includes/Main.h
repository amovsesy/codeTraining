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
#include <stdio.h>
#include <D3DX9.h>

#define RANDOM_COLOR 0xFF000000 | ((rand() * 0xFFFFFF) / RAND_MAX)

#endif // _MAIN_H_