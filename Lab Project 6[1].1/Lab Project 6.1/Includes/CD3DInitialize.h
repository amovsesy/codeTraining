//-----------------------------------------------------------------------------
// File: CD3DInitialize.h
//
// Desc: Direct3D Enumeration and initialization classes.
//
// Copyright (c) 1997-2002 Adam Hoult & Gary Simmons. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _CD3DINITIALIZE_H_
#define _CD3DINITIALIZE_H_

//-----------------------------------------------------------------------------
// CD3DInitialize Specific Includes
//-----------------------------------------------------------------------------
#include <D3DX9.h>
#include <vector>

//-----------------------------------------------------------------------------
// Name: VertexProcessingType (Enum)
// Desc: Enumeration of all possible D3D vertex processing types.
//-----------------------------------------------------------------------------
enum VERTEXPROCESSING_TYPE
{
    SOFTWARE_VP         = 1,        // Software Vertex Processing
    MIXED_VP            = 2,        // Mixed Vertex Processing
    HARDWARE_VP         = 3,        // Hardware Vertex Processing
    PURE_HARDWARE_VP    = 4         // Pure Hardware Vertex Processing
};

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class CD3DEnumDeviceOptions;
class CD3DEnumDevice;
class CD3DEnumAdapter;

//-----------------------------------------------------------------------------
// STL Vector Typedefs for Easy Access
//-----------------------------------------------------------------------------
typedef std::vector<D3DMULTISAMPLE_TYPE>    VectorMSType;
typedef std::vector<D3DFORMAT>              VectorFormat;
typedef std::vector<ULONG>                  VectorULONG;
typedef std::vector<VERTEXPROCESSING_TYPE>  VectorVPType;
typedef std::vector<D3DDISPLAYMODE>         VectorDisplayMode;
typedef std::vector<CD3DEnumDeviceOptions*> VectorDeviceOptions;
typedef std::vector<CD3DEnumDevice*>        VectorDevice;
typedef std::vector<CD3DEnumAdapter*>       VectorAdapter;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CD3DEnumDeviceOptions (Support Class)
// Desc : Stores the various device options available for any device.
//-----------------------------------------------------------------------------
class CD3DEnumDeviceOptions
{
public:
    ~CD3DEnumDeviceOptions();

    ULONG                   AdapterOrdinal;
    D3DDEVTYPE              DeviceType;
    D3DCAPS9                Caps;
    D3DFORMAT               AdapterFormat;
    D3DFORMAT               BackBufferFormat;
    bool                    Windowed;
    VectorMSType            MultiSampleTypes;
    VectorULONG             MultiSampleQuality;
    VectorFormat            DepthFormats;
    VectorVPType            VertexProcessingTypes;
    VectorULONG             PresentIntervals;
};

//-----------------------------------------------------------------------------
// Name : CD3DEnumDevice (Support Class)
// Desc : Stores the various capabilities etc for an individual device type.
//-----------------------------------------------------------------------------
class CD3DEnumDevice
{
public:
    ~CD3DEnumDevice();

    D3DDEVTYPE              DeviceType;
    D3DCAPS9                Caps;
    VectorDeviceOptions     Options;
};

//-----------------------------------------------------------------------------
// Name : CD3DEnumAdapter (Support Class)
// Desc : Stores the various adapter modes for a single enumerated adapter.
//-----------------------------------------------------------------------------
class CD3DEnumAdapter
{
public:
    ~CD3DEnumAdapter();

    ULONG                   Ordinal;
    D3DADAPTER_IDENTIFIER9  Identifier;
    VectorDisplayMode       Modes;
    VectorDevice            Devices;
};

//-----------------------------------------------------------------------------
// Name : CD3DSettings (Support Class)
// Desc : Allows us to set up the various options we will be using.
// Note : Also used internally by CD3DSettingsDlg.
//-----------------------------------------------------------------------------
class CD3DSettings 
{
public:
    
    struct Settings
    {
        ULONG                   AdapterOrdinal;
        D3DDISPLAYMODE          DisplayMode;
        D3DDEVTYPE              DeviceType;
        D3DFORMAT               BackBufferFormat;
        D3DFORMAT               DepthStencilFormat;
        D3DMULTISAMPLE_TYPE     MultisampleType;
        ULONG                   MultisampleQuality;
        VERTEXPROCESSING_TYPE   VertexProcessingType;
        ULONG                   PresentInterval;
    };

    bool        Windowed;
    Settings    Windowed_Settings;
    Settings    Fullscreen_Settings;

