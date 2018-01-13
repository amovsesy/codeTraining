//-----------------------------------------------------------------------------
// File: CTerrain.h
//
// Desc: This class loads, builds & stores the individual terrain block meshes
//       and essentially wraps the rendering loop required.
//
// Copyright (c) 1997-2002 Daedalus Developments. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _CTERRAIN_H_
#define _CTERRAIN_H_

//-----------------------------------------------------------------------------
// CTerrain Specific Includes
//-----------------------------------------------------------------------------
#include "Main.h"
#include "CObject.h"

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class CPlayer;
class CCamera;
class CTerrainBlock;
class CTerrainSplat;
class CTerrainLayer;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CTerrain (Class)
// Desc : Game Timer class, queries performance hardware if available, and 
//        calculates all the various values required for frame rate based
//        vector / value scaling.
//-----------------------------------------------------------------------------
class CTerrain
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class
    //-------------------------------------------------------------------------
	         CTerrain();
	virtual ~CTerrain();

	//-------------------------------------------------------------------------
	// Public Functions For This Class
	//-------------------------------------------------------------------------
    void                SetD3DDevice    ( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL );
    void                SetTextureFormat( const D3DFORMAT & Format, const D3DFORMAT & AlphaFormat );
    bool                LoadTerrain     ( LPCTSTR DefFile );
    float               GetHeight       ( float x, float z, bool ReverseQuad = false );
    void                Render          ( CCamera * pCamera = NULL );
    void                Release         ( );
    float              *GetHeightMap    ( ) const { return m_pHeightMap; }
    D3DXVECTOR3         GetHeightMapNormal  ( ULONG x, ULONG z );
    ULONG               GetTerrainWidth ( ) const { return m_nHeightMapWidth; }
    ULONG               GetTerrainHeight( ) const { return m_nHeightMapHeight; }
    const D3DXVECTOR3&  GetScale        ( ) const { return m_vecScale; }
    LPDIRECT3DDEVICE9   GetD3DDevice    ( ) const { return m_pD3DDevice; }
    bool                UseHardwareTnL  ( ) const { return m_bHardwareTnL; }
    USHORT              GetLayerCount   ( ) const { return m_nLayerCount; }
    CTerrainLayer      *GetLayer        ( USHORT Index ) { return m_pLayer[Index]; }
    USHORT              GetBlendTexRatio( ) const { return m_nBlendTexRatio; }

    //-------------------------------------------------------------------------
	// Public Static Functions For This Class
	//-------------------------------------------------------------------------
    static void     UpdatePlayer  ( LPVOID pContext, CPlayer * pPlayer, float TimeScale );
    static void     UpdateCamera  ( LPVOID pContext, CCamera * pCamera, float TimeScale );

private:
	//-------------------------------------------------------------------------
	// Private Variables For This Class
	//-------------------------------------------------------------------------
    D3DXVECTOR3         m_vecScale;         // Amount to scale the terrain meshes
    float              *m_pHeightMap;       // The physical heightmap data loaded
    ULONG               m_nHeightMapWidth;  // Width of the 2D heightmap data
    ULONG               m_nHeightMapHeight; // Height of the 2D heightmap data

    ULONG               m_nBlockWidth;      // Width of an individual terrain block
    ULONG               m_nBlockHeight;     // Height of an individual terrain block
    ULONG               m_nQuadsWide;       // Stores the number of quads per block
    ULONG               m_nQuadsHigh;       // Stores the number of quads per block
    USHORT              m_nBlocksWide;      // Number of blocks wide
    USHORT              m_nBlocksHigh;      // Number of blocks high
    
    USHORT              m_nBlendTexRatio;   // Number of blend map texels to map to each terrain quad
    
    CTerrainBlock     **m_pBlock;           // Simple array of terrain block pointers
    ULONG               m_nBlockCount;      // Number of terrain blocks stored here
    CTerrainLayer     **m_pLayer;           // Simple array of layer pointers
    USHORT              m_nLayerCount;      // Number of layers stored here

    LPDIRECT3DDEVICE9   m_pD3DDevice;       // D3D Device to use for creation / rendering.
    bool                m_bHardwareTnL;     // Used hardware vertex processing ?
    D3DFORMAT           m_fmtTexture;       // Texture format to use when building textures.
    D3DFORMAT           m_fmtAlphaTexture;  // Texture format to use for building alpha textures

    LPDIRECT3DTEXTURE9* m_pTexture;         // Array of textures loaded for this terrain
    USHORT              m_nTextureCount;    // Number of textures loaded.


	//-------------------------------------------------------------------------
	// Private Functions For This Class
	//-------------------------------------------------------------------------
    long            AddTerrainBlock         ( ULONG Count = 1 );
    long            AddTerrainLayer         ( USHORT Count = 1 );
    bool            GenerateLayers          ( LPCTSTR DefFile );
    bool            GenerateTerrainBlocks   ( );
    void            FilterHeightMap         ( );
    
};

