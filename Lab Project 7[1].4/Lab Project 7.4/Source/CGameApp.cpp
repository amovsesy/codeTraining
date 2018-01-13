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
    
    // Set up initial states (these will be adjusted later if not supported)
    m_FillMode      = D3DFILL_SOLID;
    m_ColorOp       = D3DTOP_ADDSIGNED;
    m_MinFilter     = D3DTEXF_LINEAR;
    m_MagFilter     = D3DTEXF_LINEAR;
    m_MipFilter     = D3DTEXF_LINEAR;
    m_Anisotropy    = 1;

    ZeroMemory( m_SkyTextures, 6 * sizeof(LPDIRECT3DTEXTURE9) );
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
    LPTSTR          WindowTitle  = _T("Terrain Alpha");
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
    ::CheckMenuRadioItem( m_hMenu, ID_CAMERAMODE_FPS, ID_CAMERAMODE_THIRDPERSON, ID_CAMERAMODE_FPS, MF_BYCOMMAND );
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

    // Enable all layer renderings
    m_bRenderLayers[0] = true;
    m_bRenderLayers[1] = true;
    m_bRenderLayers[2] = true;

    // Setup the players camera, and extract the pointer.
    // This pointer will only ever become invalid on subsequent
    // calls to CPlayer::SetCameraMode and on player destruction.
    m_Player.SetCameraMode( CCamera::MODE_FPS );
    m_pCamera = m_Player.GetCamera();

    // Setup our player's default details
    m_Player.SetFriction( 250.0f ); // Per Second
    m_Player.SetGravity( D3DXVECTOR3( 0, -500.0f, 0 ) );
    m_Player.SetMaxVelocityXZ( 125.0f );
    m_Player.SetMaxVelocityY ( 400.0f );
    m_Player.SetCamOffset( D3DXVECTOR3( 0.0f, 10.0f, 0.0f ) );
    m_Player.SetCamLag( 0.0f );
    
    // Set up the players collision volume info
    VOLUME_INFO Volume;
    Volume.Min = D3DXVECTOR3( -3,  -20, -3 );
    Volume.Max = D3DXVECTOR3(  3,  10,  3 );
    m_Player.SetVolumeInfo( Volume );

    // Setup our cameras view details
    m_pCamera->SetFOV( 90.0f );
    m_pCamera->SetViewport( m_nViewX, m_nViewY, m_nViewWidth, m_nViewHeight, 1.01f, 50000.0f );

    // Set the camera volume info (matches player volume)
    m_pCamera->SetVolumeInfo( Volume );

    // Add the update callbacks required
    m_Player.AddPlayerCallback( CTerrain::UpdatePlayer, (LPVOID)&m_Terrain );
    m_Player.AddCameraCallback( CTerrain::UpdateCamera, (LPVOID)&m_Terrain );
    
    // Lets give a small initial rotation and set initial position
    m_Player.SetPosition( D3DXVECTOR3( 5433.0f, 400.0f, 8067.0f) );
    m_Player.Rotate( -10, 135, 0 );
}