    Settings*   GetSettings() { return (Windowed) ? &Windowed_Settings : &Fullscreen_Settings; }

};

//-----------------------------------------------------------------------------
// Name : CD3DInitialize (Class)
// Desc : Direct3D Initialization class. Detects supported formats, modes and
//        capabilities, and initializes the devices based on the chosen details
//-----------------------------------------------------------------------------
class CD3DInitialize
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
	         CD3DInitialize();
	virtual ~CD3DInitialize();

	//-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
    HRESULT                 Enumerate              ( LPDIRECT3D9 pD3D );
    
    HRESULT                 CreateDisplay          ( CD3DSettings& D3DSettings, ULONG Flags = 0, HWND hWnd = NULL, WNDPROC pWndProc = NULL,
                                                     LPCTSTR Title = NULL, ULONG Width = CW_USEDEFAULT, ULONG Height = CW_USEDEFAULT, 
                                                     LPVOID lParam = NULL );

    HRESULT                 ResetDisplay           ( LPDIRECT3DDEVICE9 pD3DDevice, CD3DSettings& D3DSettings, HWND hWnd = NULL );
    
    bool                    FindBestWindowedMode   ( CD3DSettings & D3DSettings, bool bRequireHAL = false, bool bRequireREF = false );
    bool                    FindBestFullscreenMode ( CD3DSettings & D3DSettings, D3DDISPLAYMODE * pMatchMode = NULL, bool bRequireHAL = false, bool bRequireREF = false );

    D3DPRESENT_PARAMETERS   BuildPresentParameters ( CD3DSettings& D3DSettings, ULONG Flags = 0 );
    
    ULONG                   GetAdapterCount( ) const  { return m_vpAdapters.size(); }
    const CD3DEnumAdapter  *GetAdapter( ULONG Index ) { return (Index > GetAdapterCount() ) ? NULL : m_vpAdapters[Index]; }
    HWND                    GetHWND( )                { return m_hWnd; }
    const LPDIRECT3D9       GetDirect3D( )            { return m_pD3D; }
    LPDIRECT3DDEVICE9       GetDirect3DDevice( );

    
    
private:
    //-------------------------------------------------------------------------
	// Private Functions for This Class
	//-------------------------------------------------------------------------
    HRESULT             EnumerateAdapters               ( );
    HRESULT             EnumerateDisplayModes           ( CD3DEnumAdapter * pAdapter );
    HRESULT             EnumerateDevices                ( CD3DEnumAdapter * pAdapter );
    HRESULT             EnumerateDeviceOptions          ( CD3DEnumDevice  * pDevice, CD3DEnumAdapter * pAdapter );
    HRESULT             EnumerateDepthStencilFormats    ( CD3DEnumDeviceOptions * pDeviceOptions );
    HRESULT             EnumerateMultiSampleTypes       ( CD3DEnumDeviceOptions * pDeviceOptions );
    HRESULT             EnumerateVertexProcessingTypes  ( CD3DEnumDeviceOptions * pDeviceOptions );
    HRESULT             EnumeratePresentIntervals       ( CD3DEnumDeviceOptions * pDeviceOptions );

    //-------------------------------------------------------------------------
	// Private Virtual Functions for This Class
	//-------------------------------------------------------------------------
    virtual bool        ValidateDisplayMode          ( const D3DDISPLAYMODE& Mode )                         { return true; }
    virtual bool        ValidateDevice               ( const D3DDEVTYPE& Type, const D3DCAPS9& Caps )       { return true; }
    virtual bool        ValidateDeviceOptions        ( const D3DFORMAT& BackBufferFormat, bool IsWindowed ) { return true; }
    virtual bool        ValidateDepthStencilFormat   ( const D3DFORMAT& DepthStencilFormat )                { return true; }
    virtual bool        ValidateMultiSampleType      ( const D3DMULTISAMPLE_TYPE& Type )                    { return true; }
    virtual bool        ValidateVertexProcessingType ( const VERTEXPROCESSING_TYPE& Type )                  { return true; }
    virtual bool        ValidatePresentInterval      ( const ULONG& Interval )                              { return true; }
    
    //-------------------------------------------------------------------------
	// Private Variables For This Class
	//-------------------------------------------------------------------------
	LPDIRECT3D9		    m_pD3D;			    // Primary Direct3D Object.
    LPDIRECT3DDEVICE9   m_pD3DDevice;       // Created Direct3D Device.
    HWND                m_hWnd;             // Created window handle
    VectorAdapter       m_vpAdapters;       // Enumerated Adapters
    
};

#endif // _CD3DINITIALIZE_H_