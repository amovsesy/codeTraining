//-----------------------------------------------------------------------------
// File: CGameApp.cpp
//
// Desc: Game Application class, this is the central hub for all app processing
//
// Copyright (c) 1997-2002 Adam Hoult & Gary Simmons. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CGameApp Specific Includes
//-----------------------------------------------------------------------------
#include "..\\Includes\\CGameApp.h"

//-----------------------------------------------------------------------------
// CGameApp Member Functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CGameApp () (Constructor)
// Desc : CGameApp Class Constructor
//-----------------------------------------------------------------------------
CGameApp::CGameApp()
{
	// Reset / Clear all required values
    m_hWnd          = NULL;
    m_pD3D          = NULL;
    m_pD3DDevice    = NULL;
    m_bLostDevice   = false;

}

//-----------------------------------------------------------------------------
// Name : ~CGameApp () (Destructor)
// Desc : CGameApp Class Destructor
//-----------------------------------------------------------------------------
CGameApp::~CGameApp()
{
	// Shut the engine down
    ShutDown();
}

//-----------------------------------------------------------------------------
// Name : InitInstance ()
// Desc : Initialises the entire Engine here.
//-----------------------------------------------------------------------------
bool CGameApp::InitInstance( HANDLE hInstance, LPCTSTR lpCmdLine, int iCmdShow )
{
    // Create the primary display device
    if (!CreateDisplay()) { ShutDown(); return false; }

    // Build Objects
    if (!BuildObjects()) { ShutDown(); return false; }

    // Set up all required game states
    SetupGameState();

    // Setup our rendering environment
    SetupRenderStates();

    // Success!
	return true;
}

