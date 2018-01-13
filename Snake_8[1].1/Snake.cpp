#include "stdafx.h"
#include "d3dHelp.h"
#include "eiInput.h"
#include "eiLog.h"
#include "Snake.h"


const int LimitX = 200;
const int LimitY = 150;
const int IncX = 7;
const int IncY = 4;
const int SnakeLenInitial = 50;
const int SnakeLenMax = 300;
const int SnakeLenMin = 2;
const char* SphereModelFile	= "lowball.x";


const int TimerFreq = 60;
const int TimerInterval = 1000/TimerFreq;
const int TimerRes = 10;

const float CameraDist = -450;
const float FOV = D3DX_PI/4.0f;



SnakeApp theApp;



SnakeApp::SnakeApp()
	:	
		D3DApplication(),
		appActive(false),
		xInc(IncX),
		yInc(IncY),
		inputMgr(0),
		keyboard(0),
		grow(false),
		shrink(false),
		lastCycleTime(0)
{
	arialFont = new CD3DFont( "arial", 12 );
}

SnakeApp::~SnakeApp()
{
	delete arialFont;
}


bool SnakeApp::AppBegin() 
{
	// initialize DirectInput
	inputMgr = new eiInputManager( GetAppWindow() );
	keyboard = new eiKeyboard();
	keyboard->Attach( 0, 100 );

	// initialize Direct3D
	UseDepthBuffer( false );
	HRESULT hr = InitializeDirect3D();
	if (FAILED(hr))
	{
		return false;
	}

	// prepare the "snake"
	int x = 0, y = 0;
	for (int i=0; i<SnakeLenInitial; i++)
	{
		POINT pt = { x, y };
		x-=xInc;
		y-=yInc;
		pointDeque.push_back( pt );
	}

	// initialize frames per second object
	fps.Reset();

	LOGC( 0xff00ffff, "F2 - grow\n");
	LOGC( 0xff00ffff, "F3 - shrink\n");

	return true;
}

void SnakeApp::ShowFPS()
{
	if (fps.Valid())
	{
		char fpsStr[20];
		sprintf( fpsStr, "%.2f fps", fps.LastReading());
		arialFont->DrawText( 3, 3, 0xaaaaaaaa, fpsStr, 0L );
	}
}

void SnakeApp::DrawScene(LPDIRECT3DDEVICE8 device, const D3DVIEWPORT8& viewport)
{
	PointDeque::iterator it;

	for (it = pointDeque.begin(); it != pointDeque.end(); it++ )
	{
		POINT pt = *it;
		float x = static_cast<float>(pt.x);
		float y = static_cast<float>(pt.y);
		sphereModel.SetLocation( x, y, 0.0f );
		sphereModel.Render();
	}

	char str[20];
	sprintf( str, "length %d", pointDeque.size());
	arialFont->DrawText( 3, 24, 0xaaaaaaaa, str, 0L );
}


bool SnakeApp::AppUpdate ()
{
	DWORD timeNow = timeGetTime();
	
	if (timeNow >= lastCycleTime + TimerInterval)
	{
		UpdateState();
		lastCycleTime = timeNow;
	}

	LPDIRECT3DDEVICE8 device = Get3DDevice();

	HRESULT hr;
	if( FAILED( hr = device->TestCooperativeLevel() ) )
	{
		if( D3DERR_DEVICELOST == hr )
			return true;

		if( D3DERR_DEVICENOTRESET == hr )
		{
			if( FAILED( hr = Reset3DEnvironment() ) )
				return true;
		}
		return true;
	}

	Clear3DBuffer( 0 );

	if (SUCCEEDED(device->BeginScene()))
	{
		D3DVIEWPORT8 viewport;
		device->GetViewport( &viewport );

		DrawScene( device, viewport );

		ShowFPS();
		DrawLog( viewport );

		device->EndScene();

		Present3DBuffer();

		fps.NewFrame();
	}

	return true;
}

void SnakeApp::UpdateState()
{
	if (!appActive)
		return;

	DIDEVICEOBJECTDATA event;
	while (keyboard->GetEvent( event ))
	{
		if (event.dwOfs == DIK_F2)
		{
			if (event.dwData & 0x80)
				grow = true;
			else 
				grow = false;

		}
		else if (event.dwOfs == DIK_F3)
		{
			if (event.dwData & 0x80)
				shrink = true;
			else 
				shrink = false;
		}
	}

	int size = pointDeque.size();

	if ( ! grow || size >= SnakeLenMax )
		pointDeque.pop_back();


	if  (! shrink || size <= SnakeLenMin)
	{
		POINT pt = pointDeque.front();
		pt.x += xInc;
		pt.y += yInc;
		if (pt.x > LimitX || pt.x < -LimitX)
		{
			xInc =- xInc;
			pt.x += xInc;
		}
		if (pt.y > LimitY || pt.y < -LimitY)
		{
			yInc =- yInc;
			pt.y += yInc;
		}
		pointDeque.push_front( pt );
	}
}

