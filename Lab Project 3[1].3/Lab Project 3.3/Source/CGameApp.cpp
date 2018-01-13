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
// Module Local Constants
//-----------------------------------------------------------------------------
namespace
{
    const USHORT BlockWidth  = 33;                  // Number of vertices in our patch block (X)
    const USHORT BlockHeight = 33;                  // Number of vertices in our patch block (Z)
    const USHORT QuadsWide   = BlockWidth - 1;      // Number of quads in our patch block (X)
    const USHORT QuadsHigh   = BlockHeight - 1;     // Number of quads in our patch block (Z)
};

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
    m_hWnd           = NULL;
    m_pD3D           = NULL;
    m_pD3DDevice     = NULL;
    m_hIcon          = NULL;
    m_hMenu          = NULL;
    m_bLostDevice    = false;
    m_pVertexBuffer  = NULL;
    m_pIndexBuffer   = NULL;
    m_nLastFrameRate = 0;

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
    LPTSTR          WindowTitle  = _T("Dynamic Vertex Buffers");
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

    // Set our cameras look at
    D3DXMatrixLookAtLH( &m_mtxView, &D3DXVECTOR3( -24.0f, 90.0f, -24.0f ), 
                        &D3DXVECTOR3( -23.0f, 88.5f, -23.0f ), &D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );
    
    // Enable animation
    m_bAnimation = true;

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

    // Set the stream sources
    m_pD3DDevice->SetStreamSource( 0, m_pVertexBuffer, 0, sizeof(CVertex) );
    m_pD3DDevice->SetIndices( m_pIndexBuffer );

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
                    // Reset the device (Must destroy / recreate un-managed objects)
                    ReleaseObjects();
                    Initialize.ResetDisplay( m_pD3DDevice, m_D3DSettings );
                    BuildObjects();
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

                        // Reset the device (Must destroy / recreate un-managed objects)
                        ReleaseObjects();
                        Initialize.ResetDisplay( m_pD3DDevice, m_D3DSettings, m_hWnd );
                        BuildObjects();
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
                case ID_ANIM_ENABLE:
                    // Disable / enable animation
                    m_bAnimation = !m_bAnimation;
                    ::CheckMenuItem( ::GetMenu( m_hWnd ), ID_ANIM_ENABLE, 
                                     MF_BYCOMMAND | (m_bAnimation) ? MF_CHECKED :  MF_UNCHECKED );
                    break;

                case ID_FILE_CHANGEDEVICE:
                    // Signal that we want to change devices
                    ChangeDevice();
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
// Name : ChangeDevice ()
// Desc : The user requested a change of device. This function displays the
//        dialog, and also reinitializes any device as required.
//-----------------------------------------------------------------------------
void CGameApp::ChangeDevice()
{
    CMyD3DInit      Initialize;
    CD3DSettingsDlg SettingsDlg;

    // Enumerate the system graphics adapters    
    if ( FAILED(Initialize.Enumerate( m_pD3D ) ))
    {
        MessageBox( m_hWnd, _T("Device enumeration failed. The application will now exit."), _T("Fatal Error!"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        PostQuitMessage( 0 );
        return;
    } // End if Failure

    // Display the settings dialog
    int RetCode = SettingsDlg.ShowDialog( &Initialize, &m_D3DSettings );
    if ( RetCode != IDOK ) return;
    m_D3DSettings = SettingsDlg.GetD3DSettings();

    // Lets Destroy our objects and restart
    if ( m_pD3DDevice ) m_pD3DDevice->Release();
    m_pD3DDevice = NULL;

    // Create the direct 3d device etc.
    if ( FAILED (Initialize.CreateDisplay( m_D3DSettings, 0, m_hWnd )) )
    {
        MessageBox( m_hWnd, _T("Device creation failed. The application will now exit."), _T("Fatal Error!"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        PostQuitMessage( 0 );
        return;

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
}

//-----------------------------------------------------------------------------
// Name : BuildObjects ()
// Desc : Build our demonstration cube mesh, and the objects that instance it
//-----------------------------------------------------------------------------
bool CGameApp::BuildObjects()
{
    HRESULT  hRet;
    CVertex *pVertex = NULL;
    USHORT  *pIndex  = NULL;
    ULONG    ulUsage = D3DUSAGE_WRITEONLY;
    long     vx, vz;

    // Seed the random number generator
    srand( timeGetTime() );

    // Release previously built objects
    ReleaseObjects();

    // Build our buffers usage flags (i.e. Software T&L etc)
    VERTEXPROCESSING_TYPE vp = m_D3DSettings.GetSettings()->VertexProcessingType;
    if ( vp != HARDWARE_VP && vp != PURE_HARDWARE_VP ) ulUsage |= D3DUSAGE_SOFTWAREPROCESSING;

    // Create our vertex buffer 
    hRet = m_pD3DDevice->CreateVertexBuffer( sizeof(CVertex) * (BlockWidth * BlockHeight), D3DUSAGE_DYNAMIC | ulUsage, 
                                             D3DFVF_XYZ | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &m_pVertexBuffer, NULL );
    if ( FAILED( hRet ) ) return false;

    // Calculate IndexCount.... ( Number required for quads ) + ( Extra Degenerates (one per quad row except last) )
    m_nIndexCount = ((BlockWidth * 2) * QuadsHigh) + ( QuadsHigh - 1 );

    // Create our index buffer 
    hRet = m_pD3DDevice->CreateIndexBuffer( sizeof(USHORT) * m_nIndexCount, ulUsage, D3DFMT_INDEX16,
                                             D3DPOOL_DEFAULT, &m_pIndexBuffer, NULL );
    if ( FAILED( hRet ) ) return false;

    // Lock the index buffer (we only need to build this once in this example)
    hRet = m_pIndexBuffer->Lock( 0, sizeof(USHORT) * m_nIndexCount, (void**)&pIndex, 0 );
    if ( FAILED( hRet ) ) return false;

    // Calculate the indices for the patch block tri-strip 
    for ( vz = 0; vz < BlockHeight - 1; vz++ )
    {
        // Is this an odd or even row ?
        if ( (vz % 2) == 0 )
        {
            for ( vx = 0; vx < BlockWidth; vx++ )
            {
                // Force insert winding order switch degenerate ?
                if ( vx == 0 && vz > 0 ) *pIndex++ = (USHORT)(vx + vz * BlockWidth);

                // Insert next two indices
                *pIndex++ = (USHORT)(vx + vz * BlockWidth);
                *pIndex++ = (USHORT)((vx + vz * BlockWidth) + BlockWidth);

            } // Next Index Column
            
        } // End if even row
        else
        {
            for ( vx = BlockWidth - 1; vx >= 0; vx--)
            {
                // Force insert winding order switch degenerate ?
                if ( vx == (BlockWidth - 1) ) *pIndex++ = (USHORT)(vx + vz * BlockWidth);

                // Insert next two indices
                *pIndex++ = (USHORT)(vx + vz * BlockWidth);
                *pIndex++ = (USHORT)((vx + vz * BlockWidth) + BlockWidth);

            } // Next Index Column

        } // End if odd row
    
    } // Next Index Row

    // Unlock the index buffer
    if ( FAILED(m_pIndexBuffer->Unlock()) ) return false;
    
    // Force a rebuild of the vertex data
    AnimateObjects();
    
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
    // De-reference streams (just in case ours is set)
    if ( m_pD3DDevice ) 
    {
        m_pD3DDevice->SetStreamSource( 0, NULL, 0, 0 );
        m_pD3DDevice->SetIndices( NULL );

    } // End if D3DDevice

    // Release vertex buffer
    if ( m_pVertexBuffer ) m_pVertexBuffer->Release();
    if ( m_pIndexBuffer  ) m_pIndexBuffer->Release();
    m_pVertexBuffer = NULL;
    m_pIndexBuffer  = NULL;

}

//-----------------------------------------------------------------------------
// Name : FrameAdvance () (Private)
// Desc : Called to signal that we are now rendering the next frame.
//-----------------------------------------------------------------------------
void CGameApp::FrameAdvance()
{
    static TCHAR FrameRate[ 50 ];
    static TCHAR TitleBuffer[ 255 ];

    // Advance the timer
    m_Timer.Tick( );

    // Skip if app is inactive
    if ( !m_bActive ) return;
    
    // Get / Display the framerate
    if ( m_nLastFrameRate != m_Timer.GetFrameRate() )
    {
        m_nLastFrameRate = m_Timer.GetFrameRate( FrameRate );
        _stprintf( TitleBuffer, _T("Dynamic Vertex Buffers : %s"), FrameRate );
        SetWindowText( m_hWnd, TitleBuffer );

    } // End if Frame Rate Altered

    // Recover lost device if required
    if ( m_bLostDevice )
    {
        // Can we reset the device yet ?
        HRESULT hRet = m_pD3DDevice->TestCooperativeLevel();
        if ( hRet == D3DERR_DEVICENOTRESET )
        {
            CMyD3DInit Initialize;

            // Reset the device (Must destroy / recreate un-managed objects)
            ReleaseObjects();
            Initialize.ResetDisplay( m_pD3DDevice, m_D3DSettings, m_hWnd );
            BuildObjects();
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

    // Animate the meshes 
    if ( m_bAnimation ) AnimateObjects();

    // Clear the frame & depth buffer ready for drawing
    m_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0 );
    
    // Begin Scene Rendering
    m_pD3DDevice->BeginScene();

    // Render the primitive (Hardcoded, 12 primitives)
    m_pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, (BlockWidth * BlockHeight), 0, m_nIndexCount - 2 );
    
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
    // No input in this particular example
}

//-----------------------------------------------------------------------------
// Name : AnimateObjects () (Private)
// Desc : Animates the objects we currently have loaded.
//-----------------------------------------------------------------------------
void CGameApp::AnimateObjects()
{
    static float Distance = 6.28f; // 6.28 = ~(2 * PI) = 360 degrees in radians.

    ULONG       x, z;
    HRESULT     hRet;
    float       fHeight, fRed, fx, fz, fDist;
    CVertex    *pVertex  = NULL;
    D3DXVECTOR3 vecScale = D3DXVECTOR3( 4.0f, 6.0f, 4.0f );
    
    // Validate parameters
    if ( !m_pVertexBuffer ) return;
    
    // Work out time shift
    Distance -= 5.0f * m_Timer.GetTimeElapsed();
    if (Distance < 0.0f) Distance += 6.28f;

    // Lock the vertex buffer
    hRet = m_pVertexBuffer->Lock( 0, sizeof(CVertex) * (BlockWidth * BlockHeight), (void**)&pVertex, D3DLOCK_DISCARD );
    if ( FAILED(hRet) ) return;

    // Loop through each row
    for ( z = 0; z < BlockHeight; z++ )
    {
        // Loop through each column
        for ( x = 0; x < BlockWidth; x++ )
        {
            // Calculate height of the vertex
            fx = ((BlockWidth / 2.0f) - x);
            fz = ((BlockHeight / 2.0f) - z);
            fDist = sqrtf(fx * fx + fz * fz) + Distance;
            fHeight = sinf(fDist) * (vecScale.y / 2.0f);

            // Calculate the color of the vertex
            fRed = (fHeight + ( vecScale.y / 2.0f )) + 4.0f;
            fRed = fRed / (vecScale.y + 4.0f); // Normalize the colour value
            
            *pVertex++ = CVertex( x * vecScale.x, fHeight, z * vecScale.z, D3DCOLOR_COLORVALUE( fRed, 0.0f, 0.0f, 1.0f ) );

        } // Next Column

    } // Next Row

    m_pVertexBuffer->Unlock( );

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