//-----------------------------------------------------------------------------
// Name : CreateDisplay ()
// Desc : Create the display windows, devices etc, ready for rendering.
//-----------------------------------------------------------------------------
bool CGameApp::CreateDisplay()
{
	LPTSTR WindowTitle  = _T("Initialization");
    USHORT Width        = 400;
    USHORT Height       = 400;
    RECT   rc;

    // Register the new windows window class.
    WNDCLASS			wc;	
	wc.style			= CS_BYTEALIGNCLIENT | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= StaticWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= (HINSTANCE)GetModuleHandle(NULL);
	wc.hIcon			= LoadIcon( wc.hInstance, MAKEINTRESOURCE(IDI_ICON));
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH )GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= WindowTitle;
	RegisterClass(&wc);

    // Create the rendering window
	m_hWnd = CreateWindow( WindowTitle, WindowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 
                           CW_USEDEFAULT, Width, Height, NULL, LoadMenu( wc.hInstance, MAKEINTRESOURCE(IDR_MENU) ),
                           wc.hInstance, this );
	
    // Bail on error
    if (!m_hWnd) return false;

    // Retrieve the final client size of the window
    ::GetClientRect( m_hWnd, &rc );
    m_nViewX      = rc.left;
    m_nViewY      = rc.top;
    m_nViewWidth  = rc.right - rc.left;
    m_nViewHeight = rc.bottom - rc.top;

    // Show the window
	ShowWindow(m_hWnd, SW_SHOW);

    // Initialize Direct3D (Simple)
    if (!InitDirect3D( )) return false;

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : InitDirect3D () (Private)
// Desc : Performs a simple, non-enumerated, initialization of Direct3D
//-----------------------------------------------------------------------------
bool CGameApp::InitDirect3D()
{
    HRESULT               hRet;
    D3DPRESENT_PARAMETERS PresentParams;
    D3DCAPS9              Caps;
	D3DDISPLAYMODE        CurrentMode;
	
    // First of all create our D3D Object
    m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
    if (!m_pD3D) 
    {
        MessageBox( m_hWnd, _T("No compatible Direct3D object could be created."), _T("Fatal Error!"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        return false;
    
    } // End if failure

    // Fill out a simple set of present parameters
    ZeroMemory( &PresentParams, sizeof(D3DPRESENT_PARAMETERS) );

    // Select back buffer format etc
	m_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &CurrentMode);
	PresentParams.BackBufferFormat = CurrentMode.Format;
        
	// Setup remaining flags
	PresentParams.AutoDepthStencilFormat = FindDepthStencilFormat( D3DADAPTER_DEFAULT, CurrentMode, D3DDEVTYPE_HAL );
    PresentParams.SwapEffect			 = D3DSWAPEFFECT_DISCARD;
	PresentParams.Windowed				 = true;
	PresentParams.EnableAutoDepthStencil = true;
    
	// Set Creation Flags
	unsigned long ulFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

    // Check if Hardware T&L is available
    ZeroMemory( &Caps, sizeof(D3DCAPS9) );
    m_pD3D->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &Caps );
    if ( Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) ulFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
        
    // Attempt to create a HAL device
    if( FAILED( hRet = m_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd, ulFlags, &PresentParams, &m_pD3DDevice ) ) ) 
    {
        MessageBox( m_hWnd, _T("Could not create a valid HAL Direct3D device object.\r\n\r\n")
                            _T("The system will now attempt to create a device utilising the 'Reference Rasterizer' (D3DDEVTYPE_REF)"),
                            _T("Fatal Error!"), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL );
        
        // Find REF depth buffer format
        PresentParams.AutoDepthStencilFormat = FindDepthStencilFormat( D3DADAPTER_DEFAULT, CurrentMode, D3DDEVTYPE_REF );

        // Check if Hardware T&L is available
        ZeroMemory( &Caps, sizeof(D3DCAPS9) );
        ulFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        m_pD3D->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, &Caps );
        if ( Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) ulFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;

        // Attempt to create a REF device
        if( FAILED( hRet = m_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, m_hWnd, ulFlags, &PresentParams, &m_pD3DDevice ) ) ) 
        {
            MessageBox( m_hWnd, _T("Could not create a valid REF Direct3D device object.\r\n\r\nThe system will now exit."),
                                _T("Fatal Error!"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );

            // Failed
            return false;
        
        } // End if Failure (REF)

    } // End if Failure (HAL)

    // Store the present parameters
    m_D3DPresentParams = PresentParams;

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : FindDepthStencilFormat ()
// Desc : This function simply determines the best depth format that is
//        available for the specified mode.
// Note : No tests for stencil active depth buffers are made.
//-----------------------------------------------------------------------------
D3DFORMAT CGameApp::FindDepthStencilFormat( ULONG AdapterOrdinal, D3DDISPLAYMODE Mode, D3DDEVTYPE DevType )
{

    // Test for 24 bith depth buffer
    if (SUCCEEDED( m_pD3D->CheckDeviceFormat(AdapterOrdinal, DevType, Mode.Format, D3DUSAGE_DEPTHSTENCIL , D3DRTYPE_SURFACE , D3DFMT_D32 )))
    {
        if (SUCCEEDED( m_pD3D->CheckDepthStencilMatch ( AdapterOrdinal, DevType, Mode.Format, Mode.Format, D3DFMT_D32 ))) return D3DFMT_D32;
    
    } // End if 32bpp Available

    // Test for 24 bit depth buffer
    if (SUCCEEDED( m_pD3D->CheckDeviceFormat(AdapterOrdinal, DevType, Mode.Format, D3DUSAGE_DEPTHSTENCIL , D3DRTYPE_SURFACE , D3DFMT_D24X8 )))
    {
        if (SUCCEEDED( m_pD3D->CheckDepthStencilMatch ( AdapterOrdinal, DevType, Mode.Format, Mode.Format, D3DFMT_D24X8 ))) return D3DFMT_D24X8;
    
    } // End if 24bpp Available

    // Test for 16 bit depth buffer
    if (SUCCEEDED( m_pD3D->CheckDeviceFormat(AdapterOrdinal, DevType, Mode.Format, D3DUSAGE_DEPTHSTENCIL , D3DRTYPE_SURFACE , D3DFMT_D16 )))
    {
        if (SUCCEEDED( m_pD3D->CheckDepthStencilMatch ( AdapterOrdinal, DevType, Mode.Format, Mode.Format, D3DFMT_D16 ))) return D3DFMT_D16;
    
    } // End if 16bpp Available

    // No depth buffer supported
    return D3DFMT_UNKNOWN;

}

