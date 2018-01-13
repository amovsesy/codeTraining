#include "stdafx.h"
#include "GameApplication.h"


const char* szTitle = "SimpleWindowClass Sample";
const char* szHello = "hello!";


class SimpleAppClass : public GameApplication
{
private:
	virtual void GetWindowDims(long& w, long& h )	{ w = 320; h = 240; }
	virtual LPCSTR GetTitle()						{ return szTitle; }

	virtual bool Paint(HWND hwnd, WPARAM, LPARAM)
	{
		PAINTSTRUCT ps;
		HDC hdc;
		hdc = BeginPaint( hwnd, &ps );
		RECT rt;
		GetClientRect( hwnd, &rt );
		DrawText( hdc, szHello, strlen(szHello), &rt, DT_CENTER );
		EndPaint( hwnd, &ps );
		return true;
	}
};


SimpleAppClass theApp;


