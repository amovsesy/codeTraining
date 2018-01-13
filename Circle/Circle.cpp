#include "stdafx.h"
#include "d3dHelp.h"
#include "eiInput.h"
#include "eiLog.h"
#include "Circle.h"



const float CircleRadius = 200.0f;
const float AngleInc = 0.01f;

const float CameraDist = -450;
const float FOV = D3DX_PI/4.0f;

const char* SphereModelFile	= "sphere.x";


CircleApp theApp;



CircleApp::CircleApp()
	:	
		D3DApplication(),
		appActive(false),
		sphere1angle(0.0f),
		sphere2angle(D3DX_PI)
{
	arialFont = new CD3DFont( "arial", 12 );
}

CircleApp::~CircleApp()
{
	delete arialFont;
}


bool CircleApp::AppBegin() 
{
	// initialize Direct3D
	HRESULT hr = InitializeDirect3D();
	if (FAILED(hr))
	{
		return false;
	}

	// initialize frames per second object
	fps.Reset();

	return true;
}

void CircleApp::ShowFPS()
{
	if (fps.Valid())
	{
		SIZE size;
		char fpsStr[20];
		sprintf( fpsStr, "%.2f fps", fps.LastReading());
		arialFont->GetTextExtent(fpsStr, &size);
		arialFont->DrawText( 0, 0, 0xaaaaaaaa, fpsStr, 0L );
	}
}

void CircleApp::DrawScene(LPDIRECT3DDEVICE8 device, const D3DVIEWPORT8& viewport)
{
	sphere1angle += AngleInc;

	float x1 = cos( sphere1angle ) * CircleRadius;
	float z1 = sin( sphere1angle ) * CircleRadius;

	sphereModel.SetLocation( x1, 0.0f, z1 );
	sphereModel.Render();

//	LOG("%.2f %.2f\n" , x1, z1);

	sphere2angle += AngleInc;
	float x2 = cos( sphere2angle ) * CircleRadius;
	float z2 = sin( sphere2angle ) * CircleRadius;

	sphereModel.SetLocation( x2, 0.0f, z2 );
	sphereModel.Render();
}


bool CircleApp::AppUpdate ()
{
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

void CircleApp::DrawLog(const D3DVIEWPORT8& viewport)
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

		Draw2DText( arialFont, viewport.Width/2, output_y, color, p->str );

		output_y -= FONT_PITCH;
		eiLogNextEntry();
	}

	eiLogUnlock();
}



bool CircleApp::AppEnd ()
{
	// release model data
	sphereModel.Release();
	
	// terminate Direct3D
	ShutdownDirect3D();

	// final log file flush
	eiLogDestroy();

	return true;
}

bool CircleApp::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam)
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

HRESULT CircleApp::InitDeviceObjects()
{
	LPDIRECT3DDEVICE8 device = Get3DDevice();

	if (sphereModel.Load( SphereModelFile, device ) == false)
	{
		char errStr[100];
		sprintf( errStr, "can't find %s", SphereModelFile );
		MessageBox( GetAppWindow(), errStr, "Pong", MB_OK | MB_ICONEXCLAMATION);
		return D3DAPPERR_MEDIANOTFOUND;
	}

	arialFont->InitDeviceObjects( device );

	return S_OK;
}

HRESULT CircleApp::RestoreDeviceObjects()
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
	D3DUtil_InitLight( light, D3DLIGHT_DIRECTIONAL, -5.0f, -2.5f, 5.0f );
	device->SetLight( 0, &light );
	device->LightEnable( 0, TRUE );

	device->SetRenderState( D3DRS_LIGHTING, TRUE );
	device->SetRenderState( D3DRS_AMBIENT, 0x33333333 );

	return S_OK;
}

HRESULT CircleApp::DeleteDeviceObjects()
{
	sphereModel.Release();

	arialFont->DeleteDeviceObjects();

	return S_OK;
}

HRESULT CircleApp::InvalidateDeviceObjects()
{
    arialFont->InvalidateDeviceObjects();

	sphereModel.InvalidateModel();

    return S_OK;
}
