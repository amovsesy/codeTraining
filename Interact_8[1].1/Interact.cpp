#include "stdafx.h"
#include "d3dHelp.h"
#include "eiInput.h"
#include "eiLog.h"
#include "Interact.h"



const float CircleRadius = 200.0f;
const float AngleInc = 0.01f;

const float CameraDist = -450;
const float FOV = D3DX_PI/4.0f;

const char* SphereModelFile	= "sphere.x";


InteractApp theApp;



InteractApp::InteractApp()
	:	
		D3DApplication(),
		appActive(false),
		inputMgr(0),
		keyboard(0),
		sphere1angle(0.0f),
		sphere2angle(D3DX_PI),
		zBuffer(true),
		fillMode(D3DFILL_SOLID),
		shadeMode(D3DSHADE_GOURAUD)
{
	arialFont = new CD3DFont( "arial", 12 );
}

InteractApp::~InteractApp()
{
	delete arialFont;
}


bool InteractApp::AppBegin() 
{
	// initialize DirectInput
	inputMgr = new eiInputManager( GetAppWindow() );
	keyboard = new eiKeyboard();
	keyboard->Attach( 0, 100 );

	// initialize Direct3D
	InitializeDirect3D();

	// initialize frames per second object
	fps.Reset();

	LOGC( 0xffffff00, "F2 - z buffer\n");
	LOGC( 0xffffff00, "F3 - fill mode\n");
	LOGC( 0xffffff00, "F4 - shade mode\n");

	return true;
}

void InteractApp::ShowFPS()
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

void InteractApp::DrawScene(LPDIRECT3DDEVICE8 device, const D3DVIEWPORT8& viewport)
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


bool InteractApp::AppUpdate ()
{
	if (appActive)
		ProcessInput();
		
	
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

void InteractApp::ProcessInput()
{
	DIDEVICEOBJECTDATA event;
	while (keyboard->GetEvent( event ))
	{
		if (event.dwData & 0x80)
		{
			LPDIRECT3DDEVICE8 device = Get3DDevice();
			assert(device);

			if (event.dwOfs == DIK_F2)
			{
				if (zBuffer)
				{
					LOG("z-buffer disabled\n");
				    device->SetRenderState( D3DRS_ZENABLE, FALSE );
					zBuffer = false;
				}
				else
				{
					LOG("z-buffer enabled\n");
				    device->SetRenderState( D3DRS_ZENABLE, TRUE );
					zBuffer = true;
				}
			}
			else if (event.dwOfs == DIK_F3)
			{
				if (fillMode == D3DFILL_SOLID)
				{
					LOG("fill mode: WIREFRAME\n");
					device->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
					fillMode = D3DFILL_WIREFRAME;
				}
				else if (fillMode == D3DFILL_WIREFRAME)
				{
					LOG("fill mode: POINT\n");
					device->SetRenderState( D3DRS_FILLMODE, D3DFILL_POINT );
					fillMode = D3DFILL_POINT;
				}
				else
				{
					LOG("fill mode: SOLID\n");
					device->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
					fillMode = D3DFILL_SOLID;
				}

			}
			else if (event.dwOfs == DIK_F4)
			{
				if (shadeMode == D3DSHADE_FLAT)
				{
					LOG("shade mode: GOURAUD\n");
					device->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
					shadeMode = D3DSHADE_GOURAUD;
				}
				else
				{
					LOG("shade mode: FLAT\n");
					device->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );
					shadeMode = D3DSHADE_FLAT;
				}

			}
		}
	}
}

void InteractApp::DrawLog(const D3DVIEWPORT8& viewport)
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



bool InteractApp::AppEnd ()
{
	// release model data
	sphereModel.Release();

	// terminate DirectInput
	delete keyboard;
	delete inputMgr;


	// terminate Direct3D
	ShutdownDirect3D();

	// final log file flush
	eiLogDestroy();

	return true;
}

bool InteractApp::ProcessMessage(UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
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

	return false;
}

HRESULT InteractApp::InitDeviceObjects()
{
	LPDIRECT3DDEVICE8 device = Get3DDevice();

	if (sphereModel.Load( SphereModelFile, device ) == false)
	{
		char errStr[100];
		sprintf( errStr, "can't find %s", SphereModelFile );
		MessageBox( GetAppWindow(), errStr, "Pong", MB_OK | MB_ICONEXCLAMATION);
		return MK_E_CANTOPENFILE;
	}

	arialFont->InitDeviceObjects( device );

	return S_OK;
}

HRESULT InteractApp::RestoreDeviceObjects()
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

HRESULT InteractApp::DeleteDeviceObjects()
{
	sphereModel.Release();

	arialFont->DeleteDeviceObjects();

	return S_OK;
}

HRESULT InteractApp::InvalidateDeviceObjects()
{
    arialFont->InvalidateDeviceObjects();

	sphereModel.InvalidateModel();

    return S_OK;
}