//-----------------------------------------------------------------------------
// Name : SetupGameState ()
// Desc : Sets up all the initial states required by the game.
//-----------------------------------------------------------------------------
void CGameApp::SetupGameState()
{
    // Setup Default Matrix Values
    D3DXMatrixIdentity( &m_mtxView );
    
    // Enable rotation
    m_bRotation1 = true;
    m_bRotation2 = true;

    // App is active
    m_bActive    = true;

}

//-----------------------------------------------------------------------------
// Name : SetupRenderStates ()
// Desc : Sets up all the initial states required by the renderer.
//-----------------------------------------------------------------------------
void CGameApp::SetupRenderStates()
{
    // Set up new perspective projection matrix
    float fAspect = (float)m_nViewWidth / (float)m_nViewHeight;
    D3DXMatrixPerspectiveFovLH( &m_mtxProjection, D3DXToRadian( 60.0f ), fAspect, 1.01f, 1000.0f );

    // Setup our D3D Device initial states
    m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
    m_pD3DDevice->SetRenderState( D3DRS_DITHERENABLE,  TRUE );
    m_pD3DDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
    m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
    m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    // Setup our vertex FVF code
    m_pD3DDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );

    // Setup our matrices
    m_pD3DDevice->SetTransform( D3DTS_VIEW, &m_mtxView );
    m_pD3DDevice->SetTransform( D3DTS_PROJECTION, &m_mtxProjection );
}

//-----------------------------------------------------------------------------
// Name : BeginGame ()
// Desc : Signals the beginning of the physical post-initialisation stage.
//        From here on, the game engine has control over processing.
//-----------------------------------------------------------------------------
int CGameApp::BeginGame()
{
    MSG		msg;

    // Start main loop
	while (1) 
    {
        // Did we recieve a message, or are we idling ?
		if ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) 
        {
			if (msg.message == WM_QUIT) break;
			TranslateMessage( &msg );
			DispatchMessage ( &msg );
		} 
        else 
        {
			// Advance Game Frame.
			FrameAdvance();

		} // End If messages waiting
	
    } // Until quit message is receieved

    return 0;
}

//-----------------------------------------------------------------------------
// Name : ShutDown ()
// Desc : Shuts down the game engine, and frees up all resources.
//-----------------------------------------------------------------------------
bool CGameApp::ShutDown()
{
    // Destroy Direct3D Objects
    if ( m_pD3DDevice ) m_pD3DDevice->Release();
    if ( m_pD3D       ) m_pD3D->Release();
    m_pD3D       = NULL;
    m_pD3DDevice = NULL;
    
    // Destroy the render window
    if ( m_hWnd ) DestroyWindow( m_hWnd );
    m_hWnd = NULL;
    
    // Shutdown Success
    return true;
}

//-----------------------------------------------------------------------------
// Name : StaticWndProc () (Static Callback)
// Desc : This is the main messge pump for ALL display devices, it captures
//        the appropriate messages, and routes them through to the application
//        class for which it was intended, therefore giving full class access.
// Note : It is VITALLY important that you should pass your 'this' pointer to
//        the lpParam parameter of the CreateWindow function if you wish to be
//        able to pass messages back to that app object.
//-----------------------------------------------------------------------------
LRESULT CALLBACK CGameApp::StaticWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    // If this is a create message, trap the 'this' pointer passed in and store it within the window.
    if ( Message == WM_CREATE ) SetWindowLong( hWnd, GWL_USERDATA, (LONG)((CREATESTRUCT FAR *)lParam)->lpCreateParams);

    // Obtain the correct destination for this message
    CGameApp *Destination = (CGameApp*)GetWindowLong( hWnd, GWL_USERDATA );
    
    // If the hWnd has a related class, pass it through
    if (Destination) return Destination->DisplayWndProc( hWnd, Message, wParam, lParam );
    
    // No destination found, defer to system...
    return DefWindowProc( hWnd, Message, wParam, lParam );
}

