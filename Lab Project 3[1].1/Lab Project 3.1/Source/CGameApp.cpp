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
    m_hIcon         = NULL;
    m_hMenu         = NULL;
    m_bLostDevice   = false;
    m_pVertexBuffer = NULL;

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
    D3DDISPLAYMODE  MatchMode;
    CD3DSettingsDlg SettingsDlg;
    CMyD3DInit      Initialize;
    LPTSTR          WindowTitle  = _T("Vertex Buffers");
    USHORT          Width        = 400;
    USHORT          Height       = 400;
    RECT            rc;

    // First of all create our D3D Object (This is needed by the enumeration etc)
    m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
    if (!m_pD3D) 
    {
        MessageBox( m_hWnd, _T("No compatible Direct3D object could be created."), _T("Fatal Error!"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        return false;
    
    } // End if failure

    // Enumerate the system graphics adapters    
    if ( FAILED(Initialize.Enumerate( m_pD3D ) ))
    {
        MessageBox( m_hWnd, _T("Device enumeration failed. The application will now exit."), _T("Fatal Error!"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        return false;

    } // End if Failure

    // Attempt to find a good default fullscreen set
    MatchMode.Width       = 640;
    MatchMode.Height      = 480;
    MatchMode.Format      = D3DFMT_UNKNOWN;
    MatchMode.RefreshRate = 0;
    Initialize.FindBestFullscreenMode( m_D3DSettings, &MatchMode );
    
    // Attempt to find a good default windowed set
    Initialize.FindBestWindowedMode( m_D3DSettings );

    // Create the direct 3d device etc.
    if ( FAILED( Initialize.CreateDisplay( m_D3DSettings, 0, NULL, StaticWndProc, WindowTitle, Width, Height, this ) ))
    {
        MessageBox( m_hWnd, _T("Device creation failed. The application will now exit."), _T("Fatal Error!"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        return false;
    
    } // End if Failed
    
    // Retrieve created items
    m_pD3DDevice = Initialize.GetDirect3DDevice( );
    m_hWnd       = Initialize.GetHWND( );

    // Load icon and menu
    m_hIcon = LoadIcon( (HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE( IDI_ICON ) );
    m_hMenu = LoadMenu( (HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE( IDR_MENU ) );

    // Set application icon
    SetClassLong( m_hWnd, GCL_HICON, (long)m_hIcon );

    // Set menu only in windowed mode
    if ( m_D3DSettings.Windowed )
    {
        SetMenu( m_hWnd, m_hMenu );
    
    } // End if Windowed

    // Retrieve the final client size of the window
    ::GetClientRect( m_hWnd, &rc );
    m_nViewX      = rc.left;
    m_nViewY      = rc.top;
    m_nViewWidth  = rc.right - rc.left;
    m_nViewHeight = rc.bottom - rc.top;

    // Show the window
	ShowWindow(m_hWnd, SW_SHOW);

    // Success!!
    return true;
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
    // Store new viewport sizes
    RECT rc;
    ::GetClientRect( m_hWnd, &rc );
    m_nViewWidth  = rc.right - rc.left;
    m_nViewHeight = rc.bottom - rc.top;

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
    // Release any previously built objects
    ReleaseObjects ( );

    // Destroy Direct3D Objects
    if ( m_pD3DDevice ) m_pD3DDevice->Release();
    if ( m_pD3D       ) m_pD3D->Release();
    m_pD3D          = NULL;
    m_pD3DDevice    = NULL;
    
    // Destroy menu, it may not be attached
    if ( m_hMenu ) DestroyMenu( m_hMenu );
    m_hMenu         = NULL;

    // Destroy the render window
    SetMenu( m_hWnd, NULL );
    if ( m_hWnd ) DestroyWindow( m_hWnd );
    m_hWnd          = NULL;
    
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
    CMyD3DInit  Initialize;
	
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
                    Initialize.ResetDisplay( m_pD3DDevice, m_D3DSettings );
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

                case VK_RETURN:
                    if ( GetKeyState( VK_SHIFT ) & 0xFF00 )
                    {
                        // Toggle fullscreen / windowed
                        m_D3DSettings.Windowed = !m_D3DSettings.Windowed;
                        Initialize.ResetDisplay( m_pD3DDevice, m_D3DSettings, m_hWnd );
                        SetupRenderStates( );

                        // Set menu only in windowed mode
                        // (Removed by ResetDisplay automatically in fullscreen)
                        if ( m_D3DSettings.Windowed )
                        {
                            SetMenu( m_hWnd, m_hMenu );
                        } // End if Windowed

                    } // End if
                    break;
                    
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

                case ID_FILE_CHANGEDEVICE:
                {
                    CMyD3DInit      Initialize;
                    CD3DSettingsDlg SettingsDlg;

                    // Enumerate the system graphics adapters    
                    if ( FAILED(Initialize.Enumerate( m_pD3D ) ))
                    {
                        MessageBox( m_hWnd, _T("Device enumeration failed. The application will now exit."), _T("Fatal Error!"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
                        PostQuitMessage( 0 );
                        return 0;
                    } // End if Failure

                    // Display the settings dialog
                    int RetCode = SettingsDlg.ShowDialog( &Initialize, &m_D3DSettings );
                    if ( RetCode != IDOK ) break;
                    m_D3DSettings = SettingsDlg.GetD3DSettings();

                    // Lets Destroy our objects and restart
                    if ( m_pD3DDevice ) m_pD3DDevice->Release();
                    m_pD3DDevice = NULL;

                    // Create the direct 3d device etc.
                    if ( FAILED (Initialize.CreateDisplay( m_D3DSettings, 0, m_hWnd )) )
                    {
                        MessageBox( m_hWnd, _T("Device creation failed. The application will now exit."), _T("Fatal Error!"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
                        PostQuitMessage( 0 );
                        return 0;
                    
                    } // End if failure

                    // Retrieve our new device
                    m_pD3DDevice = Initialize.GetDirect3DDevice( );

                    // Set menu only in windowed mode
                    // (Removed by CreateDisplay automatically in fullscreen)
                    if ( m_D3DSettings.Windowed )
                    {
                        SetMenu( m_hWnd, m_hMenu );
                    } // End if Windowed

                    // Build our game objects
                    BuildObjects();

                    // Setup our render states
                    SetupRenderStates( );

                    break;

                }
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
    HRESULT  hRet;
    CVertex *pVertex = NULL;
    ULONG    ulUsage = D3DUSAGE_WRITEONLY;

    // Seed the random number generator
    srand( timeGetTime() );

    // Release previously built objects
    ReleaseObjects();

    // Build our buffers usage flags (i.e. Software T&L etc)
    VERTEXPROCESSING_TYPE vp = m_D3DSettings.GetSettings()->VertexProcessingType;
    if ( vp != HARDWARE_VP && vp != PURE_HARDWARE_VP ) ulUsage |= D3DUSAGE_SOFTWAREPROCESSING;

    // Create our vertex buffer ( 36 vertices (6 verts * 6 faces) )
    hRet = m_pD3DDevice->CreateVertexBuffer( sizeof(CVertex) * 36, ulUsage, D3DFVF_XYZ | D3DFVF_DIFFUSE,
                                             D3DPOOL_MANAGED, &m_pVertexBuffer, NULL );
    if ( FAILED( hRet ) ) return false;

    // Lock the vertex buffer ready to fill data
    hRet = m_pVertexBuffer->Lock( 0, sizeof(CVertex) * 36, (void**)&pVertex, 0 );
    if ( FAILED( hRet ) ) return false;
    
    // Front Face
    *pVertex++ = CVertex( -2,  2, -2, RANDOM_COLOR );
    *pVertex++ = CVertex(  2,  2, -2, RANDOM_COLOR );
    *pVertex++ = CVertex(  2, -2, -2, RANDOM_COLOR );
    
    *pVertex++ = CVertex( -2,  2, -2, RANDOM_COLOR );
    *pVertex++ = CVertex(  2, -2, -2, RANDOM_COLOR );
    *pVertex++ = CVertex( -2, -2, -2, RANDOM_COLOR );
    
    // Top Face
    *pVertex++ = CVertex( -2,  2,  2, RANDOM_COLOR );
    *pVertex++ = CVertex(  2,  2,  2, RANDOM_COLOR );
    *pVertex++ = CVertex(  2,  2, -2, RANDOM_COLOR );
    
    *pVertex++ = CVertex( -2,  2,  2, RANDOM_COLOR );
    *pVertex++ = CVertex(  2,  2, -2, RANDOM_COLOR );
    *pVertex++ = CVertex( -2,  2, -2, RANDOM_COLOR );

    // Back Face
    *pVertex++ = CVertex( -2, -2,  2, RANDOM_COLOR );
    *pVertex++ = CVertex(  2, -2,  2, RANDOM_COLOR );
    *pVertex++ = CVertex(  2,  2,  2, RANDOM_COLOR );
    
    *pVertex++ = CVertex( -2, -2,  2, RANDOM_COLOR );
    *pVertex++ = CVertex(  2,  2,  2, RANDOM_COLOR );
    *pVertex++ = CVertex( -2,  2,  2, RANDOM_COLOR );

    // Bottom Face
    *pVertex++ = CVertex( -2, -2, -2, RANDOM_COLOR );
    *pVertex++ = CVertex(  2, -2, -2, RANDOM_COLOR );
    *pVertex++ = CVertex(  2, -2,  2, RANDOM_COLOR );
    
    *pVertex++ = CVertex( -2, -2, -2, RANDOM_COLOR );
    *pVertex++ = CVertex(  2, -2,  2, RANDOM_COLOR );
    *pVertex++ = CVertex( -2, -2,  2, RANDOM_COLOR );

    // Left Face
    *pVertex++ = CVertex( -2,  2,  2, RANDOM_COLOR );
    *pVertex++ = CVertex( -2,  2, -2, RANDOM_COLOR );
    *pVertex++ = CVertex( -2, -2, -2, RANDOM_COLOR );
    
    *pVertex++ = CVertex( -2,  2,  2, RANDOM_COLOR );
    *pVertex++ = CVertex( -2, -2, -2, RANDOM_COLOR );
    *pVertex++ = CVertex( -2, -2,  2, RANDOM_COLOR );

    // Right Face
    *pVertex++ = CVertex(  2,  2, -2, RANDOM_COLOR );
    *pVertex++ = CVertex(  2,  2,  2, RANDOM_COLOR ); 
    *pVertex++ = CVertex(  2, -2,  2, RANDOM_COLOR );
    
    *pVertex++ = CVertex(  2,  2, -2, RANDOM_COLOR );
    *pVertex++ = CVertex(  2, -2,  2, RANDOM_COLOR );
    *pVertex++ = CVertex(  2, -2, -2, RANDOM_COLOR );

    // Unlock the buffer
    m_pVertexBuffer->Unlock( );

    // Our two objects should reference this vertex buffer
    m_pObject[ 0 ].SetVertexBuffer( m_pVertexBuffer );
    m_pObject[ 1 ].SetVertexBuffer( m_pVertexBuffer );

    // Set both objects matrices so that they are offset slightly
    D3DXMatrixTranslation( &m_pObject[ 0 ].m_mtxWorld, -3.5f,  2.0f, 14.0f );
    D3DXMatrixTranslation( &m_pObject[ 1 ].m_mtxWorld,  3.5f, -2.0f, 14.0f );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : ReleaseObjects ()
// Desc : Releases our objects and their associated memory so that we can
//        rebuild them, if required, during our applications life-time.
//-----------------------------------------------------------------------------
void CGameApp::ReleaseObjects( )
{
    // De-reference vertex stream (just in case ours is set)
    if ( m_pD3DDevice ) m_pD3DDevice->SetStreamSource( 0, NULL, 0, 0 );

    // Release Objects vertex buffer
    for ( ULONG i = 0; i < 2; i++ ) m_pObject[i].SetVertexBuffer( NULL );
    
    // Release vertex buffer
    if ( m_pVertexBuffer ) m_pVertexBuffer->Release();
    m_pVertexBuffer = NULL;

}

//-----------------------------------------------------------------------------
// Name : FrameAdvance () (Private)
// Desc : Called to signal that we are now rendering the next frame.
//-----------------------------------------------------------------------------
void CGameApp::FrameAdvance()
{
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
            CMyD3DInit Initialize;
            Initialize.ResetDisplay( m_pD3DDevice, m_D3DSettings, m_hWnd );
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
        // Set our object matrix
        m_pD3DDevice->SetTransform( D3DTS_WORLD, &m_pObject[i].m_mtxWorld );

        // Set the vertex stream source
        m_pD3DDevice->SetStreamSource( 0, m_pObject[i].m_pVertexBuffer, 0, sizeof(CVertex) );

        // Render the primitive (Hardcoded, 12 primitives)
        m_pD3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 12 );
    
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
    D3DXMATRIX mtxRotate;
    float      RotationYaw, RotationPitch, RotationRoll;

    // Rotate Object 1 by small amount
    if ( m_bRotation1 )
    {
        // Calculate rotation values for object 0
        RotationYaw   = D3DXToRadian( 75.0f * m_Timer.GetTimeElapsed() );
        RotationPitch = D3DXToRadian( 50.0f * m_Timer.GetTimeElapsed() );
        RotationRoll  = D3DXToRadian( 25.0f * m_Timer.GetTimeElapsed() );

        // Build rotation matrix
        D3DXMatrixRotationYawPitchRoll( &mtxRotate, RotationYaw, RotationPitch, RotationRoll );
        
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

        // Build rotation matrix
        D3DXMatrixRotationYawPitchRoll( &mtxRotate, RotationYaw, RotationPitch, RotationRoll );
        
        // Apply the rotation to our object's matrix
        D3DXMatrixMultiply( &m_pObject[ 1 ].m_mtxWorld, &mtxRotate, &m_pObject[ 1 ].m_mtxWorld );

    } // End if rotation enabled

}

//-----------------------------------------------------------------------------
// CMyD3DInit Member Functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : ValidateDisplayMode () (Private)
// Desc : Allows us to validate and reject any adapter display modes.
//-----------------------------------------------------------------------------
bool CMyD3DInit::ValidateDisplayMode( const D3DDISPLAYMODE &Mode )
{
    // Test display mode
    if ( Mode.Width < 640 || Mode.Height < 480 || Mode.RefreshRate < 60 ) return false;
    
    // Supported
    return true;
}

//-----------------------------------------------------------------------------
// Name : ValidateDevice () (Private)
// Desc : Allows us to validate and reject any devices that do not have
//        certain capabilities, or does not allow hardware rendering etc.
//-----------------------------------------------------------------------------
bool CMyD3DInit::ValidateDevice( const D3DDEVTYPE &Type, const D3DCAPS9 &Caps )
{
    // Test Capabilities (All device types supported)
    if ( !(Caps.RasterCaps & D3DPRASTERCAPS_DITHER       ) ) return false;
    if ( !(Caps.ShadeCaps & D3DPSHADECAPS_COLORGOURAUDRGB) ) return false;
    if ( !(Caps.PrimitiveMiscCaps & D3DPMISCCAPS_CULLCCW ) ) return false;
    if ( !(Caps.ZCmpCaps & D3DPCMPCAPS_LESSEQUAL         ) ) return false;

    // Supported
    return true;
}

//-----------------------------------------------------------------------------
// Name : ValidateVertexProcessingType () (Private)
// Desc : Allows us to validate and reject any vertex processing types we
//        do not wish to allow access to.
//-----------------------------------------------------------------------------
bool CMyD3DInit::ValidateVertexProcessingType( const VERTEXPROCESSING_TYPE &Type )
{
    // Test Type ( We don't need mixed  )
    if ( Type == MIXED_VP ) return false;

    // Supported
    return true;
}