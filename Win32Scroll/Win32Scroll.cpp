#include "stdafx.h"
#include "GameApplication.h"


const char* szTitle = "Win32Scroll Sample";
const char* szHello = "hello!";


const int MaxLogEntries = 60;
const int FontPitch = 16;
const int TimerInterval = 500;


class SimpleApp : public GameApplication
{
private:

	virtual HBRUSH GetBackgroundBrush()  { return (HBRUSH)GetStockObject(BLACK_BRUSH); }
	virtual void GetWindowDims(long& w, long& h )	{ w = 320; h = 320; }
	virtual LPCSTR GetTitle()						{ return szTitle; }

	virtual bool AppBegin()
	{
		font = CreateFont( 20, 0, 0, 0,
				FW_NORMAL, FALSE, FALSE, FALSE,
				ANSI_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,
				VARIABLE_PITCH,
				"Arial" );

		lastTime = timeGetTime();

		showTimerTick = false;
		showMouseMove = false;

		Log("F2 = toggle timer display");
		Log("F3 = toggle mouse display");

		return true;
	}

	virtual bool AppEnd()
	{
		return true;
	}

	virtual bool AppUpdate()
	{
		DWORD timeNow = timeGetTime();

		// use the timeThen member and the TimerInterval constant
		// to determine if the proper interval has passed

		return true;
	}

	virtual bool KeyDown(long vk, long keyData)
	{
		// use the VK_F2 and VK_F3 symbols to compare 'vk'

		Log("Keydown %d", vk);

		return true;
	}

	virtual bool Paint(HWND hwnd, WPARAM, LPARAM)
	{
		PAINTSTRUCT ps;
		HDC hdc;
		hdc = BeginPaint( hwnd, &ps );
		SelectObject( hdc, font );
		SetBkMode( hdc, TRANSPARENT );
		SetTextColor( hdc, RGB(255,255,255) );

		RECT rt;
		GetClientRect( hwnd, &rt );
		rt.left += 4;

		int maxVisible = rt.bottom / FontPitch - 1;
		int numItems = logEntries.size();
		int drawCount = min( maxVisible, numItems );
		int y = FontPitch * (drawCount-1);

		for (StringList::iterator s = logEntries.begin(); s != logEntries.end(); s++)
		{
			const std::string& str = *s;
			rt.top = y;
			DrawText( hdc, str.data(), str.length(), &rt, DT_LEFT );
			y -= FontPitch;
		}
		EndPaint( hwnd, &ps );
		return true;
	}

	void pascal Log(LPCTSTR format, ...)
	{
		va_list args;
		va_start(args, format);

		char buffer[80 + 1];
#ifdef BORLAND
		vsnprintf(buffer, sizeof(buffer)/sizeof(buffer[0]), format, args);
#else
		_vsnprintf(buffer, sizeof(buffer)/sizeof(buffer[0]), format, args);
#endif
		logEntries.push_front( buffer );

		while (logEntries.size() > MaxLogEntries)
			logEntries.pop_back();

		va_end(args);

		RedrawWindow( GetAppWindow(), 0, 0, RDW_INVALIDATE | RDW_ERASE);
	}

private:

	typedef std::list<std::string> StringList;

	StringList logEntries;

	bool showTimerTick;
	bool showMouseMove;

	DWORD lastTime;
	HFONT font;

};


SimpleApp theApp;


