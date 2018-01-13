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
#include "..\\Includes\\CCamera.h"

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
    m_LastFrameRate = 0;
    m_FillMode      = D3DFILL_SOLID;
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
    LPTSTR          WindowTitle  = _T("Simple Terrain");
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

    // Setup default menu states
    ::CheckMenuRadioItem( m_hMenu, ID_CAMERAMODE_FPS, ID_CAMERAMODE_THIRDPERSON_SC, ID_CAMERAMODE_FPS, MF_BYCOMMAND );
    ::CheckMenuRadioItem( m_hMenu, ID_FILLMODE_SOLID, ID_FILLMODE_WIREFRAME, ID_FILLMODE_SOLID, MF_BYCOMMAND );

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
    // Generate an identity matrix
    D3DXMatrixIdentity( &m_mtxIdentity );

    // App is active
    m_bActive = true;

    // Setup the players camera, and extract the pointer.
    // This pointer will only ever become invalid on subsequent
    // calls to CPlayer::SetCameraMode and on player destruction.
    m_Player.SetCameraMode( CCamera::MODE_FPS );
    m_pCamera = m_Player.GetCamera();

    // Setup our player's default details
    m_Player.SetFriction( 250.0f ); // Per Second
    m_Player.SetGravity( D3DXVECTOR3( 0, -400.0f, 0 ) );
    m_Player.SetMaxVelocityXZ( 125.0f );
    m_Player.SetMaxVelocityY ( 400.0f );
    m_Player.SetCamOffset( D3DXVECTOR3( 0.0f, 10.0f, 0.0f ) );
    m_Player.SetCamLag( 0.0f );
    
    // Set up the players collision volume info
    VOLUME_INFO Volume;
    Volume.Min = D3DXVECTOR3( -3, -10, -3 );
    Volume.Max = D3DXVECTOR3(  3,  10,  3 );
    m_Player.SetVolumeInfo( Volume );

    // Setup our cameras view details
    m_pCamera->SetFOV( 80.0f );
    m_pCamera->SetViewport( m_nViewX, m_nViewY, m_nViewWidth, m_nViewHeight, 1.01f, 5000.0f );

    // Set the camera volume info (matches player volume)
    m_pCamera->SetVolumeInfo( Volume );

    // Add the update callbacks required
    m_Player.AddPlayerCallback( CTerrain::UpdatePlayer, (LPVOID)&m_Terrain );
    m_Player.AddCameraCallback( CTerrain::UpdateCamera, (LPVOID)&m_Terrain );
    
    // Lets give a small initial rotation and set initial position
    m_Player.SetPosition( D3DXVECTOR3( 430.0f, 400.0f, 330.0f ) );
    //m_Player.Rotate( 25, 45, 0 );

     // Build base white material
    ZeroMemory( &m_BaseMaterial, sizeof(D3DMATERIAL9));
    m_BaseMaterial.Diffuse.a = 1.0f; m_BaseMaterial.Diffuse.r = 1.0f; m_BaseMaterial.Diffuse.g = 0.8f; m_BaseMaterial.Diffuse.b = 0.6f;
    m_BaseMaterial.Ambient.a = 1.0f; m_BaseMaterial.Ambient.r = 1.0f; m_BaseMaterial.Ambient.g = 1.0f; m_BaseMaterial.Ambient.b = 1.0f;

    // Setup initial light states
    ZeroMemory( &m_Light, 5 * sizeof(D3DLIGHT9) );
    
    // Main static directional light
    m_Light[0].Type      = D3DLIGHT_DIRECTIONAL;
    m_Light[0].Direction = D3DXVECTOR3( 0.650945f, -0.390567f, 0.650945f );
    m_Light[0].Diffuse.a = 1.0f;
    m_Light[0].Diffuse.r = 1.0f;
    m_Light[0].Diffuse.g = 1.0f;
    m_Light[0].Diffuse.b = 1.0f;
    m_LightEnabled[0]    = true;

    // Players following light
    m_Light[1].Type         = D3DLIGHT_POINT;
    m_Light[1].Position     = m_Player.GetPosition();
    m_Light[1].Range        = 70.0f;
    m_Light[1].Attenuation1 = 0.02f;
    m_Light[1].Attenuation2 = 0.002f;
    m_Light[1].Diffuse.a    = 1.0f;
    m_Light[1].Diffuse.r    = 1.0f; 
    m_Light[1].Diffuse.g    = 0.0f;
    m_Light[1].Diffuse.b    = 0.0f;
    m_LightEnabled[1]       = true;

    // Dynamic floating light 1
    m_Light[2].Type         = D3DLIGHT_POINT;
    m_Light[2].Position     = D3DXVECTOR3( 500, 0, 500 );
    m_Light[2].Position.y   = m_Terrain.GetHeight( m_Light[2].Position.x, m_Light[2].Position.z ) + 30.0f;
    m_Light[2].Range        = 500.0f;
    m_Light[2].Attenuation1 = 0.0002f;
    m_Light[2].Attenuation2 = 0.0001f;
    m_Light[2].Diffuse.a    = 1.0f;
    m_Light[2].Diffuse.r    = 0.0f; 
    m_Light[2].Diffuse.g    = 1.0f;
    m_Light[2].Diffuse.b    = 0.0f;
    m_LightEnabled[2]       = true;

    // Dynamic floating light 2
    m_Light[3].Type         = D3DLIGHT_POINT;
    m_Light[3].Position     = D3DXVECTOR3( 1000, 0, 1000 );
    m_Light[3].Position.y   = m_Terrain.GetHeight( m_Light[3].Position.x, m_Light[3].Position.z ) + 30.0f;
    m_Light[3].Range        = 500.0f;
    m_Light[3].Attenuation1 = 0.000002f;
    m_Light[3].Attenuation2 = 0.00002f;
    m_Light[3].Diffuse.a    = 1.0f;
    m_Light[3].Diffuse.r    = 0.0f; 
    m_Light[3].Diffuse.g    = 0.0f;
    m_Light[3].Diffuse.b    = 1.0f;
    m_LightEnabled[3]       = true;

    // Dynamic floating light 3
    m_Light[4].Type         = D3DLIGHT_POINT;
    m_Light[4].Position     = D3DXVECTOR3( 1500, 0, 1500 );
    m_Light[4].Position.y   = m_Terrain.GetHeight( m_Light[4].Position.x, m_Light[4].Position.z ) + 30.0f;
    m_Light[4].Range        = 500.0f;
    m_Light[4].Attenuation1 = 0.00002f;
    m_Light[4].Attenuation2 = 0.00002f;
    m_Light[4].Diffuse.a    = 1.0f;
    m_Light[4].Diffuse.r    = 1.0f; 
    m_Light[4].Diffuse.g    = 1.0f;
    m_Light[4].Diffuse.b    = 0.5f;
    m_LightEnabled[4]       = true;

}

