#include "stdafx.h"
#include <stdio.h>


const char* szTitle = "SimpleWindow Sample";
const char* szWindowClass = "SimpleWindowClass";
const char* szHello = "hello!";


bool RegisterWindowClass();
bool CreateSimpleWindow(int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


HINSTANCE hInst = 0;
HWND hwnd = 0;


void ReportLastError()
{
	void* lpMsgBuf;

	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			0, GetLastError(),  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );

	MessageBox( 0, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );

	LocalFree( lpMsgBuf );
 
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	hInst = hInstance;

	if ( ! RegisterWindowClass())
	{
		MessageBox( 0, "Failed to register window class", "Error", MB_ICONEXCLAMATION | MB_ICONWARNING );
		return FALSE;
	}

	if ( ! CreateSimpleWindow( nCmdShow )) 
	{
		MessageBox( 0, "Failed to create window", "Error", MB_ICONEXCLAMATION | MB_ICONWARNING );
		return FALSE;
	}

	MSG msg;
	while (GetMessage( &msg, NULL, 0, 0 )) 
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	return msg.wParam;
}

bool RegisterWindowClass()
{
	WNDCLASS wc;
	ZeroMemory( &wc, sizeof(wc) );

	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= WndProc;
	wc.hInstance		= hInst;
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName	= szWindowClass;

	if ( RegisterClass( &wc ) == 0 )
		return false;

	return true;
}

bool CreateSimpleWindow(int nCmdShow)
{
	hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
				100, 100, 320, 240, NULL, NULL, hInst, NULL);

	if ( ! hwnd)
		return false;

	ShowWindow( hwnd, nCmdShow );
	UpdateWindow( hwnd );

	return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
		case WM_PAINT:
			PAINTSTRUCT ps;
			HDC hdc;
			hdc = BeginPaint( hWnd, &ps );
			RECT rt;
			GetClientRect( hWnd, &rt );
			DrawText( hdc, szHello, strlen(szHello), &rt, DT_CENTER );
			EndPaint( hWnd, &ps );
			break;
		case WM_DESTROY:
			PostQuitMessage( 0 );
			break;
		case WM_KEYDOWN:
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
