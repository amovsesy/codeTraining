#include "stdafx.h"
#include "GameApplication.h"
#include "eiInput.h"


const char* szTitle = "DirectInputScroll Sample";


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

		inputMgr = new eiInputManager( GetAppWindow() );

		keyboard = new eiKeyboard();
		mouse = new eiPointer();

		keyboard->Attach( 0, 50 );
		mouse->Attach( 0, 256 );

		return true;
	}

	virtual bool AppEnd()
	{
		delete keyboard;
		delete mouse;
		delete inputMgr;


		return true;
	}

	virtual bool AppUpdate()
	{
		if (appActive)
		{
			CheckKeyboard();
			CheckMouse();
		}


		DWORD timeNow = timeGetTime();
		if (timeNow > lastTime + TimerInterval)
		{
			if (showTimerTick)
				Log("tick");
			lastTime = timeNow;
		}
		return true;
	}

	void CheckKeyboard()
	{
		DIDEVICEOBJECTDATA event;
		while (keyboard->GetEvent( event ))
		{
			if ( event.dwData & 0x80 )
			{
				if (event.dwOfs == DIK_F2)
				{
					showTimerTick = !showTimerTick;
					if (showTimerTick)
						Log("timer tick display on");
					else
						Log("timer tick display off");
				}
				else if (event.dwOfs == DIK_F3)
				{
					showMouseMove = !showMouseMove;
					if (showMouseMove)
						Log("mouse move display on");
					else
						Log("mouse move display off");
				}
				else
					Log("Keydown %d", event.dwOfs);
			}
			
		}
	}

	void CheckMouse()
	{
		DIDEVICEOBJECTDATA event;
		while (mouse->GetEvent( event ))
		{
			if (showMouseMove)
			{
				if (event.dwOfs == DIMOFS_X)
					Log("mouse X axis = %d", event.dwData);
				if (event.dwOfs == DIMOFS_Y)
					Log("mouse Y axis = %d", event.dwData);
			}
		}
	}

	virtual bool AppActivate()
	{
		appActive = true;
		return true;
	}

	virtual bool AppDeactivate()
	{
		appActive = false;
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
		_vsnprintf(buffer, sizeof(buffer)/sizeof(buffer[0]), format, args);
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

	eiInputManager* inputMgr;
	eiKeyboard* keyboard;
	eiPointer* mouse;

	bool appActive;
};


SimpleApp theApp;