//-----------------------------------------------------------------------------
// Name : SetupRenderStates ()
// Desc : Sets up all the initial states required by the renderer.
//-----------------------------------------------------------------------------
void CGameApp::SetupRenderStates()
{
    // Validate Requirements
    if (!m_pD3DDevice || !m_pCamera ) return;

    // Setup our D3D Device initial states
    m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
    m_pD3DDevice->SetRenderState( D3DRS_DITHERENABLE,  TRUE );
    m_pD3DDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
    m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
    m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    m_pD3DDevice->SetRenderState( D3DRS_AMBIENT, 0x0D0D0D );

    // Setup option dependant states
    m_pD3DDevice->SetRenderState( D3DRS_FILLMODE, m_FillMode );                

    // Setup our vertex FVF code
    m_pD3DDevice->SetFVF( D3DFVF_XYZ | D3DFVF_NORMAL );

    // Set base material
    m_pD3DDevice->SetMaterial( &m_BaseMaterial );

    // Set and enable all lights
    for ( ULONG i = 0; i < 5; i++ )
    {
    
        m_pD3DDevice->SetLight( i, &m_Light[i] );
        m_pD3DDevice->LightEnable( i, m_LightEnabled[i] );

    } // Next Light

    // Store new viewport sizes
    RECT rc;
    ::GetClientRect( m_hWnd, &rc );
    m_nViewWidth  = rc.right - rc.left;
    m_nViewHeight = rc.bottom - rc.top;

    // Update our device with our camera details (Required on reset)
    if ( !m_pCamera ) return;
    m_pCamera->SetViewport( m_nViewX, m_nViewY, m_nViewWidth, m_nViewHeight, 1.01f, 5000.0f );
    m_pCamera->UpdateRenderView( m_pD3DDevice );
    m_pCamera->UpdateRenderProj( m_pD3DDevice );
    
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
                    if ( m_pCamera ) m_pCamera->SetViewport( m_nViewX, m_nViewY, m_nViewWidth, m_nViewHeight, 1.01f, 5000.0f );
                    Initialize.ResetDisplay( m_pD3DDevice, m_D3DSettings );
                    SetupRenderStates( );
                
                } // End if
            
            } // End if !Minimized

			break;

        case WM_LBUTTONDOWN:
            // Capture the mouse
            SetCapture( m_hWnd );
            GetCursorPos( &m_OldCursorPos );
            break;


        case WM_LBUTTONUP:
            // Release the mouse
            ReleaseCapture( );
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
                case ID_FILE_CHANGEDEVICE:
                    // Signal that we want to change devices
                    ChangeDevice();
                    break;
                
                case ID_EXIT:
                    // Recieved key/menu command to exit app
                    SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
                    return 0;

                case ID_CAMERAMODE_FPS:
                    // Set camera mode to FPS style
                    ::CheckMenuRadioItem( m_hMenu, ID_CAMERAMODE_FPS, ID_CAMERAMODE_THIRDPERSON_SC,
                                          ID_CAMERAMODE_FPS, MF_BYCOMMAND );

                    // Setup player details
                    m_Player.SetFriction      ( 250.0f ); // Per Second
                    m_Player.SetGravity       ( D3DXVECTOR3( 0, -400.0f, 0 ) );
                    m_Player.SetMaxVelocityXZ ( 125.0f );
                    m_Player.SetMaxVelocityY  ( 400.0f );
                    m_Player.SetCamOffset     ( D3DXVECTOR3( 0.0f, 10.0f, 0.0f ) );
                    m_Player.SetCamLag        ( 0.0f ); // No camera lag

                    // Switch camera mode
                    m_Player.SetCameraMode    ( CCamera::MODE_FPS );
                    m_pCamera = m_Player.GetCamera();

                    break;

                case ID_CAMERAMODE_SPACECRAFT:
                    // Set camera mode to SPACECRAFT style
                    ::CheckMenuRadioItem( m_hMenu, ID_CAMERAMODE_FPS, ID_CAMERAMODE_THIRDPERSON_SC,
                                          ID_CAMERAMODE_SPACECRAFT, MF_BYCOMMAND );

                    // Setup player details
                    m_Player.SetFriction      ( 125.0f ); // Per Second
                    m_Player.SetGravity       ( D3DXVECTOR3( 0, 0, 0 ) );
                    m_Player.SetMaxVelocityXZ ( 400.0f );
                    m_Player.SetMaxVelocityY  ( 400.0f );
                    m_Player.SetCamOffset     ( D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) );
                    m_Player.SetCamLag        ( 0.0f ); // No camera lag

                    // Switch camera mode
                    m_Player.SetCameraMode( CCamera::MODE_SPACECRAFT );
                    m_pCamera = m_Player.GetCamera();

                    break;

                case ID_CAMERAMODE_THIRDPERSON:
                    // Set camera mode to third person style
                    ::CheckMenuRadioItem( m_hMenu, ID_CAMERAMODE_FPS, ID_CAMERAMODE_THIRDPERSON_SC,
                                          ID_CAMERAMODE_THIRDPERSON, MF_BYCOMMAND );

                    // Setup Player details
                    m_Player.SetFriction      ( 250.0f ); // Per Second
                    m_Player.SetGravity       ( D3DXVECTOR3( 0, -400.0f, 0 ) );
                    m_Player.SetMaxVelocityXZ ( 125.0f );
                    m_Player.SetMaxVelocityY  ( 400.0f );
                    m_Player.SetCamOffset     ( D3DXVECTOR3( 0.0f, 40.0f, -60.0f ) );
                    m_Player.SetCamLag        ( 0.25f ); // 1/4 second camera lag
                    
                    // Switch camera mode
                    m_Player.SetCameraMode    ( CCamera::MODE_THIRDPERSON );
                    m_pCamera = m_Player.GetCamera();

                    break;

                case ID_CAMERAMODE_THIRDPERSON_SC:
                    // Set camera mode to third person space craft style
                    ::CheckMenuRadioItem( m_hMenu, ID_CAMERAMODE_FPS, ID_CAMERAMODE_THIRDPERSON_SC,
                                          ID_CAMERAMODE_THIRDPERSON_SC, MF_BYCOMMAND );

                    // Setup Player details
                    m_Player.SetFriction      ( 125.0f ); // Per Second
                    m_Player.SetGravity       ( D3DXVECTOR3( 0, 0, 0 ) );
                    m_Player.SetMaxVelocityXZ ( 400.0f );
                    m_Player.SetMaxVelocityY  ( 400.0f );
                    m_Player.SetCamOffset     ( D3DXVECTOR3( 0.0f, 40.0f, -60.0f ) );
                    m_Player.SetCamLag        ( 0.08f ); // 1/12 second camera lag
                    
                    // Switch camera mode
                    m_Player.SetCameraMode    ( CCamera::MODE_THIRDPERSON_SC );
                    m_pCamera = m_Player.GetCamera();

                    break;

                case ID_LIGHT_0:
                case ID_LIGHT_1:
                case ID_LIGHT_2:
                case ID_LIGHT_3:
                case ID_LIGHT_4:
                {
                    // Enable / Disable the specified light
                    ULONG LightID = LOWORD(wParam) - ID_LIGHT_0, Flags = MF_BYCOMMAND;
                    m_LightEnabled[ LightID ] = !m_LightEnabled[ LightID ];
                    m_pD3DDevice->LightEnable( LightID, m_LightEnabled[ LightID ] );

                    // Adjust menu item
                    if ( m_LightEnabled[ LightID ] ) Flags |= MF_CHECKED; else Flags |= MF_UNCHECKED;
                    ::CheckMenuItem( m_hMenu, LOWORD(wParam), Flags );

                    break;

                } // End Cases
                case ID_FILLMODE_SOLID:
                    // Set fill mode to solid
                    m_FillMode = D3DFILL_SOLID;
                    m_pD3DDevice->SetRenderState( D3DRS_FILLMODE, m_FillMode );
                    ::CheckMenuRadioItem( m_hMenu, ID_FILLMODE_SOLID, ID_FILLMODE_WIREFRAME,
                                          ID_FILLMODE_SOLID, MF_BYCOMMAND );
                    break;

                case ID_FILLMODE_WIREFRAME:
                    // Set fill mode to wireframe
                    m_FillMode = D3DFILL_WIREFRAME;
                    m_pD3DDevice->SetRenderState( D3DRS_FILLMODE, m_FillMode );
                    ::CheckMenuRadioItem( m_hMenu, ID_FILLMODE_SOLID, ID_FILLMODE_WIREFRAME,
                                          ID_FILLMODE_WIREFRAME, MF_BYCOMMAND );
                    break;

            
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
// Desc : Build our demonstration meshes, and the objects that instance them
//-----------------------------------------------------------------------------
bool CGameApp::BuildObjects()
{
    VERTEXPROCESSING_TYPE vp = m_D3DSettings.GetSettings()->VertexProcessingType;
    bool HardwareTnL = true;
    
    // Are we using HardwareTnL ?
    if ( vp != HARDWARE_VP && vp != PURE_HARDWARE_VP ) HardwareTnL = false;

    // Release previously built objects
    ReleaseObjects();

    // Build our terrain data
    m_Terrain.SetD3DDevice( m_pD3DDevice, HardwareTnL );
    if ( !m_Terrain.LoadHeightMap( _T("Data\\HeightMap.raw"), 257, 257 )) return false;

    // Build a 'player' mesh (this is just a cube currently)
    CVertex * pVertex = NULL;
    srand( timeGetTime() );

    // Add the 8 cube vertices to this mesh
    if ( m_PlayerMesh.AddVertex( 8 ) < 0 ) return false;

    // Add all 4 vertices
    pVertex = &m_PlayerMesh.m_pVertex[0];
    
    // Add bottom 4 vertices
    *pVertex++ = CVertex( -3, 0, -3, D3DXVECTOR3( -0.577350f, -0.577350f, -0.577350f ) );
    *pVertex++ = CVertex( -3, 0,  3, D3DXVECTOR3( -0.577350f, -0.577350f,  0.577350f ) );
    *pVertex++ = CVertex(  3, 0,  3, D3DXVECTOR3(  0.577350f, -0.577350f,  0.577350f ) );
    *pVertex++ = CVertex(  3, 0, -3, D3DXVECTOR3(  0.577350f, -0.577350f, -0.577350f ) );
    
    // Add top 4 vertices
    *pVertex++ = CVertex( -3, 20, -3, D3DXVECTOR3( -0.577350f, 0.577350f, -0.577350f ) );
    *pVertex++ = CVertex( -3, 20,  3, D3DXVECTOR3( -0.577350f, 0.577350f,  0.577350f ) );
    *pVertex++ = CVertex(  3, 20,  3, D3DXVECTOR3(  0.577350f, 0.577350f,  0.577350f ) );
    *pVertex++ = CVertex(  3, 20, -3, D3DXVECTOR3(  0.577350f, 0.577350f, -0.577350f ) );

    // Add the indices as a strip (with one degenerate) ;)
    if ( m_PlayerMesh.AddIndex( 16 ) < 0 ) return false;
    m_PlayerMesh.m_pIndex[ 0] = 5;
    m_PlayerMesh.m_pIndex[ 1] = 6;
    m_PlayerMesh.m_pIndex[ 2] = 4;
    m_PlayerMesh.m_pIndex[ 3] = 7; //               5-6
    m_PlayerMesh.m_pIndex[ 4] = 0; //               |/|
    m_PlayerMesh.m_pIndex[ 5] = 3; //               4-7
    m_PlayerMesh.m_pIndex[ 6] = 1; //               |/|
    m_PlayerMesh.m_pIndex[ 7] = 2; //               0-3-7
    m_PlayerMesh.m_pIndex[ 8] = 3; // Degen Index   |/|/|
    m_PlayerMesh.m_pIndex[ 9] = 7; //               1-2-6
    m_PlayerMesh.m_pIndex[10] = 2; //                 |/|
    m_PlayerMesh.m_pIndex[11] = 6; //                 1-5
    m_PlayerMesh.m_pIndex[12] = 1; //                 |/|
    m_PlayerMesh.m_pIndex[13] = 5; //                 0-4
    m_PlayerMesh.m_pIndex[14] = 0;
    m_PlayerMesh.m_pIndex[15] = 4;

    // Build the mesh's vertex and index buffers
    if (FAILED(m_PlayerMesh.BuildBuffers( m_pD3DDevice, HardwareTnL, true ))) return false;

    // Our object references this mesh
    m_Object.m_pMesh = &m_PlayerMesh;

    // Link this object to our player
    m_Player.Set3rdPersonObject( &m_Object );

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
    // De-reference device objects stream (just in case any remain set)
    if ( m_pD3DDevice ) 
    {
        m_pD3DDevice->SetStreamSource( 0, NULL, 0, 0 );
        m_pD3DDevice->SetIndices( NULL );

    } // End if Device Available

    // Release any required objects
    m_Terrain.Release();
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
    if ( m_LastFrameRate != m_Timer.GetFrameRate() )
    {
        m_LastFrameRate = m_Timer.GetFrameRate( FrameRate );
        _stprintf( TitleBuffer, _T("Simple Terrain : %s"), FrameRate );
        SetWindowText( m_hWnd, TitleBuffer );

    } // End if Frame Rate Altered

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

    // Animate the game objects
    AnimateObjects();

    // Clear the frame & depth buffer ready for drawing
    m_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x79D3FF, 1.0f, 0 );
    
    // Begin Scene Rendering
    m_pD3DDevice->BeginScene();
    
    // Reset our world matrix
    m_pD3DDevice->SetTransform( D3DTS_WORLD, &m_mtxIdentity );

    // Render our terrain objects
    m_Terrain.Render( m_pCamera );

    // Request our player render itselfs
    m_Player.Render( m_pD3DDevice );

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
    static UCHAR pKeyBuffer[ 256 ];
    ULONG        Direction = 0;
    POINT        CursorPos;
    float        X = 0.0f, Y = 0.0f;

    // Retrieve keyboard state
    if ( !GetKeyboardState( pKeyBuffer ) ) return;

    // Check the relevant keys
    if ( pKeyBuffer[ VK_UP    ] & 0xF0 ) Direction |= CPlayer::DIR_FORWARD;
    if ( pKeyBuffer[ VK_DOWN  ] & 0xF0 ) Direction |= CPlayer::DIR_BACKWARD;
    if ( pKeyBuffer[ VK_LEFT  ] & 0xF0 ) Direction |= CPlayer::DIR_LEFT;
    if ( pKeyBuffer[ VK_RIGHT ] & 0xF0 ) Direction |= CPlayer::DIR_RIGHT;
    if ( pKeyBuffer[ VK_PRIOR ] & 0xF0 ) Direction |= CPlayer::DIR_UP;
    if ( pKeyBuffer[ VK_NEXT  ] & 0xF0 ) Direction |= CPlayer::DIR_DOWN;
    
    // Now process the mouse (if the button is pressed)
    if ( GetCapture() == m_hWnd )
    {
        // Hide the mouse pointer
        SetCursor( NULL );

        // Retrieve the cursor position
        GetCursorPos( &CursorPos );

        // Calculate mouse rotational values
        X = (float)(CursorPos.x - m_OldCursorPos.x) / 3.0f;
        Y = (float)(CursorPos.y - m_OldCursorPos.y) / 3.0f;

        // Reset our cursor position so we can keep going forever :)
        SetCursorPos( m_OldCursorPos.x, m_OldCursorPos.y );

    } // End if Captured

    // Update if we have moved
    if ( Direction > 0 || X != 0.0f || Y != 0.0f )
    {
        // Rotate our camera
        if ( X || Y ) 
        {
            // Are they holding the right mouse button ?
            if ( pKeyBuffer[ VK_RBUTTON ] & 0xF0 )
                m_Player.Rotate( Y, 0.0f, -X );
            else
                m_Player.Rotate( Y, X, 0.0f );
        
        } // End if any rotation

        // Any Movement ?
        if ( Direction ) 
        {
            // Move our player (Force applied must be greater than total friction)
            m_Player.Move( Direction, 500.0f * m_Timer.GetTimeElapsed(), true );

        } // End if any movement

    } // End if camera moved

    // Update our camera (updates velocity etc)
    m_Player.Update( m_Timer.GetTimeElapsed() );

    // Update the device matrix
    m_pCamera->UpdateRenderView( m_pD3DDevice );

}