//-----------------------------------------------------------------------------
// Name : DisplayWndProc ()
// Desc : The display devices internal WndProc function. All messages being
//        passed to this function are relative to the window it owns.
//-----------------------------------------------------------------------------
LRESULT CGameApp::DisplayWndProc( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam )
{
    // Determine message type
	switch (Message)
    {
		case WM_CREATE:
            break;
		
        case WM_CLOSE:
			PostQuitMessage(0);
			break;
		
        case WM_DESTROY:
			PostQuitMessage(0);
			break;
		
        case WM_SIZE:

            if ( wParam == SIZE_MINIMIZED )
            {
                // App is inactive
                m_bActive = false;
            
            } // App has been minimized
            else
            {
                // App is active
                m_bActive = true;

                // Store new viewport sizes
                m_nViewWidth  = LOWORD( lParam );
                m_nViewHeight = HIWORD( lParam );
        
                if (m_pD3DDevice) 
                {
                    // Reset the device
                    m_pD3DDevice->Reset( &m_D3DPresentParams );
                    SetupRenderStates( );
                
                } // End if
            
            } // End if !Minimized

			break;

        case WM_KEYDOWN:

            // Which key was pressed?
			switch (wParam) 
            {
				case VK_ESCAPE:
					PostQuitMessage(0);
					return 0;
                    
			} // End Switch

			break;

        case WM_COMMAND:

            // Process Menu Items
            switch( LOWORD(wParam) )
            {
                case ID_ANIM_ROTATION1:
                    // Disable / enable rotation
                    m_bRotation1 = !m_bRotation1;
                    ::CheckMenuItem( ::GetMenu( m_hWnd ), ID_ANIM_ROTATION1, 
                                     MF_BYCOMMAND | (m_bRotation1) ? MF_CHECKED :  MF_UNCHECKED );
                    break;

                case ID_ANIM_ROTATION2:
                    // Disable / enable rotation
                    m_bRotation2 = !m_bRotation2;
                    ::CheckMenuItem( ::GetMenu( m_hWnd ), ID_ANIM_ROTATION2, 
                                     MF_BYCOMMAND | (m_bRotation2) ? MF_CHECKED :  MF_UNCHECKED );
                    break;

                case ID_EXIT:
                    // Recieved key/menu command to exit app
                    SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
                    return 0;
            
            } // End Switch

		default:
			return DefWindowProc(hWnd, Message, wParam, lParam);

    } // End Message Switch
    
    return 0;
}

