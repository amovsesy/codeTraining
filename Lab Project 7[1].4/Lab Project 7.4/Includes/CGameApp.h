//-----------------------------------------------------------------------------
// File: CGameApp.h
//
// Desc: Game Application class, this is the central hub for all app processing
//
// Copyright (c) 1997-2002 Adam Hoult & Gary Simmons. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _CGAMEAPP_H_
#define _CGAMEAPP_H_

//-----------------------------------------------------------------------------
// CGameApp Specific Includes
//-----------------------------------------------------------------------------
#include "Main.h"
#include "CTimer.h"
#include "CObject.h"
#include "CPlayer.h"
#include "CTerrain.h"
#include "CD3DSettingsDlg.h"

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class CCamera;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CGameApp (Class)
// Desc : Central game engine, initialises the game and handles core processes.
//-----------------------------------------------------------------------------
class CGameApp
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
	         CGameApp();
	virtual ~CGameApp();

	//-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
    LRESULT     DisplayWndProc( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam );
	bool        InitInstance( HANDLE hInstance, LPCTSTR lpCmdLine, int iCmdShow );
    int         BeginGame( );
	bool        ShutDown( );

    //-------------------------------------------------------------------------
	// App State Access Functions
	//-------------------------------------------------------------------------
    const D3DTEXTUREOP & GetColorOp() const { return m_ColorOp; }
    bool                 GetRenderLayer( UCHAR Index ) const { return (Index < 3) ? m_bRenderLayers[Index] : true; }
	
private:
    //-------------------------------------------------------------------------
	// Private Functions for This Class
	//-------------------------------------------------------------------------
    bool        BuildObjects      ( );
    void        ReleaseObjects    ( );
    void        FrameAdvance      ( );
    bool        CreateDisplay     ( );
    void        ChangeDevice      ( );
    void        SetupGameState    ( );
    void        SetupRenderStates ( );
    void        AnimateObjects    ( );
    void        ProcessInput      ( );
    bool        TestDeviceCaps    ( );
    void        SelectMenuItems   ( );

    bool        BuildSkyBox       ( );
    void        RenderSkyBox      ( );
    
    //-------------------------------------------------------------------------
	// Private Static Functions For This Class
	//-------------------------------------------------------------------------
    static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

    //-------------------------------------------------------------------------
	// Private Variables For This Class
	//-------------------------------------------------------------------------
    CTerrain                m_Terrain;          // Simple terrain object (stores data)
    
    CPlayer                 m_Player;           // Player class used to manipulate our player object
    CCamera               * m_pCamera;          // A cached copy of the camera attached to the player

    D3DXMATRIX              m_mtxIdentity;      // A basic identity matrix
    
    CTimer                  m_Timer;            // Game timer
    ULONG                   m_LastFrameRate;    // Used for making sure we update only when fps changes.
    
    HWND                    m_hWnd;             // Main window HWND
    HICON                   m_hIcon;            // Window Icon
    HMENU                   m_hMenu;            // Window Menu
    
    bool                    m_bLostDevice;      // Is the 3d device currently lost ?
    bool                    m_bActive;          // Is the application active ?

    LPDIRECT3D9             m_pD3D;             // Direct3D Object
    LPDIRECT3DDEVICE9       m_pD3DDevice;       // Direct3D Device Object
    CD3DSettings            m_D3DSettings;      // The settings used to initialize D3D
    
    D3DFILLMODE             m_FillMode;         // Which fill mode are we using ?
    D3DTEXTUREOP            m_ColorOp;          // Which color op are we using?
    D3DTEXTUREFILTERTYPE    m_MagFilter;        // Magnification Filter to use.
    D3DTEXTUREFILTERTYPE    m_MinFilter;        // Minification Filter to use.
    D3DTEXTUREFILTERTYPE    m_MipFilter;        // Mip-Map filter to use.
    ULONG                   m_Anisotropy;       // Anisotropy level to use.

    D3DFORMAT               m_TextureFormat;    // Standard texture format for us to use.
    D3DFORMAT               m_AlphaFormat;      // Alpha texture format for us to use.

    bool                    m_MagFilterCaps[10];// Capabilities supported for our required filters
    bool                    m_MinFilterCaps[10];// Capabilities supported for our required filters
    bool                    m_MipFilterCaps[10];// Capabilities supported for our required mip filters
    ULONG                   m_MaxTextures;      // Capabilities supported for our required pass count
    ULONG                   m_MaxAnisotropy;    // Capabilities supported for anisotropy filter

    ULONG                   m_nViewX;           // X Position of render viewport
    ULONG                   m_nViewY;           // Y Position of render viewport
    ULONG                   m_nViewWidth;       // Width of render viewport
    ULONG                   m_nViewHeight;      // Height of render viewport

    POINT                   m_OldCursorPos;     // Old cursor position for tracking

    CObject                 m_Object;           // The object referencing the player mesh
    CMesh                   m_PlayerMesh;       // The player mesh (cube ;)

    CMesh                   m_SkyMesh;          // The skybox mesh (also a cube)
    LPDIRECT3DTEXTURE9      m_SkyTextures[6];   // The skybox textures

    bool                    m_bRenderLayers[3]; // Enable / Disable layers.

};

//-----------------------------------------------------------------------------
// Name : CMyD3DInit (Subclass)
// Desc : Initialization class, derived from CD3DInitialize to supply any
//        validation of device settings that are deemed applicable by this
//        application.
//-----------------------------------------------------------------------------
class CMyD3DInit : public CD3DInitialize
{
private:
    //-------------------------------------------------------------------------
	// Private Virtual Functions for This Class (CD3DInitialize)
	//-------------------------------------------------------------------------
    virtual bool        ValidateDisplayMode          ( const D3DDISPLAYMODE& Mode );
    virtual bool        ValidateDevice               ( const D3DDEVTYPE& Type, const D3DCAPS9& Caps );
    virtual bool        ValidateVertexProcessingType ( const VERTEXPROCESSING_TYPE& Type );
};

#endif // _CGAMEAPP_H_