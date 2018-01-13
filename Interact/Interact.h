
#ifndef EISAMPLEAPP_H
#define EISAMPLEAPP_H


#include "D3DApplication.h"
#include "FPS.h"
#include "d3dfile.h"


class eiInputManager;
class eiKeyboard;
class eiPointer;
class eiGameController;



class InteractApp : public D3DApplication
{
public:
	InteractApp();
	~InteractApp();

private:

	// GameApplication overrides
	virtual HBRUSH GetBackgroundBrush()  { return (HBRUSH)GetStockObject(BLACK_BRUSH); }
	virtual LPCSTR GetTitle()					{ return "Interact Sample"; }
	bool AppBegin();
	bool AppUpdate();
	bool AppEnd();
	bool AppActivate()		{ appActive = true;  return true; }
	bool AppDeactivate()	{ appActive = false; return true; }
	bool ProcessMessage ( UINT Message, WPARAM wParam, LPARAM lParam );
	void Paint();

	// D3DApplication overrides
	HRESULT RestoreDeviceObjects();
	HRESULT InitDeviceObjects();
	HRESULT DeleteDeviceObjects();
	HRESULT InvalidateDeviceObjects();

	// helper functions
	void ProcessInput();
	void DrawScene(LPDIRECT3DDEVICE8, const D3DVIEWPORT8&);
	void ShowFPS();
	void DrawLog(const D3DVIEWPORT8& viewport);

private:
	bool appActive;

	eiInputManager* inputMgr;
	eiKeyboard* keyboard;
	
	D3DModel sphereModel;
	float sphere1angle, sphere2angle;
	bool zBuffer;
	D3DFILLMODE fillMode;
	D3DSHADEMODE shadeMode;

	CD3DFont* arialFont;


	FpsData fps;
};


#endif