//-----------------------------------------------------------------------------
// Name : BuildObjects ()
// Desc : Build our demonstration cube mesh, and the objects that instance it
//-----------------------------------------------------------------------------
bool CGameApp::BuildObjects()
{
    CPolygon * pPoly = NULL;

    // Seed the random number generator
    srand( timeGetTime() );

    // Add 6 polygons to this mesh.
    if ( m_Mesh.AddPolygon( 6 ) < 0 ) return false;

    // Front Face
    pPoly = m_Mesh.m_pPolygon[0];
    if ( pPoly->AddVertex( 4 ) < 0 ) return false;
    
    pPoly->m_pVertex[0] = CVertex( -2,  2, -2, RANDOM_COLOR );
    pPoly->m_pVertex[1] = CVertex(  2,  2, -2, RANDOM_COLOR );
    pPoly->m_pVertex[2] = CVertex(  2, -2, -2, RANDOM_COLOR );
    pPoly->m_pVertex[3] = CVertex( -2, -2, -2, RANDOM_COLOR );
    
    // Top Face
    pPoly = m_Mesh.m_pPolygon[1];
    if ( pPoly->AddVertex( 4 ) < 0 ) return false;
    
    pPoly->m_pVertex[0] = CVertex( -2,  2,  2, RANDOM_COLOR );
    pPoly->m_pVertex[1] = CVertex(  2,  2,  2, RANDOM_COLOR );
    pPoly->m_pVertex[2] = CVertex(  2,  2, -2, RANDOM_COLOR );
    pPoly->m_pVertex[3] = CVertex( -2,  2, -2, RANDOM_COLOR );

    // Back Face
    pPoly = m_Mesh.m_pPolygon[2];
    if ( pPoly->AddVertex( 4 ) < 0 ) return false;

    pPoly->m_pVertex[0] = CVertex( -2, -2,  2, RANDOM_COLOR );
    pPoly->m_pVertex[1] = CVertex(  2, -2,  2, RANDOM_COLOR );
    pPoly->m_pVertex[2] = CVertex(  2,  2,  2, RANDOM_COLOR );
    pPoly->m_pVertex[3] = CVertex( -2,  2,  2, RANDOM_COLOR );

    // Bottom Face
    pPoly = m_Mesh.m_pPolygon[3];
    if ( pPoly->AddVertex( 4 ) < 0 ) return false;

    pPoly->m_pVertex[0] = CVertex( -2, -2, -2, RANDOM_COLOR );
    pPoly->m_pVertex[1] = CVertex(  2, -2, -2, RANDOM_COLOR );
    pPoly->m_pVertex[2] = CVertex(  2, -2,  2, RANDOM_COLOR );
    pPoly->m_pVertex[3] = CVertex( -2, -2,  2, RANDOM_COLOR );

    // Left Face
    pPoly = m_Mesh.m_pPolygon[4];
    if ( pPoly->AddVertex( 4 ) < 0 ) return false;

    pPoly->m_pVertex[0] = CVertex( -2,  2,  2, RANDOM_COLOR );
    pPoly->m_pVertex[1] = CVertex( -2,  2, -2, RANDOM_COLOR );
    pPoly->m_pVertex[2] = CVertex( -2, -2, -2, RANDOM_COLOR );
    pPoly->m_pVertex[3] = CVertex( -2, -2,  2, RANDOM_COLOR );

    // Right Face
    pPoly = m_Mesh.m_pPolygon[5];
    if ( pPoly->AddVertex( 4 ) < 0 ) return false;

    pPoly->m_pVertex[0] = CVertex(  2,  2, -2, RANDOM_COLOR );
    pPoly->m_pVertex[1] = CVertex(  2,  2,  2, RANDOM_COLOR ); 
    pPoly->m_pVertex[2] = CVertex(  2, -2,  2, RANDOM_COLOR );
    pPoly->m_pVertex[3] = CVertex(  2, -2, -2, RANDOM_COLOR );

    // Our two objects should reference this mesh
    m_pObject[ 0 ].m_pMesh = &m_Mesh;
    m_pObject[ 1 ].m_pMesh = &m_Mesh;

    // Set both objects matrices so that they are offset slightly
    D3DXMatrixTranslation( &m_pObject[ 0 ].m_mtxWorld, -3.5f,  2.0f, 14.0f );
    D3DXMatrixTranslation( &m_pObject[ 1 ].m_mtxWorld,  3.5f, -2.0f, 14.0f );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : FrameAdvance () (Private)
// Desc : Called to signal that we are now rendering the next frame.
//-----------------------------------------------------------------------------
void CGameApp::FrameAdvance()
{
    CMesh      *pMesh = NULL;
 
    // Advance the timer
    m_Timer.Tick( );
    
    // Skip if app is inactive
    if ( !m_bActive ) return;

    // Recover lost device if required
    if ( m_bLostDevice )
    {
        // Can we reset the device yet ?
        HRESULT hRet = m_pD3DDevice->TestCooperativeLevel();
        if ( hRet == D3DERR_DEVICENOTRESET )
        {
            // Restore the device
            m_pD3DDevice->Reset( &m_D3DPresentParams );
            SetupRenderStates( );
            m_bLostDevice = false;
        
        } // End if can reset
        else
        {
            return;

        } // End if cannot reset

    } // End if Device Lost

    // Poll & Process input devices
    ProcessInput();

    // Animate the two objects
    AnimateObjects();

    // Clear the frame & depth buffer ready for drawing
    m_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0 );
    
    // Begin Scene Rendering
    m_pD3DDevice->BeginScene();

    // Loop through each object
    for ( ULONG i = 0; i < 2; i++ )
    {
        // Store mesh for easy access
        pMesh = m_pObject[i].m_pMesh;

        // Set our object matrix
        m_pD3DDevice->SetTransform( D3DTS_WORLD, &m_pObject[i].m_mtxWorld );

        // Loop through each polygon
        for ( ULONG f = 0; f < pMesh->m_nPolygonCount; f++ )
        {
            CPolygon * pPolygon = pMesh->m_pPolygon[f];
            
            // Render the primitive
            m_pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, pPolygon->m_nVertexCount - 2, pPolygon->m_pVertex, sizeof(CVertex) );
    
        } // Next Polygon
    
    } // Next Object

    // End Scene Rendering
    m_pD3DDevice->EndScene();
    
    // Present the buffer
    if ( FAILED(m_pD3DDevice->Present( NULL, NULL, NULL, NULL )) ) m_bLostDevice = true;

}