//-----------------------------------------------------------------------------
// Name : AnimateObjects () (Private)
// Desc : Animates the objects we currently have loaded.
//-----------------------------------------------------------------------------
void CGameApp::AnimateObjects()
{
    static float Angle1 = 0;
    static float Angle2 = 6.28f;

    // Update Light Positions
    m_Light[1].Position = m_Player.GetPosition();
    m_pD3DDevice->SetLight( 1, &m_Light[1] );

    m_Light[2].Position.x = 500.0f + (sinf( Angle1 ) * 250);
    m_Light[2].Position.z = 500.0f + (cosf( Angle1 ) * 250);
    m_Light[2].Position.y   = m_Terrain.GetHeight( m_Light[2].Position.x, m_Light[2].Position.z ) + 30.0f;
    m_pD3DDevice->SetLight( 2, &m_Light[2] );

    m_Light[3].Position.x = 1000.0f + (sinf( Angle2 ) * 250);
    m_Light[3].Position.z = 1000.0f + (cosf( Angle2 ) * 250);
    m_Light[3].Position.y   = m_Terrain.GetHeight( m_Light[3].Position.x, m_Light[3].Position.z ) + 30.0f;
    m_pD3DDevice->SetLight( 3, &m_Light[3] );

    m_Light[4].Position.x = 1500.0f + (sinf( Angle1 ) * 250);
    m_Light[4].Position.z = 1500.0f + (cosf( Angle1 ) * 250);
    m_Light[4].Position.y   = m_Terrain.GetHeight( m_Light[4].Position.x, m_Light[4].Position.z ) + 30.0f;
    m_pD3DDevice->SetLight( 4, &m_Light[4] );

    // Update angle values
    Angle1 += 0.5f * m_Timer.GetTimeElapsed();
    if ( Angle1 > 6.28f ) Angle1 -= 6.28f;

    Angle2 -= 1.0f * m_Timer.GetTimeElapsed();
    if ( Angle2 < 0.0f ) Angle2 += 6.28f;

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