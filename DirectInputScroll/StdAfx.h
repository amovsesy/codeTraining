
#pragma warning( disable : 4786 )  // disable browser symbol truncation warning (stl templates)


#pragma once


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers


// Windows Header Files:
#include <windows.h>
#include <mmsystem.h>
#include <assert.h>
#include <list>

#include <dxerr8.h>



void ReportDirectXError(const char* title, HRESULT r, const char* file, int line, const char* str );