//-----------------------------------------------------------------------------
// Name : CTerrainBlock (Class)
// Desc : An individual block of the terrain used for rendering.
//-----------------------------------------------------------------------------
class CTerrainBlock
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class
    //-------------------------------------------------------------------------
	         CTerrainBlock();
	virtual ~CTerrainBlock();

	//-------------------------------------------------------------------------
	// Public Functions For This Class
	//-------------------------------------------------------------------------
    bool    GenerateBlock ( CTerrain * pParent, ULONG StartX, ULONG StartZ, ULONG BlockWidth, ULONG BlockHeight );
    void    Render        ( LPDIRECT3DDEVICE9 pD3DDevice, USHORT LayerIndex );

	//-------------------------------------------------------------------------
	// Public Variables For This Class
	//-------------------------------------------------------------------------
    ULONG                   m_nStartX;          // X Position in heightmap we start
    ULONG                   m_nStartZ;          // Z Position in heightmap we start
    ULONG                   m_nBlockWidth;      // Width of an individual terrain block
    ULONG                   m_nBlockHeight;     // Height of an individual terrain block
    ULONG                   m_nQuadsWide;       // Number of quads in this block
    ULONG                   m_nQuadsHigh;       // Number of quads in this block
    CTerrain              * m_pParent;          // Parent terrain pointer.
    CTerrainBlock         * m_pNeighbours[9];   // Neighbour block pointers
    USHORT                * m_pLayerUsage;      // Layer usage table
    USHORT                  m_nSplatCount;      // Number of splat levels stored
    CTerrainSplat        ** m_pSplatLevel;      // Actual splat levels stored
    LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;    // Terrain blocks vertex buffer

private:
    
    //-------------------------------------------------------------------------
	// Private Functions For This Class
	//-------------------------------------------------------------------------
    bool    CountLayerUsage     ( );
    bool    GenerateSplats      ( );
    bool    GenerateSplatLevel  ( USHORT TerrainLayer );
    long    AddSplatLevel       ( USHORT Count );
    bool    GenerateBlendMaps   ( );
    
};

//-----------------------------------------------------------------------------
// Name : CTerrainSplat (Class)
// Desc : Stores information about the splat level.
//-----------------------------------------------------------------------------
class CTerrainSplat
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class
    //-------------------------------------------------------------------------
     CTerrainSplat();
    ~CTerrainSplat();

	//-------------------------------------------------------------------------
	// Public Variables For This Class
	//-------------------------------------------------------------------------
    LPDIRECT3DINDEXBUFFER9  m_pIndexBuffer;     // Index buffer for rendering splat
    ULONG                   m_nIndexCount;      // Pre-Calculated Number of indices for rendering 
    ULONG                   m_nPrimitiveCount;  // Pre-calculated number of primitives for rendering
    USHORT                  m_nLayerIndex;      // Layer index used for this splat level
    LPDIRECT3DTEXTURE9      m_pBlendTexture;    // Generated blend texture.
       
};

//-----------------------------------------------------------------------------
// Name : CTerrainLayer (Class)
// Desc : Stores information about an individual terrain layer
//-----------------------------------------------------------------------------
class CTerrainLayer
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class
    //-------------------------------------------------------------------------
     CTerrainLayer();
    ~CTerrainLayer();

    //-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
    UCHAR   GetFilteredAlpha( ULONG x, ULONG z );
	
    //-------------------------------------------------------------------------
	// Public Variables For This Class
	//-------------------------------------------------------------------------
    D3DXMATRIX          m_mtxTexture;       // The texture matrix applied to this layer
    UCHAR              *m_pBlendMap;        // The blend map data for this layer
    ULONG               m_nLayerWidth;      // Width of the layer alpha map
    ULONG               m_nLayerHeight;     // Height of the layer alpha map
    short               m_nTextureIndex;    // Index of the texture to use
       
};

#endif // _CTERRAIN_H_