//-----------------------------------------------------------------------------
// Name : ProcessInput () (Private)
// Desc : Simply polls the input devices and performs basic input operations
//-----------------------------------------------------------------------------
void CGameApp::ProcessInput( )
{
    // Simple strafing
    if ( GetKeyState( VK_LEFT  ) & 0xFF00 ) m_mtxView._41 += 25.0f * m_Timer.GetTimeElapsed();
    if ( GetKeyState( VK_RIGHT ) & 0xFF00 ) m_mtxView._41 -= 25.0f * m_Timer.GetTimeElapsed();
        
    // Update the device matrix
    if (m_pD3DDevice) m_pD3DDevice->SetTransform( D3DTS_VIEW, &m_mtxView );

}

//-----------------------------------------------------------------------------
// Name : AnimateObjects () (Private)
// Desc : Animates the objects we currently have loaded.
//-----------------------------------------------------------------------------
void CGameApp::AnimateObjects()
{
    // Note : Expanded for the purposes of this example only.
    D3DXMATRIX mtxYaw, mtxPitch, mtxRoll, mtxRotate;
    float RotationYaw, RotationPitch, RotationRoll;

    // Rotate Object 1 by small amount
    if ( m_bRotation1 )
    {
        // Calculate rotation values for object 0
        RotationYaw   = D3DXToRadian( 75.0f * m_Timer.GetTimeElapsed() );
        RotationPitch = D3DXToRadian( 50.0f * m_Timer.GetTimeElapsed() );
        RotationRoll  = D3DXToRadian( 25.0f * m_Timer.GetTimeElapsed() );

        // Build rotation matrices 
        D3DXMatrixIdentity( &mtxRotate );
        D3DXMatrixRotationY( &mtxYaw, RotationYaw);
        D3DXMatrixRotationX( &mtxPitch,RotationPitch);
        D3DXMatrixRotationZ( &mtxRoll, RotationRoll);
        
        // Concatenate the rotation matrices
        D3DXMatrixMultiply( &mtxRotate, &mtxRotate, &mtxYaw );
        D3DXMatrixMultiply( &mtxRotate, &mtxRotate, &mtxPitch );
        D3DXMatrixMultiply( &mtxRotate, &mtxRotate, &mtxRoll );
            
        // Apply the rotation to our object's matrix
        D3DXMatrixMultiply( &m_pObject[ 0 ].m_mtxWorld, &mtxRotate, &m_pObject[ 0 ].m_mtxWorld );

    } // End if Rotation Enabled

    // Rotate Object 2 by small amount
    if ( m_bRotation2 )
    {
        // Calculate rotation values for object 1
        RotationYaw   = D3DXToRadian( -25.0f * m_Timer.GetTimeElapsed() );
        RotationPitch = D3DXToRadian(  50.0f * m_Timer.GetTimeElapsed() );
        RotationRoll  = D3DXToRadian( -75.0f * m_Timer.GetTimeElapsed() );

        // Build rotation matrices 
        D3DXMatrixIdentity( &mtxRotate );
        D3DXMatrixRotationY( &mtxYaw, RotationYaw);
        D3DXMatrixRotationX( &mtxPitch,RotationPitch);
        D3DXMatrixRotationZ( &mtxRoll, RotationRoll);
        
        // Concatenate the rotation matrices
        D3DXMatrixMultiply( &mtxRotate, &mtxRotate, &mtxYaw );
        D3DXMatrixMultiply( &mtxRotate, &mtxRotate, &mtxPitch );
        D3DXMatrixMultiply( &mtxRotate, &mtxRotate, &mtxRoll );
            
        // Apply the rotation to our object's matrix
        D3DXMatrixMultiply( &m_pObject[ 1 ].m_mtxWorld, &mtxRotate, &m_pObject[ 1 ].m_mtxWorld );

    } // End if rotation enabled

}
