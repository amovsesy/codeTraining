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
// Name : CGameApp () (Constructor)
// Desc : CGameApp Class Constructor
//-----------------------------------------------------------------------------
CGameApp::CGameApp()
{
	// Reset / Clear all required values
    m_hWnd              = NULL;
    m_hdcFrameBuffer    = NULL;
    m_hbmSelectOut      = NULL;
    m_hbmFrameBuffer    = NULL;
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

    // Success!
	return true;
}

//-----------------------------------------------------------------------------
// Name : CreateDisplay ()
// Desc : Validate and set the current display device plugin, and create the
//        primary display device. Returns false only if we should bail.
//-----------------------------------------------------------------------------
bool CGameApp::CreateDisplay()
{
    LPTSTR  WindowTitle  = _T("Software Render");
    USHORT  Width        = 400;
    USHORT  Height       = 400;
    HDC     hDC          = NULL;
    RECT    rc;
    
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

    // Build the frame buffer
    if (!BuildFrameBuffer( Width, Height )) return false;
    
	// Show the window
	ShowWindow(m_hWnd, SW_SHOW);

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : BuildFrameBuffer ()
// Desc : Creates the frame buffer memory DC ready for use.
// Note : Destroys the DC / Bitmap when needed, so can be re-used
//-----------------------------------------------------------------------------
bool CGameApp::BuildFrameBuffer( ULONG Width, ULONG Height )
{
    // Obtain window's HDC.
    HDC hDC = ::GetDC( m_hWnd );

    // Create Frame Buffers Device Context if not already existant
    if ( !m_hdcFrameBuffer ) m_hdcFrameBuffer = ::CreateCompatibleDC( hDC );
    
    // If an old FrameBuffer bitmap has already been generated then delete it
    if ( m_hbmFrameBuffer ) 
    {
        // Select the frame buffer back out and destroy it
        ::SelectObject( m_hdcFrameBuffer, m_hbmSelectOut );
        ::DeleteObject( m_hbmFrameBuffer );
        m_hbmFrameBuffer = NULL;
        m_hbmSelectOut   = NULL;

    } // End if

    // Create the frame buffer
    m_hbmFrameBuffer = CreateCompatibleBitmap( hDC, Width, Height );    
    if ( !m_hbmFrameBuffer ) return false;

    // Select this bitmap into our Frame Buffer DC
    m_hbmSelectOut = (HBITMAP)::SelectObject( m_hdcFrameBuffer, m_hbmFrameBuffer );

    // Release windows HDC.
    ::ReleaseDC( m_hWnd, hDC );

    // Set up initial DC states
    ::SetBkMode( m_hdcFrameBuffer, TRANSPARENT );

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : ClearFrameBuffer () (Private)
// Desc : Clears the Frame Buffer (fills with the value passed)
//-----------------------------------------------------------------------------
void CGameApp::ClearFrameBuffer( ULONG Color )
{

    LOGBRUSH logBrush;
    HBRUSH   hBrush = NULL, hOldBrush = NULL;
    
    // Set up the rendering brush
    logBrush.lbStyle = BS_SOLID;
    
    // Set up the colour, converted to BGR & stripped of alpha
    logBrush.lbColor = 0x00FFFFFF & RGB2BGR( Color );
    
    // Create the rendering brush
    hBrush = ::CreateBrushIndirect( &logBrush );
    if (!hBrush) return;

    // Select into the frame buffer DC
    hOldBrush = (HBRUSH)::SelectObject( m_hdcFrameBuffer, hBrush );

    // Draw the rectangle
    Rectangle( m_hdcFrameBuffer, m_nViewX, m_nViewY, m_nViewWidth, m_nViewHeight );
    
    // Destroy rendering brush
    ::SelectObject( m_hdcFrameBuffer, hOldBrush );
    ::DeleteObject( hBrush );

}

//-----------------------------------------------------------------------------
// Name : PresentFrameBuffer ()
// Desc : We can now render the frame buffer to the final output device
//-----------------------------------------------------------------------------
void CGameApp::PresentFrameBuffer( )
{    
    HDC hDC = NULL; 

    // Retrieve the DC of the window
    hDC = ::GetDC(m_hWnd);

    // Blit the frame buffer to the screen
    ::BitBlt( hDC, m_nViewX, m_nViewY, m_nViewWidth, m_nViewHeight, m_hdcFrameBuffer,
              m_nViewX, m_nViewY, SRCCOPY );

    // Clean up
    ::ReleaseDC( m_hWnd, hDC );

}

//-----------------------------------------------------------------------------
// Name : DrawLine () (Private)
// Desc : Simple line drawing using GDI for clarity.
//-----------------------------------------------------------------------------
void CGameApp::DrawLine( const D3DXVECTOR3 & vtx1, const D3DXVECTOR3 & vtx2, ULONG Color )
{
    LOGPEN logPen;
    HPEN   hPen = NULL, hOldPen = NULL;
    
    // Set up the rendering pen
    logPen.lopnStyle   = PS_SOLID;
    logPen.lopnWidth.x = 1;
    logPen.lopnWidth.y = 1;

    // Set up the colour, converted to BGR & stripped of alpha
    logPen.lopnColor   = 0x00FFFFFF & RGB2BGR( Color ); 
    
    // Create the rendering pen
    hPen = ::CreatePenIndirect( &logPen );
    if (!hPen) return;

    // Select into the frame buffer DC
    hOldPen = (HPEN)::SelectObject( m_hdcFrameBuffer, hPen );

    // Draw the line segment
    MoveToEx( m_hdcFrameBuffer, (long)vtx1.x, (long)vtx1.y, NULL );
    LineTo( m_hdcFrameBuffer, (long)vtx2.x, (long)vtx2.y );

    // Destroy rendering pen
    ::SelectObject( m_hdcFrameBuffer, hOldPen );
    ::DeleteObject( hPen );
    
}

//-----------------------------------------------------------------------------
// Name : SetupGameState ()
// Desc : Sets up all the initial states required by the game.
//-----------------------------------------------------------------------------
void CGameApp::SetupGameState()
{
    float      fAspect;
    
    // Setup Default Matrix Values
    D3DXMatrixIdentity( &m_mtxView );
    
    // Generate our screen aspect ratio
    fAspect = (float)m_nViewWidth / (float)m_nViewHeight;

    // Set up a perspective projection matrix
    D3DXMatrixPerspectiveFovLH( &m_mtxProjection, D3DXToRadian( 60.0f ), fAspect, 1.01f, 1000.0f );
    
    // Enable rotation
    m_bRotation1 = true;
    m_bRotation2 = true;

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
    // Destroy the frame buffer and associated DC's
    if ( m_hdcFrameBuffer && m_hbmFrameBuffer )
    {
        ::SelectObject( m_hdcFrameBuffer, m_hbmSelectOut );
        ::DeleteObject( m_hbmFrameBuffer );
        ::DeleteDC( m_hdcFrameBuffer );

    } // End if

    // Destroy the render window
    if ( m_hWnd ) DestroyWindow( m_hWnd );
    
    // Clear all variables
    m_hWnd              = NULL;
    m_hbmFrameBuffer    = NULL;
    m_hdcFrameBuffer    = NULL;
    
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
    float fAspect;

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

            // Store new viewport sizes
            m_nViewWidth  = LOWORD( lParam );
            m_nViewHeight = HIWORD( lParam );
        
            // Set up new perspective projection matrix
            fAspect = (float)m_nViewWidth / (float)m_nViewHeight;
            D3DXMatrixPerspectiveFovLH( &m_mtxProjection, D3DXToRadian( 60.0f ), fAspect, 1.01f, 1000.0f );

            // Rebuild the new frame buffer
            BuildFrameBuffer( m_nViewWidth, m_nViewHeight );

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

    // Add 6 polygons to this mesh.
    if ( m_Mesh.AddPolygon( 6 ) < 0 ) return false;

    // Front Face
    pPoly = m_Mesh.m_pPolygon[0];
    if ( pPoly->AddVertex( 4 ) < 0 ) return false;

    pPoly->m_pVertex[0] = CVertex( -2,  2, -2 );
    pPoly->m_pVertex[1] = CVertex(  2,  2, -2 );
    pPoly->m_pVertex[2] = CVertex(  2, -2, -2 );
    pPoly->m_pVertex[3] = CVertex( -2, -2, -2 );
    
    // Top Face
    pPoly = m_Mesh.m_pPolygon[1];
    if ( pPoly->AddVertex( 4 ) < 0 ) return false;
    
    pPoly->m_pVertex[0] = CVertex( -2,  2,  2 );
    pPoly->m_pVertex[1] = CVertex(  2,  2,  2 );
    pPoly->m_pVertex[2] = CVertex(  2,  2, -2 );
    pPoly->m_pVertex[3] = CVertex( -2,  2, -2 );

    // Back Face
    pPoly = m_Mesh.m_pPolygon[2];
    if ( pPoly->AddVertex( 4 ) < 0 ) return false;

    pPoly->m_pVertex[0] = CVertex( -2, -2,  2 );
    pPoly->m_pVertex[1] = CVertex(  2, -2,  2 );
    pPoly->m_pVertex[2] = CVertex(  2,  2,  2 );
    pPoly->m_pVertex[3] = CVertex( -2,  2,  2 ),

    // Bottom Face
    pPoly = m_Mesh.m_pPolygon[3];
    if ( pPoly->AddVertex( 4 ) < 0 ) return false;

    pPoly->m_pVertex[0] = CVertex( -2, -2, -2 );
    pPoly->m_pVertex[1] = CVertex(  2, -2, -2 );
    pPoly->m_pVertex[2] = CVertex(  2, -2,  2 );
    pPoly->m_pVertex[3] = CVertex( -2, -2,  2 );

    // Left Face
    pPoly = m_Mesh.m_pPolygon[4];
    if ( pPoly->AddVertex( 4 ) < 0 ) return false;

    pPoly->m_pVertex[0] = CVertex( -2,  2,  2 );
    pPoly->m_pVertex[1] = CVertex( -2,  2, -2 );
    pPoly->m_pVertex[2] = CVertex( -2, -2, -2 );
    pPoly->m_pVertex[3] = CVertex( -2, -2,  2 );

    // Right Face
    pPoly = m_Mesh.m_pPolygon[5];
    if ( pPoly->AddVertex( 4 ) < 0 ) return false;

    pPoly->m_pVertex[0] = CVertex(  2,  2, -2 );
    pPoly->m_pVertex[1] = CVertex(  2,  2,  2 ); 
    pPoly->m_pVertex[2] = CVertex(  2, -2,  2 );
    pPoly->m_pVertex[3] = CVertex(  2, -2, -2 );

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
    TCHAR       lpszFPS[30];

    // Advance the timer
    m_Timer.Tick( 60.0f );
    
    // Animate the two objects
    AnimateObjects();

    // Clear the frame buffer ready for drawing
    ClearFrameBuffer( 0x00FFFFFF );
    
    // Loop through each object
    for ( ULONG i = 0; i < 2; i++ )
    {
        // Store mesh for easy access
        pMesh = m_pObject[i].m_pMesh;

        // Loop through each polygon
        for ( ULONG f = 0; f < pMesh->m_nPolygonCount; f++ )
        {
            // Render the primitive
            DrawPrimitive( pMesh->m_pPolygon[f], &m_pObject[i].m_mtxWorld );
    
        } // Next Polygon
    
    } // Next Object

    // Display Frame Rate
    m_Timer.GetFrameRate( lpszFPS );
    TextOut( m_hdcFrameBuffer, 5, 5, lpszFPS, strlen( lpszFPS ) ); 
    
    // Present the buffer
    PresentFrameBuffer();

}

//-----------------------------------------------------------------------------
// Name : DrawPrimitive () (Private)
// Desc : This function renders an individual polygon.
//-----------------------------------------------------------------------------
void CGameApp::DrawPrimitive( CPolygon * pPoly, D3DXMATRIX * pmtxWorld )
{
    D3DXVECTOR3 vtxPrevious, vtxCurrent;

    // Loop round each vertex transforming as we go
    for ( USHORT v = 0; v < pPoly->m_nVertexCount + 1; v++ ) 
    {
        // Store the current vertex
        vtxCurrent = (D3DXVECTOR3&)pPoly->m_pVertex[ v % pPoly->m_nVertexCount ];

        // Multiply the vertex position by the World / object matrix
        D3DXVec3TransformCoord( &vtxCurrent, &vtxCurrent, pmtxWorld );

        // Multiply by View matrix
        D3DXVec3TransformCoord( &vtxCurrent, &vtxCurrent, &m_mtxView );

        // Multiply by Projection matrix
        D3DXVec3TransformCoord( &vtxCurrent, &vtxCurrent, &m_mtxProjection );

        // Convert to screen space coordinates
        vtxCurrent.x =   vtxCurrent.x * m_nViewWidth  / 2 + m_nViewX + m_nViewWidth  / 2;
        vtxCurrent.y =  -vtxCurrent.y * m_nViewHeight / 2 + m_nViewY + m_nViewHeight / 2;

        // If this is the first vertex, continue. This is the first point of our first line.
        if ( v == 0 ) { vtxPrevious = vtxCurrent; continue; }

        // Draw the line
        DrawLine( vtxPrevious, vtxCurrent, 0 );

        // Store this as new line's first point
        vtxPrevious = vtxCurrent; 

    } // Next Vertex
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