//-----------------------------------------------------------------------------
// Name : SetupRenderStates ()
// Desc : Sets up all the initial states required by the renderer.
//-----------------------------------------------------------------------------
void CGameApp::SetupRenderStates()
{
    // Test the device capabilities.
    if (!TestDeviceCaps( )) { PostQuitMessage(0); return; }

    // Setup our D3D Device initial states
    m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
    m_pD3DDevice->SetRenderState( D3DRS_DITHERENABLE,  TRUE );
    m_pD3DDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
    m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
    m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    // Set up sampler states.
    m_pD3DDevice->SetSamplerState( 0, D3DSAMP_MINFILTER    , m_MinFilter );
    m_pD3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER    , m_MagFilter );
    m_pD3DDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER    , m_MipFilter );
    m_pD3DDevice->SetSamplerState( 0, D3DSAMP_MAXANISOTROPY, m_Anisotropy );
    m_pD3DDevice->SetSamplerState( 1, D3DSAMP_MINFILTER    , m_MinFilter );
    m_pD3DDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER    , m_MagFilter );
    m_pD3DDevice->SetSamplerState( 1, D3DSAMP_MIPFILTER    , m_MipFilter );
    m_pD3DDevice->SetSamplerState( 1, D3DSAMP_MAXANISOTROPY, m_Anisotropy );
    m_pD3DDevice->SetSamplerState( 1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
    m_pD3DDevice->SetSamplerState( 1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
    
    // Setup option dependant states
    m_pD3DDevice->SetRenderState( D3DRS_FILLMODE, m_FillMode );                

    // Setup our default vertex FVF code
    m_pD3DDevice->SetFVF( VERTEX_FVF );

    // Store new viewport sizes
    RECT rc;
    ::GetClientRect( m_hWnd, &rc );
    m_nViewWidth  = rc.right - rc.left;
    m_nViewHeight = rc.bottom - rc.top;

    // Update our device with our camera details (Required on reset)
    if ( !m_pCamera ) return;
    m_pCamera->SetViewport( m_nViewX, m_nViewY, m_nViewWidth, m_nViewHeight, 1.01f, 50000.0f );
    m_pCamera->UpdateRenderView( m_pD3DDevice );
    m_pCamera->UpdateRenderProj( m_pD3DDevice );

    // Set up the menu item selections (Which may have changed during device validations)
    SelectMenuItems();

}

//-----------------------------------------------------------------------------
// Name : TestDeviceCaps ()
// Desc : Sets up our application state based on which caps we have available.
//-----------------------------------------------------------------------------
bool CGameApp::TestDeviceCaps( )
{
    HRESULT  hRet;
    D3DCAPS9 Caps;
    ULONG    Enable, Value;

    // Retrieve device caps, the following states are not required (as in ValidateDevice)
    CD3DSettings::Settings * pSettings = m_D3DSettings.GetSettings();
    hRet = m_pD3D->GetDeviceCaps( pSettings->AdapterOrdinal, pSettings->DeviceType, &Caps );
    if ( FAILED(hRet) ) return false;
    
    // Reset our caps storage.
    // Note: These store the available of actual state values
    ZeroMemory( m_MinFilterCaps, 10 * sizeof(bool) );
    ZeroMemory( m_MagFilterCaps, 10 * sizeof(bool) );
    ZeroMemory( m_MipFilterCaps, 10 * sizeof(bool) );
    
    // Set up those states always supported
    m_MinFilterCaps[D3DTEXF_NONE] = true;
    m_MagFilterCaps[D3DTEXF_NONE] = true;
    m_MipFilterCaps[D3DTEXF_NONE] = true;
    m_MaxTextures   = 0;
    m_MaxAnisotropy = 0;

    // Test Texture Filter Caps
    Value = Caps.TextureFilterCaps;

    // Determine if anisotropic minification filtering is supported
    Enable = MF_ENABLED;
    if ( !(Value & D3DPTFILTERCAPS_MINFANISOTROPIC) ) Enable = MF_DISABLED | MF_GRAYED;
    EnableMenuItem( m_hMenu, ID_MINFILTER_ANISOTROPIC, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) m_MinFilterCaps[D3DTEXF_ANISOTROPIC] = true;

    // Determine if linear minification filtering is supported
    Enable = MF_ENABLED;
    if ( !(Value & D3DPTFILTERCAPS_MINFLINEAR) ) Enable = MF_DISABLED | MF_GRAYED;
    EnableMenuItem( m_hMenu, ID_MINFILTER_LINEAR, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) m_MinFilterCaps[D3DTEXF_LINEAR] = true;
    
    // Determine if point minification filtering is supported
    Enable = MF_ENABLED;
    if ( !(Value & D3DPTFILTERCAPS_MINFPOINT) ) Enable = MF_DISABLED | MF_GRAYED;
    EnableMenuItem( m_hMenu, ID_MINFILTER_POINT, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) m_MinFilterCaps[D3DTEXF_POINT] = true;

    // Determine if anisotropic magnification filtering is supported
    Enable = MF_ENABLED;
    if ( !(Value & D3DPTFILTERCAPS_MAGFANISOTROPIC) ) Enable = MF_DISABLED | MF_GRAYED;
    EnableMenuItem( m_hMenu, ID_MAGFILTER_ANISOTROPIC, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) m_MagFilterCaps[D3DTEXF_ANISOTROPIC] = true;

    // Determine if linear magnification filtering is supported
    Enable = MF_ENABLED;
    if ( !(Value & D3DPTFILTERCAPS_MAGFLINEAR) ) Enable = MF_DISABLED | MF_GRAYED;
    EnableMenuItem( m_hMenu, ID_MAGFILTER_LINEAR, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) m_MagFilterCaps[D3DTEXF_LINEAR] = true;
    
    // Determine if point magnification filtering is supported
    Enable = MF_ENABLED;
    if ( !(Value & D3DPTFILTERCAPS_MAGFPOINT) ) Enable = MF_DISABLED | MF_GRAYED;
    EnableMenuItem( m_hMenu, ID_MAGFILTER_POINT, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) m_MagFilterCaps[D3DTEXF_POINT] = true;
    
    // Determine if linear mip filtering is supported
    Enable = MF_ENABLED;
    if ( !(Value & D3DPTFILTERCAPS_MIPFLINEAR) ) Enable = MF_DISABLED | MF_GRAYED;
    EnableMenuItem( m_hMenu, ID_MIPFILTER_LINEAR, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) m_MipFilterCaps[D3DTEXF_LINEAR] = true;

    // Determine if point mip filtering is supported
    Enable = MF_ENABLED;
    if ( !(Value & D3DPTFILTERCAPS_MIPFPOINT) ) Enable = MF_DISABLED | MF_GRAYED;
    EnableMenuItem( m_hMenu, ID_MIPFILTER_POINT, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) m_MipFilterCaps[D3DTEXF_POINT] = true;

    // Test anisotropy levels
    Value = Caps.MaxAnisotropy;

    // Determine which anisotropy levels are supported
    if ( Value < 1 ) Enable = MF_DISABLED | MF_GRAYED; else Enable = MF_ENABLED;
    EnableMenuItem( m_hMenu, ID_MAXANISOTROPY_1, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) m_MaxAnisotropy = 1;

    if ( Value < 2 ) Enable = MF_DISABLED | MF_GRAYED; else Enable = MF_ENABLED;
    EnableMenuItem( m_hMenu, ID_MAXANISOTROPY_2, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) m_MaxAnisotropy = 2;

    if ( Value < 4 ) Enable = MF_DISABLED | MF_GRAYED; else Enable = MF_ENABLED;
    EnableMenuItem( m_hMenu, ID_MAXANISOTROPY_4, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) m_MaxAnisotropy = 4;

    if ( Value < 8 ) Enable = MF_DISABLED | MF_GRAYED; else Enable = MF_ENABLED;
    EnableMenuItem( m_hMenu, ID_MAXANISOTROPY_8, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) m_MaxAnisotropy = 8;

    if ( Value < 16 ) Enable = MF_DISABLED | MF_GRAYED; else Enable = MF_ENABLED;
    EnableMenuItem( m_hMenu, ID_MAXANISOTROPY_16, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) m_MaxAnisotropy = 16;
    
    if ( Value < 32 ) Enable = MF_DISABLED | MF_GRAYED; else Enable = MF_ENABLED;
    EnableMenuItem( m_hMenu, ID_MAXANISOTROPY_32, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) m_MaxAnisotropy = 32;

    if ( Value < 64 ) Enable = MF_DISABLED | MF_GRAYED; else Enable = MF_ENABLED;
    EnableMenuItem( m_hMenu, ID_MAXANISOTROPY_64, MF_BYCOMMAND | Enable );
    if ( Enable == MF_ENABLED ) m_MaxAnisotropy = 64;

    // Now determine if our currently selected states are supported, swap otherwise
    if ( m_MinFilterCaps[ m_MinFilter ] == false )
    {
             if ( m_MinFilterCaps[ D3DTEXF_ANISOTROPIC ] ) m_MinFilter = D3DTEXF_ANISOTROPIC;
        else if ( m_MinFilterCaps[ D3DTEXF_LINEAR      ] ) m_MinFilter = D3DTEXF_LINEAR;
        else if ( m_MinFilterCaps[ D3DTEXF_POINT       ] ) m_MinFilter = D3DTEXF_POINT;
        else if ( m_MinFilterCaps[ D3DTEXF_NONE        ] ) m_MinFilter = D3DTEXF_NONE;
        else return false;
    
    } // End if Filter not supported

    if ( m_MagFilterCaps[ m_MagFilter ] == false )
    {
             if ( m_MagFilterCaps[ D3DTEXF_ANISOTROPIC ] ) m_MagFilter = D3DTEXF_ANISOTROPIC;
        else if ( m_MagFilterCaps[ D3DTEXF_LINEAR      ] ) m_MagFilter = D3DTEXF_LINEAR;
        else if ( m_MagFilterCaps[ D3DTEXF_POINT       ] ) m_MagFilter = D3DTEXF_POINT;
        else if ( m_MagFilterCaps[ D3DTEXF_NONE        ] ) m_MagFilter = D3DTEXF_NONE;
        else return false;
    
    } // End if Filter not supported

    if ( m_MipFilterCaps[ m_MipFilter ] == false )
    {
             if ( m_MipFilterCaps[ D3DTEXF_ANISOTROPIC ] ) m_MipFilter = D3DTEXF_ANISOTROPIC;
        else if ( m_MipFilterCaps[ D3DTEXF_LINEAR      ] ) m_MipFilter = D3DTEXF_LINEAR;
        else if ( m_MipFilterCaps[ D3DTEXF_POINT       ] ) m_MipFilter = D3DTEXF_POINT;
        else if ( m_MipFilterCaps[ D3DTEXF_NONE        ] ) m_MipFilter = D3DTEXF_NONE;
        else return false;
    
    } // End if Mip-Filter not supported

    // Test max anisotropy
    if ( m_Anisotropy > m_MaxAnisotropy ) m_Anisotropy = m_MaxAnisotropy;

    ULONG      Ordinal = pSettings->AdapterOrdinal;
    D3DDEVTYPE Type    = pSettings->DeviceType;
    D3DFORMAT  AFormat = pSettings->DisplayMode.Format;
    
    m_TextureFormat    = D3DFMT_UNKNOWN;
    m_AlphaFormat      = D3DFMT_UNKNOWN;
    
    // Find texture formats we would like to use (Prefer compressed textures in this demo)
    if ( SUCCEEDED( m_pD3D->CheckDeviceFormat( Ordinal, Type, AFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT1 )) )
        m_TextureFormat = D3DFMT_DXT1;
    else if ( SUCCEEDED( m_pD3D->CheckDeviceFormat( Ordinal, Type, AFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_X8R8G8B8 )) )
        m_TextureFormat = D3DFMT_X8R8G8B8;
    else if ( SUCCEEDED( m_pD3D->CheckDeviceFormat( Ordinal, Type, AFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_R5G6B5 )) )
        m_TextureFormat = D3DFMT_R5G6B5;
    else if ( SUCCEEDED( m_pD3D->CheckDeviceFormat( Ordinal, Type, AFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_X1R5G5B5 )) )
        m_TextureFormat = D3DFMT_X1R5G5B5;
    else
    {
        MessageBox( 0, "No valid texture formats could be found.", "Error.", MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        return false;
    
    } // End if 

    // Find alpha texture formats we would like to use (Prefer compressed textures in this demo)
    if ( SUCCEEDED( m_pD3D->CheckDeviceFormat( Ordinal, Type, AFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_DXT3 )) ) 
        m_AlphaFormat = D3DFMT_DXT3;
    else if ( SUCCEEDED( m_pD3D->CheckDeviceFormat( Ordinal, Type, AFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8 )) )
        m_AlphaFormat = D3DFMT_A8R8G8B8;
    else if ( SUCCEEDED( m_pD3D->CheckDeviceFormat( Ordinal, Type, AFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_A1R5G5B5 )) )
        m_AlphaFormat = D3DFMT_A1R5G5B5;
    else if ( SUCCEEDED( m_pD3D->CheckDeviceFormat( Ordinal, Type, AFormat, 0, D3DRTYPE_TEXTURE, D3DFMT_A4R4G4B4 )) )
        m_AlphaFormat = D3DFMT_A4R4G4B4;
    else
    {
        MessageBox( 0, "No valid alpha texture formats could be found.", "Error.", MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        return false;
    
    } // End if 

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : SelectMenuItems ()
// Desc : Sets up our application's menu item selection states.
//-----------------------------------------------------------------------------
void CGameApp::SelectMenuItems()
{
    // Select which minfication filter menu item is selected
    switch ( m_MinFilter )
    {
        case D3DTEXF_NONE: 
            ::CheckMenuRadioItem( m_hMenu, ID_MINFILTER_NONE, ID_MINFILTER_ANISOTROPIC, ID_MINFILTER_NONE, MF_BYCOMMAND );
            break;
        case D3DTEXF_POINT: 
            ::CheckMenuRadioItem( m_hMenu, ID_MINFILTER_NONE, ID_MINFILTER_ANISOTROPIC, ID_MINFILTER_POINT, MF_BYCOMMAND );
            break;
        case D3DTEXF_LINEAR: 
            ::CheckMenuRadioItem( m_hMenu, ID_MINFILTER_NONE, ID_MINFILTER_ANISOTROPIC, ID_MINFILTER_LINEAR, MF_BYCOMMAND );
            break;
        case D3DTEXF_ANISOTROPIC: 
            ::CheckMenuRadioItem( m_hMenu, ID_MINFILTER_NONE, ID_MINFILTER_ANISOTROPIC, ID_MINFILTER_ANISOTROPIC, MF_BYCOMMAND );
            break;
    
    } // End Switch

    // Select which magnification filter menu item is selected
    switch ( m_MagFilter )
    {
        case D3DTEXF_NONE: 
            ::CheckMenuRadioItem( m_hMenu, ID_MAGFILTER_NONE, ID_MAGFILTER_ANISOTROPIC, ID_MAGFILTER_NONE, MF_BYCOMMAND );
            break;
        case D3DTEXF_POINT: 
            ::CheckMenuRadioItem( m_hMenu, ID_MAGFILTER_NONE, ID_MAGFILTER_ANISOTROPIC, ID_MAGFILTER_POINT, MF_BYCOMMAND );
            break;
        case D3DTEXF_LINEAR: 
            ::CheckMenuRadioItem( m_hMenu, ID_MAGFILTER_NONE, ID_MAGFILTER_ANISOTROPIC, ID_MAGFILTER_LINEAR, MF_BYCOMMAND );
            break;
        case D3DTEXF_ANISOTROPIC: 
            ::CheckMenuRadioItem( m_hMenu, ID_MAGFILTER_NONE, ID_MAGFILTER_ANISOTROPIC, ID_MAGFILTER_ANISOTROPIC, MF_BYCOMMAND );
            break;
    
    } // End Switch
    
    // Select which mip-filter menu item is selected
    switch ( m_MipFilter )
    {
        case D3DTEXF_NONE: 
            ::CheckMenuRadioItem( m_hMenu, ID_MIPFILTER_NONE, ID_MIPFILTER_LINEAR, ID_MIPFILTER_NONE, MF_BYCOMMAND );
            break;
        case D3DTEXF_POINT: 
            ::CheckMenuRadioItem( m_hMenu, ID_MIPFILTER_NONE, ID_MIPFILTER_LINEAR, ID_MIPFILTER_POINT, MF_BYCOMMAND );
            break;
        case D3DTEXF_LINEAR: 
            ::CheckMenuRadioItem( m_hMenu, ID_MIPFILTER_NONE, ID_MIPFILTER_LINEAR, ID_MIPFILTER_LINEAR, MF_BYCOMMAND );
            break;

    } // End Switch

    // Select which fill mode menu item is selected
    switch ( m_FillMode )
    {
        case D3DFILL_WIREFRAME:
            ::CheckMenuRadioItem( m_hMenu, ID_FILLMODE_SOLID, ID_FILLMODE_WIREFRAME, ID_FILLMODE_WIREFRAME, MF_BYCOMMAND );
            break;

        case D3DFILL_SOLID:
            ::CheckMenuRadioItem( m_hMenu, ID_FILLMODE_SOLID, ID_FILLMODE_WIREFRAME, ID_FILLMODE_SOLID, MF_BYCOMMAND );
            break;

    } // End Switch

    // Select which anisotropy level menu item is selected
    switch ( m_Anisotropy )
    {
        case 1:
            ::CheckMenuRadioItem( m_hMenu, ID_MAXANISOTROPY_1, ID_MAXANISOTROPY_64, ID_MAXANISOTROPY_1, MF_BYCOMMAND );
            break;
        case 2:
            ::CheckMenuRadioItem( m_hMenu, ID_MAXANISOTROPY_1, ID_MAXANISOTROPY_64, ID_MAXANISOTROPY_2, MF_BYCOMMAND );
            break;
        case 4:
            ::CheckMenuRadioItem( m_hMenu, ID_MAXANISOTROPY_1, ID_MAXANISOTROPY_64, ID_MAXANISOTROPY_4, MF_BYCOMMAND );
            break;
        case 8:
            ::CheckMenuRadioItem( m_hMenu, ID_MAXANISOTROPY_1, ID_MAXANISOTROPY_64, ID_MAXANISOTROPY_8, MF_BYCOMMAND );
            break;
        case 16:
            ::CheckMenuRadioItem( m_hMenu, ID_MAXANISOTROPY_1, ID_MAXANISOTROPY_64, ID_MAXANISOTROPY_16, MF_BYCOMMAND );
            break;
        case 32:
            ::CheckMenuRadioItem( m_hMenu, ID_MAXANISOTROPY_1, ID_MAXANISOTROPY_64, ID_MAXANISOTROPY_32, MF_BYCOMMAND );
            break;
        case 64:
            ::CheckMenuRadioItem( m_hMenu, ID_MAXANISOTROPY_1, ID_MAXANISOTROPY_64, ID_MAXANISOTROPY_64, MF_BYCOMMAND );
            break;

    } // End Switch    
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
                    if ( m_pCamera ) m_pCamera->SetViewport( m_nViewX, m_nViewY, m_nViewWidth, m_nViewHeight, 1.01f, 50000.0f );
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
                    ::CheckMenuRadioItem( m_hMenu, ID_CAMERAMODE_FPS, ID_CAMERAMODE_THIRDPERSON,
                                          ID_CAMERAMODE_FPS, MF_BYCOMMAND );

                    // Setup player details
                    m_Player.SetFriction      ( 250.0f ); // Per Second
                    m_Player.SetGravity       ( D3DXVECTOR3( 0, -500.0f, 0 ) );
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
                    ::CheckMenuRadioItem( m_hMenu, ID_CAMERAMODE_FPS, ID_CAMERAMODE_THIRDPERSON,
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
                    ::CheckMenuRadioItem( m_hMenu, ID_CAMERAMODE_FPS, ID_CAMERAMODE_THIRDPERSON,
                                          ID_CAMERAMODE_THIRDPERSON, MF_BYCOMMAND );

                    // Setup Player details
                    m_Player.SetFriction      ( 250.0f ); // Per Second
                    m_Player.SetGravity       ( D3DXVECTOR3( 0, -500.0f, 0 ) );
                    m_Player.SetMaxVelocityXZ ( 125.0f );
                    m_Player.SetMaxVelocityY  ( 400.0f );
                    m_Player.SetCamOffset     ( D3DXVECTOR3( 0.0f, 40.0f, -60.0f ) );
                    m_Player.SetCamLag        ( 0.25f ); // 1/4 second camera lag
                    
                    // Switch camera mode
                    m_Player.SetCameraMode    ( CCamera::MODE_THIRDPERSON );
                    m_pCamera = m_Player.GetCamera();

                    break;

                case ID_FILLMODE_SOLID:
                    // Set fill mode to solid
                    m_FillMode = D3DFILL_SOLID;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_FILLMODE_WIREFRAME:
                    // Set fill mode to wireframe
                    m_FillMode = D3DFILL_WIREFRAME;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MINFILTER_NONE:
                    // Set filter modes to none
                    m_MinFilter = D3DTEXF_NONE;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MINFILTER_POINT:
                    // Set filter modes to point
                    m_MinFilter = D3DTEXF_POINT;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MINFILTER_LINEAR:
                    // Set filter modes to linear
                    m_MinFilter = D3DTEXF_LINEAR;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MINFILTER_ANISOTROPIC:
                    // Set filter modes to anisotropic
                    m_MinFilter = D3DTEXF_ANISOTROPIC;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MAGFILTER_NONE:
                    // Set filter modes to none
                    m_MagFilter = D3DTEXF_NONE;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MAGFILTER_POINT:
                    // Set filter modes to point
                    m_MagFilter = D3DTEXF_POINT;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MAGFILTER_LINEAR:
                    // Set filter modes to linear
                    m_MagFilter = D3DTEXF_LINEAR;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MAGFILTER_ANISOTROPIC:
                    // Set filter modes to anisotropic
                    m_MagFilter = D3DTEXF_ANISOTROPIC;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;


                case ID_MIPFILTER_NONE:
                    // Set mip filter modes to none
                    m_MipFilter = D3DTEXF_NONE;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MIPFILTER_POINT:
                    // Set mip filter modes to point
                    m_MipFilter = D3DTEXF_POINT;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_MIPFILTER_LINEAR:
                    // Set mip filter modes to linear
                    m_MipFilter = D3DTEXF_LINEAR;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case  ID_MAXANISOTROPY_1:
                    // Set alternate max anisotropy 
                    m_Anisotropy = 1;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case  ID_MAXANISOTROPY_2:
                    // Set alternate max anisotropy 
                    m_Anisotropy = 2;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case  ID_MAXANISOTROPY_4:
                    // Set alternate max anisotropy 
                    m_Anisotropy = 4;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case  ID_MAXANISOTROPY_8:
                    // Set alternate max anisotropy 
                    m_Anisotropy = 8;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case  ID_MAXANISOTROPY_16:
                    // Set alternate max anisotropy 
                    m_Anisotropy = 16;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case  ID_MAXANISOTROPY_32:
                    // Set alternate max anisotropy 
                    m_Anisotropy = 32;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case  ID_MAXANISOTROPY_64:
                    // Set alternate max anisotropy 
                    m_Anisotropy = 64;
                    SetupRenderStates(); // Called here to allow state code centralization
                    break;

                case ID_RENDERLAYER_BASE:
                    m_bRenderLayers[0] = !m_bRenderLayers[0];
                    ::CheckMenuItem( m_hMenu, ID_RENDERLAYER_BASE, MF_BYCOMMAND | m_bRenderLayers[0] ? MF_CHECKED : MF_UNCHECKED );
                    break;

                case ID_RENDERLAYER_1:
                    m_bRenderLayers[1] = !m_bRenderLayers[1];
                    ::CheckMenuItem( m_hMenu, ID_RENDERLAYER_1, MF_BYCOMMAND | m_bRenderLayers[1] ? MF_CHECKED : MF_UNCHECKED );
                    break;

                case ID_RENDERLAYER_2:
                    m_bRenderLayers[2] = !m_bRenderLayers[2];
                    ::CheckMenuItem( m_hMenu, ID_RENDERLAYER_2, MF_BYCOMMAND | m_bRenderLayers[2] ? MF_CHECKED : MF_UNCHECKED );
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
    char IniPath[MAX_PATH];

    VERTEXPROCESSING_TYPE vp = m_D3DSettings.GetSettings()->VertexProcessingType;
    bool HardwareTnL = true;
    
    // Are we using HardwareTnL ?
    if ( vp != HARDWARE_VP && vp != PURE_HARDWARE_VP ) HardwareTnL = false;

    // Release previously built objects
    ReleaseObjects();

    // Retrieve the current directory and append the def file name.
    GetCurrentDirectory( MAX_PATH, IniPath );
    strcat( IniPath, "\\Data\\Level1.ini" );

    // Build the terrain data
    m_Terrain.SetD3DDevice( m_pD3DDevice, HardwareTnL );
    if ( !m_Terrain.LoadTerrain( IniPath )) return false;

    // Build a 'player' mesh (this is just a cube currently)
    CLitVertex * pVertex = NULL;
    srand( timeGetTime() );

    // Add the 8 cube vertices to this mesh
    m_PlayerMesh.SetVertexFormat( LITVERTEX_FVF, sizeof(CLitVertex));
    if ( m_PlayerMesh.AddVertex( 8 ) < 0 ) return false;
    pVertex = (CLitVertex*)m_PlayerMesh.m_pVertex;
    
    // Add bottom 4 vertices
    *pVertex++ = CLitVertex( -3, 0, -3, RANDOM_COLOR );
    *pVertex++ = CLitVertex( -3, 0,  3, RANDOM_COLOR );
    *pVertex++ = CLitVertex(  3, 0,  3, RANDOM_COLOR );
    *pVertex++ = CLitVertex(  3, 0, -3, RANDOM_COLOR );
    
    // Add top 4 vertices
    *pVertex++ = CLitVertex( -3, 20, -3, RANDOM_COLOR );
    *pVertex++ = CLitVertex( -3, 20,  3, RANDOM_COLOR );
    *pVertex++ = CLitVertex(  3, 20,  3, RANDOM_COLOR );
    *pVertex++ = CLitVertex(  3, 20, -3, RANDOM_COLOR );

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

    // Build the skybox
    if ( !BuildSkyBox() ) return false;

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
        m_pD3DDevice->SetTexture( 0, NULL );
        m_pD3DDevice->SetTexture( 1, NULL );

    } // End if Device Available

    // Release textures
    for ( ULONG i = 0; i < 6; ++i ) if ( m_SkyTextures[i] ) m_SkyTextures[i]->Release();
    ZeroMemory( m_SkyTextures, 6 * sizeof(LPDIRECT3DTEXTURE9) );

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
        _stprintf( TitleBuffer, _T("Terrain Alpha : %s"), FrameRate );
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

    // Sky box must ALWAYS be rendered first
    RenderSkyBox();
    
    // Render player mesh before terrain, because terrain may render alpha components
    m_Player.Render( m_pD3DDevice );

    // Reset our world matrix (player sets it)
    m_pD3DDevice->SetTransform( D3DTS_WORLD, &m_mtxIdentity );

    // Render our terrain objects
    m_Terrain.Render( m_pCamera );

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
    // No objects to animate in this demonstration
}

//-----------------------------------------------------------------------------
// Name : BuildSkyBox ()
// Desc : Build the sky box data ready for rendering.
//-----------------------------------------------------------------------------
bool CGameApp::BuildSkyBox()
{
    HRESULT hRet;
    
    // Allocate our 24 mesh vertices
    m_SkyMesh.SetVertexFormat( LITVERTEX_FVF, sizeof(CLitVertex) );
    if ( m_SkyMesh.AddVertex( 24 ) < 0 ) return false;
    
    // Build the skybox vertices
    CLitVertex * pVertices = (CLitVertex*)m_SkyMesh.m_pVertex;

    // Front quad (remember all quads point inward)
    pVertices[0]  = CLitVertex( -10.0f,  10.0f,  10.0f, 0xFFFFFFFF, 0.0f, 0.0f );
    pVertices[1]  = CLitVertex(  10.0f,  10.0f,  10.0f, 0xFFFFFFFF, 1.0f, 0.0f );
    pVertices[2]  = CLitVertex(  10.0f, -10.0f,  10.0f, 0xFFFFFFFF, 1.0f, 1.0f );
    pVertices[3]  = CLitVertex( -10.0f, -10.0f,  10.0f, 0xFFFFFFFF, 0.0f, 1.0f );

    // Back Quad
    pVertices[4]  = CLitVertex(  10.0f,  10.0f, -10.0f, 0xFFFFFFFF, 0.0f, 0.0f );
    pVertices[5]  = CLitVertex( -10.0f,  10.0f, -10.0f, 0xFFFFFFFF, 1.0f, 0.0f );
    pVertices[6]  = CLitVertex( -10.0f, -10.0f, -10.0f, 0xFFFFFFFF, 1.0f, 1.0f );
    pVertices[7]  = CLitVertex(  10.0f, -10.0f, -10.0f, 0xFFFFFFFF, 0.0f, 1.0f );

    // Left Quad
    pVertices[8]  = CLitVertex( -10.0f,  10.0f, -10.0f, 0xFFFFFFFF, 0.0f, 0.0f );
    pVertices[9]  = CLitVertex( -10.0f,  10.0f,  10.0f, 0xFFFFFFFF, 1.0f, 0.0f );
    pVertices[10] = CLitVertex( -10.0f, -10.0f,  10.0f, 0xFFFFFFFF, 1.0f, 1.0f );
    pVertices[11] = CLitVertex( -10.0f, -10.0f, -10.0f, 0xFFFFFFFF, 0.0f, 1.0f );

    // Right Quad
    pVertices[12] = CLitVertex(  10.0f,  10.0f,  10.0f, 0xFFFFFFFF, 0.0f, 0.0f );
    pVertices[13] = CLitVertex(  10.0f,  10.0f, -10.0f, 0xFFFFFFFF, 1.0f, 0.0f );
    pVertices[14] = CLitVertex(  10.0f, -10.0f, -10.0f, 0xFFFFFFFF, 1.0f, 1.0f );
    pVertices[15] = CLitVertex(  10.0f, -10.0f,  10.0f, 0xFFFFFFFF, 0.0f, 1.0f );

    // Top Quad
    pVertices[16] = CLitVertex( -10.0f,  10.0f, -10.0f, 0xFFFFFFFF, 0.0f, 0.0f );
    pVertices[17] = CLitVertex(  10.0f,  10.0f, -10.0f, 0xFFFFFFFF, 1.0f, 0.0f );
    pVertices[18] = CLitVertex(  10.0f,  10.0f,  10.0f, 0xFFFFFFFF, 1.0f, 1.0f );
    pVertices[19] = CLitVertex( -10.0f,  10.0f,  10.0f, 0xFFFFFFFF, 0.0f, 1.0f );

    // Bottom Quad
    pVertices[20] = CLitVertex( -10.0f, -10.0f,  10.0f, 0xFFFFFFFF, 0.0f, 0.0f );
    pVertices[21] = CLitVertex(  10.0f, -10.0f,  10.0f, 0xFFFFFFFF, 1.0f, 0.0f );
    pVertices[22] = CLitVertex(  10.0f, -10.0f, -10.0f, 0xFFFFFFFF, 1.0f, 1.0f );
    pVertices[23] = CLitVertex( -10.0f, -10.0f, -10.0f, 0xFFFFFFFF, 0.0f, 1.0f );

    // Allocate the indices
    if ( m_SkyMesh.AddIndex( 24 ) < 0 ) return false;
    USHORT * pIndices = m_SkyMesh.m_pIndex;

    // Set the indices for each face (tri strip)
    for ( ULONG i = 0; i < 6; i++ )
    {
        // Build the skybox indices
        *pIndices++ = (i*4);
        *pIndices++ = (i*4) + 1;
        *pIndices++ = (i*4) + 3;
        *pIndices++ = (i*4) + 2;
    
    } // Next 'face'

    VERTEXPROCESSING_TYPE vp = m_D3DSettings.GetSettings()->VertexProcessingType;
    bool HardwareTnL = true;
    
    // Are we using HardwareTnL ?
    if ( vp != HARDWARE_VP && vp != PURE_HARDWARE_VP ) HardwareTnL = false;

    // Build the mesh buffers
    if ( FAILED(m_SkyMesh.BuildBuffers( m_pD3DDevice, HardwareTnL ))) return false;

    // Load Textures
    hRet  = D3DXCreateTextureFromFile( m_pD3DDevice, _T("Data\\SkyBox_Front.jpg") , &m_SkyTextures[0] );
    hRet |= D3DXCreateTextureFromFile( m_pD3DDevice, _T("Data\\SkyBox_Back.jpg")  , &m_SkyTextures[1] );
    hRet |= D3DXCreateTextureFromFile( m_pD3DDevice, _T("Data\\SkyBox_Left.jpg")  , &m_SkyTextures[2] );
    hRet |= D3DXCreateTextureFromFile( m_pD3DDevice, _T("Data\\SkyBox_Right.jpg") , &m_SkyTextures[3] );
    hRet |= D3DXCreateTextureFromFile( m_pD3DDevice, _T("Data\\SkyBox_Top.jpg")   , &m_SkyTextures[4] );
    hRet |= D3DXCreateTextureFromFile( m_pD3DDevice, _T("Data\\SkyBox_Bottom.jpg"), &m_SkyTextures[5] );
    if ( FAILED(hRet) ) return false;
    
    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : RenderSkyBox ()
// Desc : Render the sky box to the frame buffer.
//-----------------------------------------------------------------------------
void CGameApp::RenderSkyBox( )
{
    D3DXMATRIX mtxWorld;
    D3DXMatrixIdentity( &mtxWorld );

    // Validate parameters
    if ( !m_pCamera || !m_pD3DDevice ) return;
    
    // Generate our sky box rendering origin and set as world matrix
    D3DXVECTOR3 CamPos = m_pCamera->GetPosition();
    D3DXMatrixTranslation( &mtxWorld, CamPos.x, CamPos.y + 1.3f, CamPos.z );
    m_pD3DDevice->SetTransform( D3DTS_WORLD, &mtxWorld );

    // Set up rendering states for the sky box
    m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    m_pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
    m_pD3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
    m_pD3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

    // Disable second stage if enabled
    m_pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

    // Disable texture transforms
    m_pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
    
    // Render the sky box
    m_pD3DDevice->SetFVF( m_SkyMesh.m_nFVFCode );
    m_pD3DDevice->SetIndices( m_SkyMesh.m_pIndexBuffer );
    m_pD3DDevice->SetStreamSource( 0, m_SkyMesh.m_pVertexBuffer, 0, m_SkyMesh.m_nStride );
    
    // Render the 6 sides of the skybox
    for ( ULONG i = 0; i < 6; ++i )
    {
        m_pD3DDevice->SetTexture( 0, m_SkyTextures[i] );
        m_pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, 24, i * 4, 2 );

    } // Next side

    // Reset our states
    m_pD3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
    m_pD3DDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
    m_pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
    m_pD3DDevice->SetTransform( D3DTS_WORLD, &m_mtxIdentity );
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
    
    // Must support 'SelectArg1' texture op.
    if ( !(Caps.TextureOpCaps & D3DTEXOPCAPS_SELECTARG1 ) ) return false;

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