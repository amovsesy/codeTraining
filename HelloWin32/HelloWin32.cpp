#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	MessageBox( 0, "Hello Win32!", "WinMain", MB_OK | MB_ICONEXCLAMATION);


	return 0;
}