void SnakeApp::DrawLog(const D3DVIEWPORT8& viewport)
{
	eiLogLock();

	int maxVisible = (viewport.Height - viewport.Y) / FONT_PITCH - 1;
	int numItems = eiLogNumEntries();
	int drawCount = min( maxVisible, numItems );
	int output_y = FONT_PITCH * (drawCount-1);

	eiLogResetIterator();

	for (int i = 0; i < drawCount; i++)
	{
		const LogEntry* p = eiLogEntry();

		DWORD color;
		if (p->clr == 0)
			color = 0xaaaaaaaa;
		else
			color = p->clr;

		Draw2DText( arialFont, viewport.Width/2 + viewport.Width/4, output_y, color, p->str );

		output_y -= FONT_PITCH;
		eiLogNextEntry();
	}

	eiLogUnlock();
}



bool SnakeApp::AppEnd ()
{
	// release model data
	sphereModel.Release();
	
	// terminate Direct3D
	ShutdownDirect3D();

	// terminate DirectInput
	delete keyboard;
	delete inputMgr;

	// final log file flush
	eiLogDestroy();

	return true;
}

bool SnakeApp::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_SYSCOMMAND:
			// Prevent moving/sizing and power loss
			switch( wParam )
			{
			case SC_MOVE:
			case SC_SIZE:
			case SC_MAXIMIZE:
			case SC_KEYMENU:
			case SC_MONITORPOWER:
			return true;
			}
			break;
		case WM_SYSKEYDOWN:
			if( VK_RETURN == wParam )
				ToggleFullscreen();
			break;
	};

	return D3DApplication::ProcessMessage( msg, wParam, lParam );
}

HRESULT SnakeApp::InitDeviceObjects()
{
	LPDIRECT3DDEVICE8 device = Get3DDevice();

	if (sphereModel.Load( SphereModelFile, device ) == false)
	{
		char errStr[100];
		sprintf( errStr, "can't find %s", SphereModelFile );
		MessageBox( GetAppWindow(), errStr, "Pong", MB_OK | MB_ICONEXCLAMATION);
		return D3DAPPERR_MEDIANOTFOUND;
	}

//	sphereModel.Scale( 2.5f );

	arialFont->InitDeviceObjects( device );

	return S_OK;
}

HRESULT SnakeApp::RestoreDeviceObjects()
{
	sphereModel.RestoreModel();
	arialFont->RestoreDeviceObjects();

	LPDIRECT3DDEVICE8 device = Get3DDevice();
	const D3DSURFACE_DESC& backBuf = GetBackBufferDesc();

	D3DXVECTOR3 vEyePt    = D3DXVECTOR3( 0.0f, 0.0f, CameraDist );
	D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f,  0.0f );
	D3DXVECTOR3 vUpVec    = D3DXVECTOR3( 0.0f, 1.0f,  0.0f );

	D3DXMATRIX  matWorld, matView, matProj;
	D3DXMatrixIdentity( &matWorld );
	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
	FLOAT fAspect = ((FLOAT)backBuf.Width) / backBuf.Height;
	D3DXMatrixPerspectiveFovLH( &matProj, FOV, fAspect, 1.0f, 4000.0f );

	device->SetTransform( D3DTS_WORLD,      &matWorld );
	device->SetTransform( D3DTS_VIEW,       &matView );
	device->SetTransform( D3DTS_PROJECTION, &matProj );
	
	D3DLIGHT8 light;
	D3DUtil_InitLight( light, D3DLIGHT_DIRECTIONAL, -5.0f, -0.0f, 0.0f );
	device->SetLight( 0, &light );
	device->LightEnable( 0, TRUE );

	device->SetRenderState( D3DRS_LIGHTING, TRUE );
	device->SetRenderState( D3DRS_AMBIENT, 0x33333333 );

	return S_OK;
}

HRESULT SnakeApp::DeleteDeviceObjects()
{
	sphereModel.Release();

	arialFont->DeleteDeviceObjects();

	return S_OK;
}

HRESULT SnakeApp::InvalidateDeviceObjects()
{
    arialFont->InvalidateDeviceObjects();

	sphereModel.InvalidateModel();

    return S_OK;
}
