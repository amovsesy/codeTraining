
#ifndef EISAMPLEAPP_H
#define EISAMPLEAPP_H


#include "D3DApplication.h"
#include "FPS.h"
#include "d3dfile.h"






class SnakeApp : public D3DApplication
{
public:
	SnakeApp();
	~SnakeApp();

private:

	// GameApplication overrides
	virtual HBRUSH GetBackgroundBrush()  { return (HBRUSH)GetStockObject(BLACK_BRUSH); }
	virtual LPCSTR GetTitle()					{ return "Snake Sample"; }
	bool AppBegin();
	bool AppUpdate();
	bool AppEnd();
	bool AppActivate()		{ appActive = true;  return true; }
	bool AppDeactivate()	{ appActive = false; return true; }
	bool ProcessMessage ( UINT Message, WPARAM wParam, LPARAM lParam );
	void Paint();

	// D3DApplication overrides
	HRESULT InitDeviceObjects();
	HRESULT DeleteDeviceObjects();
	HRESULT RestoreDeviceObjects();
	HRESULT InvalidateDeviceObjects();

	// helper functions
	void DrawScene(LPDIRECT3DDEVICE8, const D3DVIEWPORT8&);
	void ShowFPS();
	void DrawLog(const D3DVIEWPORT8& viewport);

	void UpdateState();

private:
	bool appActive;

	D3DModel sphereModel;

	typedef std::deque<POINT> PointDeque;
	PointDeque pointDeque;

	int xInc;
	int yInc;

	bool grow, shrink;

	eiInputManager* inputMgr;
	eiKeyboard* keyboard;


	DWORD lastCycleTime;

	CD3DFont* arialFont;

	FpsData fps;
};


#endif