//-----------------------------------------------------------------------------
// File: CTerrain.cpp
//
// Desc: This class loads, builds & stores the individual terrain block meshes
//       and essentially wraps the rendering loop required.
//
// Copyright (c) 1997-2002 Daedalus Developments. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CTerrain Specific Includes
//-----------------------------------------------------------------------------
#include "..\\Includes\\CTerrain.h"
#include "..\\Includes\\CObject.h"
#include "..\\Includes\\CPlayer.h"
#include "..\\Includes\\CCamera.h"
#include "..\\Includes\\CGameApp.h"

//-----------------------------------------------------------------------------
// Modulate Local Constants
//-----------------------------------------------------------------------------
namespace
{
    const char DataPath[] = "Data\\";               // The path to the data files.
};

//-----------------------------------------------------------------------------
// Name : CTerrain () (Constructor)
// Desc : CTerrain Class Constructor
//-----------------------------------------------------------------------------
CTerrain::CTerrain()
{
    // Reset all required values
    m_pD3DDevice        = NULL;

    m_pHeightMap        = NULL;
    m_nHeightMapWidth   = 0;
    m_nHeightMapHeight  = 0;
    m_nBlockWidth       = 0;
    m_nBlockHeight      = 0;
    m_nQuadsWide        = 0;
    m_nQuadsHigh        = 0;
    m_nLayerCount       = 0;
    m_nBlendTexRatio    = 1;
    
    m_pBlock            = NULL;
    m_nBlockCount       = 0;
    m_pLayer            = NULL;
    m_nLayerCount       = 0;
    m_pTexture          = NULL;
    m_nTextureCount     = 0;

    m_vecScale          = D3DXVECTOR3( 1.0f, 1.0f, 1.0f );

}

//-----------------------------------------------------------------------------
// Name : CTerrain () (Destructor)
// Desc : CTerrain Class Destructor
//-----------------------------------------------------------------------------
CTerrain::~CTerrain()
{
    // Release any allocated memory
    Release();
}

//-----------------------------------------------------------------------------
// Name : Release ()
// Desc : Allows us to release any allocated memory and reuse.
//-----------------------------------------------------------------------------
void CTerrain::Release()
{
    // Release Heightmap
    if ( m_pHeightMap ) delete[]m_pHeightMap;
    
    // Release Blocks
    if ( m_pBlock ) 
    {
        // Delete all individual blocks in the array.
        for ( ULONG i = 0; i < m_nBlockCount; i++ )
        {
            if ( m_pBlock[i] ) delete m_pBlock[i];
        
        } // Next Mesh

        // Free up the array itself
        delete []m_pBlock;
    
    } // End if

    // Release Layers
    if ( m_pLayer ) 
    {
        // Delete all individual layers in the array.
        for ( ULONG i = 0; i < m_nLayerCount; i++ )
        {
            if ( m_pLayer[i] ) delete m_pLayer[i];
        
        } // Next Layer

        // Free up the array itself
        delete []m_pLayer;
    
    } // End if

    // Release Textures
    if ( m_pTexture ) 
    {
        // Release all individual textures in the array.
        for ( ULONG i = 0; i < m_nTextureCount; i++ )
        {
            if ( m_pTexture[i] ) m_pTexture[i]->Release();
        
        } // Next Texture

        // Free up the array itself
        delete []m_pTexture;
    
    } // End if

    // Release our D3D Object ownership
    if ( m_pD3DDevice     ) m_pD3DDevice->Release();

    // Clear Variables
    m_pD3DDevice        = NULL;
    m_pHeightMap        = NULL;
    m_nHeightMapWidth   = 0;
    m_nHeightMapHeight  = 0;
    m_nBlockWidth       = 0;
    m_nBlockHeight      = 0;
    m_nQuadsWide        = 0;
    m_nQuadsHigh        = 0;
    m_nLayerCount       = 0;
    m_nBlendTexRatio    = 1;

    m_pBlock            = NULL;
    m_nBlockCount       = 0;
    m_pLayer            = NULL;
    m_nLayerCount       = 0;
    m_pTexture          = NULL;
    m_nTextureCount     = 0;
    
}

//-----------------------------------------------------------------------------
// Name : SetTextureFormat()
// Desc : Informs our scene manager with which format standard textures should
//        be created.
//-----------------------------------------------------------------------------
void CTerrain::SetTextureFormat( const D3DFORMAT & Format, const D3DFORMAT & AlphaFormat )
{
    // Store texture format
    m_fmtTexture      = Format;
    m_fmtAlphaTexture = AlphaFormat;
}

//-----------------------------------------------------------------------------
// Name : LoadTerrain ()
// Desc : Load the terrain definition file and generate the terrain data.
// Note : For GetPrivateProfileString to work correctly, 'DefFile' must contain
//        the FULL path to the terrain definition file.
//-----------------------------------------------------------------------------
bool CTerrain::LoadTerrain( LPCTSTR DefFile )
{
    FILE  * pFile = NULL;
    char    Buffer  [1025], Section [100], Value[100], FileName[MAX_PATH];
    ULONG   i;

    // Cannot load if already allocated (must be explicitly released for reuse)
    if ( m_pBlock ) return false;

    // Must have an already set D3D Device
    if ( !m_pD3DDevice ) return false;
    
    // Read in the terrain definition values specified by the file
    strcpy( Section, "General" );
    GetPrivateProfileString( Section, "Heightmap", "", FileName, MAX_PATH - 1, DefFile );
    GetPrivateProfileString( Section, "Scale", "1, 1, 1", Buffer, 1024, DefFile );
    sscanf( Buffer, "%g,%g,%g", &m_vecScale.x, &m_vecScale.y, &m_vecScale.z );
    GetPrivateProfileString( Section, "TerrainSize", "257, 257", Buffer, 1024, DefFile );
    sscanf( Buffer, "%i,%i", &m_nHeightMapWidth, &m_nHeightMapHeight );
    GetPrivateProfileString( Section, "BlockSize", "17, 17", Buffer, 1024, DefFile );
    sscanf( Buffer, "%i,%i", &m_nBlockWidth, &m_nBlockHeight );
    GetPrivateProfileString( Section, "BlendTexRatio", "1", Buffer, 1024, DefFile );
    sscanf( Buffer, "%i", &m_nBlendTexRatio );

    // Store secondary data
    m_nQuadsWide = m_nBlockWidth - 1;
    m_nQuadsHigh = m_nBlockHeight - 1;

    // Attempt to allocate space for this heightmap information
    m_pHeightMap = new float[m_nHeightMapWidth * m_nHeightMapHeight];
    if (!m_pHeightMap) return false;

    // Build the heightmap path / filename
    strcpy( Buffer, DataPath );
    strcat( Buffer, FileName );

    // Open up the heightmap file
    pFile = _tfopen( Buffer, _T("rb") );
    if (!pFile) return false;

    // Read the heightmap data
    for ( i = 0; i < m_nHeightMapWidth * m_nHeightMapHeight; i++ )
    {
        UCHAR HeightValue;
        fread( &HeightValue, 1, 1, pFile );

        // Store it as floating point
        m_pHeightMap[i] = (float)HeightValue;
    
    } // Next Value
    
    // Finish up
    fclose( pFile );

    // Filter the heightmap data
    FilterHeightMap();

    // Load in the texture data
    strcpy( Section, "Textures" );
    m_nTextureCount = GetPrivateProfileInt( Section, "TextureCount", 0,DefFile );
    if ( m_nTextureCount > 0 )
    {
        // Allocate space for specified textures
        m_pTexture = new LPDIRECT3DTEXTURE9[ m_nTextureCount ];
        if ( !m_pTexture ) return false;

        // Loop through and read in texture filenames etc.
        for ( i = 0; i < m_nTextureCount; ++i )
        {
            // Build the value string
            sprintf( Value, "Texture[%i]", i );

            // Retrieve the filename
            GetPrivateProfileString( Section, Value, "", FileName, MAX_PATH - 1, DefFile );

            // Build the texture path / filename
            strcpy( Buffer, DataPath );
            strcat( Buffer, FileName );

            // Load it in (Ignore failure, it's not fatal)
            D3DXCreateTextureFromFileEx( m_pD3DDevice, Buffer, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
                                         0, m_fmtTexture, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
                                         0, NULL, NULL, &m_pTexture[i] );

        } // Next Texture

    } // If any textures

    // Generate the terrain layer data
    if ( !GenerateLayers( DefFile ) ) return false;

    // Build the terrain blocks
    if ( !GenerateTerrainBlocks() ) return false;

    // Erase the blend maps, they are no longer required
    for ( i = 0; i < m_nLayerCount; i++ ) 
    {
        if ( m_pLayer[i]->m_pBlendMap ) { delete []m_pLayer[i]->m_pBlendMap; m_pLayer[i]->m_pBlendMap = NULL; }    
    
    } // Next Layer

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : GenerateLayers()
// Desc : Generate the layer data for this terrain.
//-----------------------------------------------------------------------------
bool CTerrain::GenerateLayers( LPCTSTR DefFile )
{
    ULONG Width  = (m_nHeightMapWidth  - 1) * m_nBlendTexRatio;
    ULONG Height = (m_nHeightMapHeight - 1) * m_nBlendTexRatio;
    char  Buffer  [1025], FileName[MAX_PATH], Section [100];
    ULONG i, j, x, z, LayerCount;
    float Angle;
    UCHAR Value;
    D3DXVECTOR2 Scale;

    HRESULT             hRet;
    D3DXIMAGE_INFO      Info;
    LPDIRECT3DSURFACE9  pSurface = NULL;

    // Read in the terrain layer data
    strcpy( Section, "General" );
    LayerCount = GetPrivateProfileInt( Section, "LayerCount", 1, DefFile );
    
    // Allocate our layer data
    if ( AddTerrainLayer( LayerCount ) < 0 ) return false;
    
    // Read in the element data
    for ( i = 0; i < m_nLayerCount; i++ )
    {
        CTerrainLayer * pLayer = m_pLayer[i];

        // Build section string
        if ( i == 0 )
            strcpy( Section, "Base Layer" );
        else
            sprintf( Section, "Layer %i", i );

        // Store layer properties
        pLayer->m_nTextureIndex = (short)GetPrivateProfileInt( Section, "TextureIndex", 0, DefFile );
        pLayer->m_nLayerWidth   = Width;
        pLayer->m_nLayerHeight  = Height;    
    
        // Calculate layer texture matrix
        GetPrivateProfileString( Section, "Translation", "0.0, 0.0", Buffer, 1024, DefFile );
        sscanf( Buffer, "%g,%g", &pLayer->m_mtxTexture._31, &pLayer->m_mtxTexture._32 );
        GetPrivateProfileString( Section, "Rotation", "0.0", Buffer, 1024, DefFile );
        sscanf( Buffer, "%g", &Angle );

        // Rotate the texture matrix
        if ( Angle != 0.0f )
        {
            Angle = D3DXToRadian( Angle );
            pLayer->m_mtxTexture._11 =  cosf(Angle); pLayer->m_mtxTexture._12 = sinf(Angle);
            pLayer->m_mtxTexture._21 = -sinf(Angle); pLayer->m_mtxTexture._22 = cosf(Angle);
        
        } // End if apply any rotation

        // Scale values
        GetPrivateProfileString( Section, "Scale", "1.0, 1.0", Buffer, 1024, DefFile );
        sscanf( Buffer, "%g,%g", &Scale.x, &Scale.y );
        pLayer->m_mtxTexture._11 *= Scale.x; pLayer->m_mtxTexture._21 *= Scale.x; pLayer->m_mtxTexture._31 *= Scale.x;
        pLayer->m_mtxTexture._12 *= Scale.y; pLayer->m_mtxTexture._22 *= Scale.y; pLayer->m_mtxTexture._32 *= Scale.y;
        
        // Allocate our layer blend map array (these are temporary arrays)
        pLayer->m_pBlendMap = new UCHAR[ Width * Height ];
        if (!pLayer->m_pBlendMap) return false;

        // Set the blend map data to full transparency for now
        memset( pLayer->m_pBlendMap, 0, Width * Height );

        // Base layer is always fully opaque
        if  ( i == 0 ) { memset( pLayer->m_pBlendMap, 255, Width * Height ); continue; }
        
        // Get layer filename for non base layers
        GetPrivateProfileString( Section, "LayerMap", "", FileName, MAX_PATH - 1, DefFile );

        // Build the layer map path / filename
        strcpy( Buffer, DataPath );
        strcat( Buffer, FileName );

        // Get the source file info
        if ( FAILED(D3DXGetImageInfoFromFile( Buffer, &Info ) )) return false;
        
        // Create the off screen surface in sys mem, in a format useful to us
        hRet = m_pD3DDevice->CreateOffscreenPlainSurface( Info.Width, Info.Height, D3DFMT_X8R8G8B8, 
                                                          D3DPOOL_SYSTEMMEM, &pSurface, NULL  );
        if ( FAILED(hRet) ) return false;

        // Load in the image
        hRet = D3DXLoadSurfaceFromFile( pSurface, NULL, NULL, Buffer, NULL, D3DX_DEFAULT, 0, NULL );
        if ( FAILED(hRet) ) { pSurface->Release(); return false; }
        
        // Lock the surface and copy over the data into our blend map array
        D3DLOCKED_RECT LockedRect;
        hRet = pSurface->LockRect( &LockedRect, NULL, D3DLOCK_READONLY );
        if ( FAILED(hRet) ) { pSurface->Release(); return false; }

        ULONG * pBits = (ULONG*)LockedRect.pBits;
        
        // Loop through each row
        for ( z = 0; z < Info.Height; ++z )
        {
            // Loop through each column and extract just the blue pixel data
            for ( x = 0; x < Info.Width; ++x ) pLayer->m_pBlendMap[ x + z * Width ] = (UCHAR)(pBits[x] & (0x000000FF));
            
            // Move to the next row
            pBits += LockedRect.Pitch / 4;

        } // Next row

        // Unlock & release the surface, we have decoded it now
        pSurface->UnlockRect();
        pSurface->Release();

        // Clamp values to min and max
        for ( j = 0; j < (Width * Height); j++ )
        {
            UCHAR MinAlpha = 15;
            UCHAR MaxAlpha = 220;
            
            // Clamp layer value
            Value = pLayer->m_pBlendMap[ j ];
            if ( Value < MinAlpha ) Value = 0;
            if ( Value > MaxAlpha ) Value = 255;
            pLayer->m_pBlendMap[ j ] = Value;

        } // Next Alpha Value

    } // Next Layer

    // Now we need to parse the layers and determine which alpha pixels are occluded
    for ( i = 0; i < m_nLayerCount; i++ )
    {
        CTerrainLayer * pLayer = m_pLayer[i];

        for ( z = 0; z < Height; z++ )
        {
            for ( x = 0; x < Width; x++ )
            {
                // Determine if we need to test occlusion
                Value = pLayer->m_pBlendMap[ x + z * Width ];

                if ( Value > 0 )
                {
                    // Is this obscured by any layers above ?
                    Value = 0;
                    for ( j = i + 1; j < m_nLayerCount; j++ )
                    {
                        Value = m_pLayer[j]->GetFilteredAlpha( x, z );
                        if (Value == 255) break;

                    } // Next Layer

                    // Layer is obscred if a layer above is opaque
                    if (Value == 255) pLayer->m_pBlendMap[ x + z * Width ] = 0;

                } // End if Test Occlusion

            } // Next Column

        } // Next Row

    } // Next Layer

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : GenerateTerrainBlocks()
// Desc : Generate each of the individual blocks required.
//-----------------------------------------------------------------------------
bool CTerrain::GenerateTerrainBlocks( )
{
    ULONG x, z, ax, az, Counter;

    // Calculate block values
    m_nBlocksWide = (USHORT)(m_nHeightMapWidth - 1) / m_nQuadsWide;
    m_nBlocksHigh = (USHORT)(m_nHeightMapHeight - 1) / m_nQuadsHigh;
    
    // Allocate enough blocks to store the separate parts of this terrain
    if ( AddTerrainBlock(  m_nBlocksWide * m_nBlocksHigh )  < 0 ) return false;

    // Initialize each terrain block
    for ( z = 0; z < m_nBlocksHigh; z++ )
    {
        for ( x = 0; x < m_nBlocksHigh; x++ )
        {
            CTerrainBlock * pBlock = m_pBlock[ x + z * m_nBlocksWide ];

            // Calculate Neighbour Information
            Counter = 0;
            for ( az = -1; az <= 1; az++ )
            {
                for ( ax = -1; ax <= 1; ax++, Counter++ )
                {
                    // Reset to NULL to begin with
                    pBlock->m_pNeighbours[Counter] = NULL;
                    
                    // Bail if we are out of bounds
                    if ( (x + ax) < 0 || (z + az) < 0 || (x + ax) >= m_nBlocksWide || (z + az) >= m_nBlocksHigh ) continue;
                
                    // Store Neighbour
                    pBlock->m_pNeighbours[Counter] = m_pBlock[ (x + ax) + (z + az) * m_nBlocksWide ]; 

                } // Next Adjacent Column

            } // Next Adjacent Row
            
        } // Next Column
    
    } // Next Row

    // Generate each terrain block
    for ( z = 0; z < m_nBlocksHigh; z++ )
    {
        for ( x = 0; x < m_nBlocksHigh; x++ )
        {
            CTerrainBlock * pBlock = m_pBlock[ x + z * m_nBlocksWide ];
            
            // Generate the block
            if (!pBlock->GenerateBlock( this, x * m_nQuadsWide, z * m_nQuadsHigh, m_nBlockWidth, m_nBlockHeight )) return false;

        } // Next Column
    
    } // Next Row

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : FilterHeightMap ()
// Desc : Filter the heightmap to smooth out those bumps.
//-----------------------------------------------------------------------------
void CTerrain::FilterHeightMap( )
{
    ULONG x, z;
    float Value;

    // Validate requirements
    if (!m_pHeightMap) return;
    
    // Allocate the result
    float * pResult = new float[m_nHeightMapWidth * m_nHeightMapHeight];
    if (!pResult) return;

    // Copy over data to retain edges
    memcpy( pResult, m_pHeightMap, m_nHeightMapWidth * m_nHeightMapHeight * sizeof(float) );

    // Loop through and filter values (simple box style filter)
    for ( z = 1; z < m_nHeightMapHeight - 1; ++z )
    {
        for ( x = 1; x < m_nHeightMapWidth - 1; ++x )
        {
            Value  = m_pHeightMap[ (x - 1) + (z - 1) * m_nHeightMapWidth ]; 
            Value += m_pHeightMap[ (x    ) + (z - 1) * m_nHeightMapWidth ]; 
            Value += m_pHeightMap[ (x + 1) + (z - 1) * m_nHeightMapWidth ]; 

            Value += m_pHeightMap[ (x - 1) + (z    ) * m_nHeightMapWidth ]; 
            Value += m_pHeightMap[ (x    ) + (z    ) * m_nHeightMapWidth ]; 
            Value += m_pHeightMap[ (x + 1) + (z    ) * m_nHeightMapWidth ]; 

            Value += m_pHeightMap[ (x - 1) + (z + 1) * m_nHeightMapWidth ]; 
            Value += m_pHeightMap[ (x    ) + (z + 1) * m_nHeightMapWidth ]; 
            Value += m_pHeightMap[ (x + 1) + (z + 1) * m_nHeightMapWidth ]; 

            // Store the result
            pResult[ x + z * m_nHeightMapWidth ] = Value / 9.0f;

        } // Next X

    } // Next Z

    // Release the old array
    delete []m_pHeightMap;

    // Store the new one
    m_pHeightMap = pResult;

}

//-----------------------------------------------------------------------------
// Name : GetHeightMapNormal ()
// Desc : Retrieves the normal at this position in the heightmap
//-----------------------------------------------------------------------------
D3DXVECTOR3 CTerrain::GetHeightMapNormal( ULONG x, ULONG z )
{
	D3DXVECTOR3 Normal, Edge1, Edge2;
	ULONG       HMIndex, HMAddX, HMAddZ;
    float       y1, y2, y3;

	// Make sure we are not out of bounds
	if ( x < 0.0f || z < 0.0f || x >= m_nHeightMapWidth || z >= m_nHeightMapHeight ) return D3DXVECTOR3(0.0f, 1.0f, 0.0f);

    // Calculate the index in the heightmap array
    HMIndex = x + z * m_nHeightMapWidth;
    
    // Calculate the number of pixels to add in either direction to
    // obtain the best neighbouring heightmap pixel.
    if ( x < (m_nHeightMapWidth - 1))  HMAddX = 1; else HMAddX = -1;
	if ( z < (m_nHeightMapHeight - 1)) HMAddZ = m_nHeightMapWidth; else HMAddZ = -(signed)m_nHeightMapWidth;
	
    // Get the three height values
	y1 = m_pHeightMap[HMIndex] * m_vecScale.y;
	y2 = m_pHeightMap[HMIndex + HMAddX] * m_vecScale.y; 
	y3 = m_pHeightMap[HMIndex + HMAddZ] * m_vecScale.y;
			
	// Calculate Edges
	Edge1 = D3DXVECTOR3( 0.0f, y3 - y1, m_vecScale.z );
	Edge2 = D3DXVECTOR3( m_vecScale.x, y2 - y1, 0.0f );
			
	// Calculate Resulting Normal
	D3DXVec3Cross( &Normal, &Edge1, &Edge2);
	D3DXVec3Normalize( &Normal, &Normal );
	
    // Return it.
	return Normal;
}

//-----------------------------------------------------------------------------
// Name : GetHeight ()
// Desc : Retrieves the height at the given world space location
// Note : Pass in true to the 'ReverseQuad' parameter to reverse the direction
//        in which the quads dividing edge is based (Normally Top Right to
//        bottom left assuming pixel space)
//-----------------------------------------------------------------------------
float CTerrain::GetHeight( float x, float z, bool ReverseQuad )
{
    float fTopLeft, fTopRight, fBottomLeft, fBottomRight;

    // Adjust Input Values
    x = x / m_vecScale.x;
    z = z / m_vecScale.z;

    // Make sure we are not OOB
    if ( x < 0.0f || z < 0.0f || x >= m_nHeightMapWidth || z >= m_nHeightMapHeight ) return 0.0f;	

    // First retrieve the Heightmap Points
    int ix = (int)x;
    int iz = (int)z;
	
    // Calculate the remainder (percent across quad)
    float fPercentX = x - ((float)ix);
    float fPercentZ = z - ((float)iz);

    if ( ReverseQuad )
    {
        // First retrieve the height of each point in the dividing edge
        fTopLeft     = m_pHeightMap[ix + iz * m_nHeightMapWidth] * m_vecScale.y;
        fBottomRight = m_pHeightMap[(ix + 1) + (iz + 1) * m_nHeightMapWidth] * m_vecScale.y;

        // Which triangle of the quad are we in ?
        if ( fPercentX < fPercentZ )
        {
            fBottomLeft = m_pHeightMap[ix + (iz + 1) * m_nHeightMapWidth] * m_vecScale.y;
		    fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
        
        } // End if Left Triangle
        else
        {
            fTopRight   = m_pHeightMap[(ix + 1) + iz * m_nHeightMapWidth] * m_vecScale.y;
		    fBottomLeft = fTopLeft + (fBottomRight - fTopRight);

        } // End if Right Triangle
    
    } // End if Quad is reversed
    else
    {
        // First retrieve the height of each point in the dividing edge
        fTopRight   = m_pHeightMap[(ix + 1) + iz * m_nHeightMapWidth] * m_vecScale.y;
        fBottomLeft = m_pHeightMap[ix + (iz + 1) * m_nHeightMapWidth] * m_vecScale.y;

        // Calculate which triangle of the quad are we in ?
        if ( fPercentX < (1.0f - fPercentZ)) 
        {
            fTopLeft = m_pHeightMap[ix + iz * m_nHeightMapWidth] * m_vecScale.y;
            fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
        
        } // End if Left Triangle
        else
        {
            fBottomRight = m_pHeightMap[(ix + 1) + (iz + 1) * m_nHeightMapWidth] * m_vecScale.y;
            fTopLeft = fTopRight + (fBottomLeft - fBottomRight);

        } // End if Right Triangle
    
    } // End if Quad is not reversed
    
    // Calculate the height interpolated across the top and bottom edges
    float fTopHeight    = fTopLeft    + ((fTopRight - fTopLeft) * fPercentX );
    float fBottomHeight = fBottomLeft + ((fBottomRight - fBottomLeft) * fPercentX );

    // Calculate the resulting height interpolated between the two heights
    return fTopHeight + ((fBottomHeight - fTopHeight) * fPercentZ );
}

//-----------------------------------------------------------------------------
// Name : Render()
// Desc : Renders all of the meshes stored within this terrain object.
//-----------------------------------------------------------------------------
void CTerrain::Render( CCamera * pCamera )
{
    USHORT i;
    ULONG  j;
    
    // Validate parameters
    if( !m_pD3DDevice ) return;

    // Setup our terrain render states
    m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, true );
    m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    // stage 0 coloring : get color from texture0*diffuse
    m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    // stage 0 alpha : nada
    m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
    m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
    
    // stage 1 coloring : nada
    m_pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
    m_pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
    
    // stage 1 alpha : get alpha from texture1
    m_pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
    m_pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

    // Enable Stage Texture Transforms
    m_pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
    
    // Setup our terrain vertex FVF code
    m_pD3DDevice->SetFVF( VERTEX_FVF );

    // Loop through blocks and signal a render
    for ( j = 0; j < m_nBlockCount; j++ )
    {
        // Skip if block is not within the viewing frustum
        if ( pCamera && (!pCamera->BoundsInFrustum( m_pBlock[j]->m_BoundsMin, m_pBlock[j]->m_BoundsMax )) ) continue;

        m_pD3DDevice->SetStreamSource( 0, m_pBlock[j]->m_pVertexBuffer, 0, sizeof(CVertex) );

        // Loop through all active layers
        for ( i = 0; i < m_nLayerCount; i++ )
        {
            // Skip if this layer is disabled
            if ( GetGameApp()->GetRenderLayer( i ) == false ) continue;

            CTerrainLayer * pLayer = m_pLayer[i];
            if ( !m_pBlock[j]->m_pLayerUsage[ i ] ) continue;

            // Set our texturing information
            m_pD3DDevice->SetTexture( 0, m_pTexture[pLayer->m_nTextureIndex] );
            m_pD3DDevice->SetTransform( D3DTS_TEXTURE0, &pLayer->m_mtxTexture );
            
            m_pBlock[j]->Render( m_pD3DDevice, i );

        } // Next Block

    } // Next Layer

}

//-----------------------------------------------------------------------------
// Name : AddTerrainBlock()
// Desc : Adds a terrain block, or multiple blocks, to this object.
// Note : Returns the index for the first block added, or -1 on failure.
//-----------------------------------------------------------------------------
long CTerrain::AddTerrainBlock( ULONG Count )
{
    CTerrainBlock ** pBlockBuffer = NULL;
    
    // Allocate new resized array
    if (!( pBlockBuffer = new CTerrainBlock*[ m_nBlockCount + Count ] )) return -1;

    // Clear out slack pointers
    ZeroMemory( &pBlockBuffer[ m_nBlockCount ], Count * sizeof( CTerrainBlock* ) );

    // Existing Data?
    if ( m_pBlock )
    {
        // Copy old data into new buffer
        memcpy( pBlockBuffer, m_pBlock, m_nBlockCount * sizeof( CTerrainBlock* ) );

        // Release old buffer
        delete []m_pBlock;

    } // End if
    
    // Store pointer for new buffer
    m_pBlock = pBlockBuffer;

    // Allocate new Block pointers
    for ( UINT i = 0; i < Count; i++ )
    {
        // Allocate new Block
        if (!( m_pBlock[ m_nBlockCount ] = new CTerrainBlock() )) return -1;

        // Increase overall Block count
        m_nBlockCount++;

    } // Next Polygon
    
    // Return first Block
    return m_nBlockCount - Count;
}

//-----------------------------------------------------------------------------
// Name : AddTerrainLayer()
// Desc : Adds a terrain layer, or multiple layers, to this terrain.
// Note : Returns the index for the first layer added, or -1 on failure.
//-----------------------------------------------------------------------------
long CTerrain::AddTerrainLayer( USHORT Count )
{
    CTerrainLayer ** pLayerBuffer = NULL;
    
    // Allocate new resized array
    if (!( pLayerBuffer = new CTerrainLayer*[ m_nLayerCount + Count ] )) return -1;

    // Clear out slack pointers
    ZeroMemory( &pLayerBuffer[ m_nLayerCount ], Count * sizeof( CTerrainLayer* ) );

    // Existing Data?
    if ( m_pLayer )
    {
        // Copy old data into new buffer
        memcpy( pLayerBuffer, m_pLayer, m_nLayerCount * sizeof( CTerrainLayer* ) );

        // Release old buffer
        delete []m_pLayer;

    } // End if
    
    // Store pointer for new buffer
    m_pLayer = pLayerBuffer;

    // Allocate new Layer pointers
    for ( UINT i = 0; i < Count; i++ )
    {
        // Allocate new Layer
        if (!( m_pLayer[ m_nLayerCount ] = new CTerrainLayer() )) return -1;

        // Increase overall Layer count
        m_nLayerCount++;

    } // Next Layer
    
    // Return first Layer
    return m_nLayerCount - Count;
}

//-----------------------------------------------------------------------------
// Name : SetD3DDevice()
// Desc : Sets the D3D Device that will be used for buffer creation and renering
//-----------------------------------------------------------------------------
void CTerrain::SetD3DDevice( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL )
{
    // Validate Parameters
    if ( !pD3DDevice ) return;

    // Store D3D Device and add a reference
    m_pD3DDevice = pD3DDevice;
    m_pD3DDevice->AddRef();

    // Store vertex processing type for buffer creation
    m_bHardwareTnL = HardwareTnL;
}

//-----------------------------------------------------------------------------
// Name : UpdatePlayer() (Static)
// Desc : Called to allow the terrain object to update the player details
//        based on the height from the terrain for example.
//-----------------------------------------------------------------------------
void CTerrain::UpdatePlayer( LPVOID pContext, CPlayer * pPlayer, float TimeScale )
{
    // Validate Parameters
    if ( !pContext || !pPlayer ) return;

    VOLUME_INFO Volume   = pPlayer->GetVolumeInfo();
    D3DXVECTOR3 Position = pPlayer->GetPosition();
    D3DXVECTOR3 Velocity = pPlayer->GetVelocity();

    // Retrieve the height of the terrain at this position
    float fHeight = ((CTerrain*)pContext)->GetHeight( Position.x, Position.z, true ) - Volume.Min.y;

    // Determine if the position is lower than the height at this position
    if ( Position.y < fHeight )
    {
        // Update camera details
        Velocity.y = 0;
        Position.y = fHeight;

        // Update the camera
        pPlayer->SetVelocity( Velocity );
        pPlayer->SetPosition( Position );

    } // End if colliding

}

//-----------------------------------------------------------------------------
// Name : UpdateCamera() (Static)
// Desc : Called to allow the terrain object to update the camera details
//        based on the height from the terrain for example.
//-----------------------------------------------------------------------------
void CTerrain::UpdateCamera( LPVOID pContext, CCamera * pCamera, float TimeScale )
{
    CTerrain * pTerrain = (CTerrain*)pContext;

    // Validate Requirements
    if (!pContext || !pCamera ) return;
    if ( pCamera->GetCameraMode() != CCamera::MODE_THIRDPERSON ) return;

    VOLUME_INFO Volume   = pCamera->GetVolumeInfo();
    D3DXVECTOR3 Position = pCamera->GetPosition();
    bool        ReverseQuad = false;

    float fHeight = pTerrain->GetHeight( Position.x, Position.z, true ) - Volume.Min.y;

    // Determine if the position is lower than the height at this position
    if ( Position.y < fHeight )
    {
        // Update camera details
        Position.y = fHeight;
        pCamera->SetPosition( Position );

    } // End if colliding

    // Retrieve the player at which the camera is looking
    CPlayer * pPlayer = pCamera->GetPlayer();
    if (!pPlayer) return;

    // We have updated the position of either our player or camera
    // We must now instruct the camera to look at the players position
    ((CCam3rdPerson*)pCamera)->SetLookAt(  pPlayer->GetPosition() );

}


//-----------------------------------------------------------------------------
// Name : CTerrainBlock () (Constructor)
// Desc : CTerrainBlock Class Constructor
//-----------------------------------------------------------------------------
CTerrainBlock::CTerrainBlock( )
{
	// Reset / Clear all required values
    m_nStartX       = 0;
    m_nStartZ       = 0;
    m_pParent       = NULL;
    m_pLayerUsage   = NULL;
    m_nSplatCount   = 0;
    m_pSplatLevel   = NULL;
    m_pVertexBuffer = NULL;

    ZeroMemory( m_pNeighbours, 9 * sizeof(CTerrainBlock*) );
}

//-----------------------------------------------------------------------------
// Name : ~CTerrainBlock () (Destructor)
// Desc : CTerrainBlock Class Destructor
//-----------------------------------------------------------------------------
CTerrainBlock::~CTerrainBlock()
{
    ULONG i;

	// Release Splat Levels
    if ( m_pSplatLevel )
    {
        for ( i = 0; i < m_nSplatCount; i++ )
        {
            if ( m_pSplatLevel[i] ) delete m_pSplatLevel[i];

        } // Next Splat Level

        // Delete the array
        delete []m_pSplatLevel;

    } // End if Allocated

    // Release flat arrays
    if ( m_pLayerUsage ) delete []m_pLayerUsage;

    // Release Direct3D Resources
    if ( m_pVertexBuffer ) m_pVertexBuffer->Release();

    // Reset pointers
    m_pSplatLevel   = NULL;
    m_pLayerUsage   = NULL;
    m_pVertexBuffer = NULL;
}

//-----------------------------------------------------------------------------
// Name : GenerateBlock ()
// Desc : Generate this terrain block.
//-----------------------------------------------------------------------------
bool CTerrainBlock::GenerateBlock( CTerrain * pParent, ULONG StartX, ULONG StartZ, ULONG BlockWidth, ULONG BlockHeight )
{
    ULONG             x, z;
    HRESULT           hRet;
    ULONG             Usage      = D3DUSAGE_WRITEONLY;
    USHORT           *pIndex     = NULL;
    CVertex          *pVertex    = NULL;
    float            *pHeightMap = NULL;
    LPDIRECT3DDEVICE9 pD3DDevice = NULL;
    D3DXVECTOR3       VertexPos, LightDir = D3DXVECTOR3( 0.650945f, -0.390567f, 0.650945f );

    // Validate requirements
    if (!pParent || !pParent->GetD3DDevice() || !pParent->GetHeightMap()) return false;

    // Store some values
    m_pParent      = pParent;
    m_nStartX      = StartX;
    m_nStartZ      = StartZ;
    m_nBlockWidth  = BlockWidth;
    m_nBlockHeight = BlockHeight;
    m_nQuadsHigh   = BlockHeight - 1;
    m_nQuadsWide   = BlockWidth - 1;
    m_nQuadsHigh   = BlockHeight - 1;
    pHeightMap     = pParent->GetHeightMap();
    pD3DDevice     = pParent->GetD3DDevice();

    // Calculate buffer usage
    if ( !m_pParent->UseHardwareTnL() ) Usage |= D3DUSAGE_SOFTWAREPROCESSING;

    // Create the vertex buffer ready for generation
    hRet = pD3DDevice->CreateVertexBuffer((BlockWidth * BlockHeight) * sizeof(CVertex), Usage, VERTEX_FVF, D3DPOOL_MANAGED, &m_pVertexBuffer, NULL );
    if (FAILED(hRet)) return false;

    // Lock the vertex buffer ready to fill data
    hRet = m_pVertexBuffer->Lock( 0, (BlockWidth * BlockHeight) * sizeof(CVertex), (LPVOID*)&pVertex, 0 );
    if (FAILED(hRet)) return false;

    // Reset bounding box data
    m_BoundsMin = D3DXVECTOR3( 999999.0f, 999999.0f, 999999.0f );
    m_BoundsMax = D3DXVECTOR3( -999999.0f, -999999.0f, -999999.0f );

    // Loop through and generate the vertex data
    for ( z = StartZ; z < StartZ + BlockHeight; z++ )
    {
        for ( x = StartX; x < StartX + BlockWidth; x++ )
        {
            VertexPos.x = (float)x * m_pParent->GetScale().x;
            VertexPos.y = pHeightMap[ x + z * pParent->GetTerrainWidth() ] * m_pParent->GetScale().y;
            VertexPos.z = (float)z * m_pParent->GetScale().z;

            // Calculate vertex colour scale
            float fRed = 1.0f, fGreen = 1.0f, fBlue = 1.0f, fScale = 0.25f;
            
            // Generate average scale (for diffuse lighting calc)
            fScale  = D3DXVec3Dot( &pParent->GetHeightMapNormal( x, z ), &(-LightDir));
            fScale += D3DXVec3Dot( &pParent->GetHeightMapNormal( x + 1, z ), &(-LightDir));
            fScale += D3DXVec3Dot( &pParent->GetHeightMapNormal( x + 1, z + 1 ), &(-LightDir));
            fScale += D3DXVec3Dot( &pParent->GetHeightMapNormal( x, z + 1 ), &(-LightDir));
            fScale /= 4.0f;

            // Increase Saturation
            fScale += 0.25f; //0.05f; 

            // Clamp colour saturation
            if ( fScale > 1.0f ) fScale = 1.0f;
            if ( fScale < 0.4f ) fScale = 0.4f;

            // Store Vertex Values
            pVertex->x       = VertexPos.x;
            pVertex->y       = VertexPos.y;
            pVertex->z       = VertexPos.z;
            pVertex->Diffuse = D3DCOLOR_COLORVALUE( fRed * fScale, fGreen * fScale, fBlue * fScale, 1.0f );
            pVertex->tu      = (float)x;
            pVertex->tv      = (float)z;
            pVertex->tu2     = (float)(x - StartX) / m_nQuadsWide;
            pVertex->tv2     = (float)(z - StartZ) / m_nQuadsHigh;

            // Calculate bounding box data
            if ( VertexPos.x < m_BoundsMin.x ) m_BoundsMin.x = VertexPos.x;
            if ( VertexPos.y < m_BoundsMin.y ) m_BoundsMin.y = VertexPos.y;
            if ( VertexPos.z < m_BoundsMin.z ) m_BoundsMin.z = VertexPos.z;
            if ( VertexPos.x > m_BoundsMax.x ) m_BoundsMax.x = VertexPos.x;
            if ( VertexPos.y > m_BoundsMax.y ) m_BoundsMax.y = VertexPos.y;
            if ( VertexPos.z > m_BoundsMax.z ) m_BoundsMax.z = VertexPos.z;
            
            // Move to next vertex
            pVertex++;

        } // Next Column
    
    } // Next Row

    // Finished with the vertex buffer
    m_pVertexBuffer->Unlock();

    // Determine all the layers used by this block
    if ( !CountLayerUsage() ) return false;

    // Generate Splat Levels for this block
    if ( !GenerateSplats() ) return false;

    // Generate the blend maps
    if ( !GenerateBlendMaps() ) return false;

    // Success!
    return true;
}


//-----------------------------------------------------------------------------
// Name : CountLayerUsage () (Private)
// Desc : Count up the number of times a layer is used by this block.
//-----------------------------------------------------------------------------
bool CTerrainBlock::CountLayerUsage()
{
    USHORT i;
    ULONG  x, z;
    UCHAR  Value;

    // Allocate the layer usage array
    m_pLayerUsage = new USHORT[ m_pParent->GetLayerCount() ];
    if( !m_pLayerUsage ) return false;
    ZeroMemory( m_pLayerUsage, m_pParent->GetLayerCount() * sizeof(USHORT));

    // Pre-Calculate loop counts
    ULONG LoopStartX = (m_nStartX * m_pParent->GetBlendTexRatio());
    ULONG LoopStartZ = (m_nStartZ * m_pParent->GetBlendTexRatio());
    ULONG LoopEndX   = (m_nStartX + m_nQuadsWide) * m_pParent->GetBlendTexRatio();
    ULONG LoopEndZ   = (m_nStartZ + m_nQuadsHigh) * m_pParent->GetBlendTexRatio();

    // Determine which layers we are using in this block
    for ( z = LoopStartZ; z < LoopEndZ; z++ )
    {
        for ( x = LoopStartX; x < LoopEndX; x++ )
        {
            // Loop through each layer
            for ( i = 0; i < m_pParent->GetLayerCount(); i++ )
            {
                CTerrainLayer * pLayer = m_pParent->GetLayer(i);

                // Retrieve alpha value
                Value = pLayer->m_pBlendMap[ x + z * pLayer->m_nLayerWidth ];
                if ( Value > 0 ) m_pLayerUsage[i]++;

            } // Next Layer

        } // Next Column

    } // Next Row
    
    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : GenerateSplats () (Private)
// Desc : Generate the various splat levels required for this block
//-----------------------------------------------------------------------------
bool CTerrainBlock::GenerateSplats()
{
    USHORT i;

    // Allocate the required number of splat levels
    if ( AddSplatLevel( m_pParent->GetLayerCount() ) < 0 ) return false;

    // Loop through each layer
    for ( i = 0; i < m_pParent->GetLayerCount(); i++ )
    {
        // Is this layer in use ?
        if ( !m_pLayerUsage[i] ) continue;

        // Generate the splat level for this layer
        if (!GenerateSplatLevel( i )) return false;

    } // Next Layer

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : GenerateSplatLevel () (Private)
// Desc : Generate an individual splat level for this terrain block.
//-----------------------------------------------------------------------------
bool CTerrainBlock::GenerateSplatLevel( USHORT TerrainLayer )
{
    HRESULT   hRet;
    USHORT   *pIndex = NULL;
    ULONG     x, z, ax, az;
    UCHAR     Value;
    float     BlendTexels = m_pParent->GetBlendTexRatio();

    LPDIRECT3DDEVICE9 pD3DDevice = m_pParent->GetD3DDevice();
    bool HardwareTnL = m_pParent->UseHardwareTnL();
    CTerrainLayer * pLayer = m_pParent->GetLayer( TerrainLayer );

    // Calculate usage variable
    ULONG Usage = D3DUSAGE_WRITEONLY;
    if (!HardwareTnL) Usage |= D3DUSAGE_SOFTWAREPROCESSING;

    // Allocate a new splat
    CTerrainSplat * pSplat = new CTerrainSplat;
    if (!pSplat) return false;

    // Store the splat
    m_pSplatLevel[ TerrainLayer ] = pSplat;

    // Store layer index (handy later on)
    pSplat->m_nLayerIndex = TerrainLayer;

    // Create the index buffer ready for generation
    hRet = pD3DDevice->CreateIndexBuffer( ((m_nQuadsWide * m_nQuadsHigh) * 6) * sizeof(USHORT), Usage, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pSplat->m_pIndexBuffer, NULL );
    if ( FAILED(hRet ) ) return false;

    // Lock the index buffer ready to fill data
    hRet = pSplat->m_pIndexBuffer->Lock( 0, ((m_nQuadsWide * m_nQuadsHigh) * 6) * sizeof(USHORT), (void**)&pIndex, 0 );
    if ( FAILED(hRet ) ) return false;

    // Calculate the indices for the splat tri-list
    for ( z = 0; z < m_nQuadsHigh; z++ )
    {
        // Pre-Calc Loop starts / ends
        ULONG LoopStartZ = ( z + m_nStartZ ) * BlendTexels;
        ULONG LoopEndZ   = LoopStartZ + BlendTexels;

        for ( x = 0; x < m_nQuadsWide; x++ )
        {
            // Pre-Calc Loop starts / ends
            ULONG LoopStartX = ( x + m_nStartX ) * BlendTexels;
            ULONG LoopEndX   = LoopStartX + BlendTexels;

            // Determine if element is visible anywhere
            for ( az = LoopStartZ; az < LoopEndZ; az++ )
            {
                for ( ax = LoopStartX; ax < LoopEndX; ax++ )
                {
                    // Retrieve the layer data
                    Value = pLayer->m_pBlendMap[ ax + az * pLayer->m_nLayerWidth ];
                    if ( Value > 0 ) break;
                
                } // Next Alpha Column

                // Break if we found one
                if ( Value > 0 ) break;

            } // Next Alpha Row

            // Should we write the quad here ?
            if ( Value == 0 ) continue;

            // Insert next two triangles
            *pIndex++ = (USHORT)(x + z * m_nBlockWidth);
            *pIndex++ = (USHORT)(x + (z + 1) * m_nBlockWidth);
            *pIndex++ = (USHORT)((x + 1) + (z + 1) * m_nBlockWidth);

            *pIndex++ = (USHORT)(x + z * m_nBlockWidth);
            *pIndex++ = (USHORT)((x + 1) + (z + 1) * m_nBlockWidth);
            *pIndex++ = (USHORT)((x + 1) + z * m_nBlockWidth);

            // Increase our index & Primitive counts
            pSplat->m_nIndexCount     += 6;
            pSplat->m_nPrimitiveCount += 2;

        } // Next Element Column
    
    } // Next Element ROw

    // Unlock the index buffer
    pSplat->m_pIndexBuffer->Unlock();

    // Success!!
    return true;

}

//-----------------------------------------------------------------------------
// Name : GenerateBlendMaps () (Private)
// Desc : Now generate the blend maps to blend the splats together.
//-----------------------------------------------------------------------------
bool CTerrainBlock::GenerateBlendMaps( )
{
    HRESULT hRet;
    ULONG Width, Height, i, x, z;
    LPDIRECT3DDEVICE9 pD3DDevice = m_pParent->GetD3DDevice();
    D3DLOCKED_RECT LockData;
    UCHAR Value;
    ULONG BlendTexels = m_pParent->GetBlendTexRatio();

    // Bail if we have no data
    if ( m_nSplatCount == 0 ) return true;
    
    // Calculate width / height of the texture
    Width = (m_nQuadsWide * BlendTexels);
    Height = (m_nQuadsHigh * BlendTexels);
    
    // Calculate each splats blend map
    for ( i = 0; i < m_nSplatCount; i++ )
    {
        // Bail if this is an empty splat level
        if ( !m_pSplatLevel[i] ) continue;

        CTerrainLayer * pLayer = m_pParent->GetLayer( i );
        
        // We never generate an alpha map for terrain layer 0
        if ( m_pSplatLevel[i]->m_nLayerIndex == 0) continue;
        
        // Create our blend texture
        hRet = pD3DDevice->CreateTexture( Width, Height, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &m_pSplatLevel[i]->m_pBlendTexture, NULL );
        if ( FAILED(hRet) ) return false;
            
        // Lock the texture
        hRet = m_pSplatLevel[i]->m_pBlendTexture->LockRect( 0, &LockData, NULL, 0 );
        if ( FAILED(hRet) ) return false;
         
        USHORT * pBuffer = (USHORT*)LockData.pBits;

        // Loop through each pixel and store
        for ( z = 0; z < Height; z++ )
        {
            for ( x = 0; x < Width; x++, pBuffer++ )
            {
                // Retrieve alpha value
                Value = pLayer->m_pBlendMap[ (x + (m_nStartX * BlendTexels)) + (z + (m_nStartZ * BlendTexels)) * pLayer->m_nLayerWidth ];

                // Store value in buffer ( Shift right 4 and left 12 )
                *pBuffer = ((LONG)Value << 8) & 0xF000;
            
            } // Next Column

            // Add on pitch tail
            pBuffer += LockData.Pitch - ( Width * sizeof(USHORT) );
        
        } // Next Row

        // Unlock the blend texture
        m_pSplatLevel[i]->m_pBlendTexture->UnlockRect( 0 );

    } // Next Splat Level        

    // Success!!
    return true;

}

//-----------------------------------------------------------------------------
// Name : AddSplatLevel()
// Desc : Adds a splat level , or multiple levels, to this terrain block.
// Note : Returns the index for the first splat added, or -1 on failure.
//-----------------------------------------------------------------------------
long CTerrainBlock::AddSplatLevel( USHORT Count )
{
    CTerrainSplat ** pSplatBuffer = NULL;
    
    // Allocate new resized array
    if (!( pSplatBuffer = new CTerrainSplat*[ m_nSplatCount + Count ] )) return -1;

    // Clear out slack pointers
    ZeroMemory( &pSplatBuffer[ m_nSplatCount ], Count * sizeof( CTerrainSplat* ) );

    // Existing Data?
    if ( m_pSplatLevel )
    {
        // Copy old data into new buffer
        memcpy( pSplatBuffer, m_pSplatLevel, m_nSplatCount * sizeof( CTerrainSplat* ) );

        // Release old buffer
        delete []m_pSplatLevel;

    } // End if
    
    // Store pointer for new buffer
    m_pSplatLevel = pSplatBuffer;

    // Increase overall Splat count (we leave unused splats @ NULL)
    m_nSplatCount += Count;

    // Return first Splat
    return m_nSplatCount - Count;
}

//-----------------------------------------------------------------------------
// Name : Render ()
// Desc : Render the terrain block
//-----------------------------------------------------------------------------
void CTerrainBlock::Render( LPDIRECT3DDEVICE9 pD3DDevice, USHORT LayerIndex )
{
    // Bail if this layer is not in use
    if ( !m_pSplatLevel[LayerIndex] ) return;

    // Set up vertex streams & Textures
    pD3DDevice->SetIndices( m_pSplatLevel[LayerIndex]->m_pIndexBuffer );
    pD3DDevice->SetTexture( 1, m_pSplatLevel[LayerIndex]->m_pBlendTexture );

    // Render the vertex buffer
    if ( m_pSplatLevel[LayerIndex]->m_nPrimitiveCount == 0 ) return;
    pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, (m_nBlockWidth * m_nBlockHeight), 0, m_pSplatLevel[LayerIndex]->m_nPrimitiveCount );

}

//-----------------------------------------------------------------------------
// Name : CTerrainSplat () (Constructor)
// Desc : CTerrainSplat Class Constructor
//-----------------------------------------------------------------------------
CTerrainSplat::CTerrainSplat( )
{
	// Reset / Clear all required values
    m_pIndexBuffer      = NULL;
    m_nIndexCount       = 0;
    m_nPrimitiveCount   = 0;
    m_nLayerIndex       = 0;
    m_pBlendTexture     = NULL;
}

//-----------------------------------------------------------------------------
// Name : ~CTerrainSplat () (Destructor)
// Desc : CTerrainSplat Class Destructor
//-----------------------------------------------------------------------------
CTerrainSplat::~CTerrainSplat()
{
    // Release Direct3D Objects
    if ( m_pIndexBuffer  ) m_pIndexBuffer->Release();
    if ( m_pBlendTexture ) m_pBlendTexture->Release();
   
    // Reset pointers
    m_pIndexBuffer      = NULL;
    m_pBlendTexture     = NULL;
}

//-----------------------------------------------------------------------------
// Name : CTerrainLayer () (Constructor)
// Desc : CTerrainLayer Class Constructor
//-----------------------------------------------------------------------------
CTerrainLayer::CTerrainLayer( )
{
	// Reset / Clear all required values
    m_nTextureIndex = 0;
    m_nLayerWidth   = 0;
    m_nLayerHeight  = 0;
    m_pBlendMap     = NULL;
    D3DXMatrixIdentity( &m_mtxTexture );
}

//-----------------------------------------------------------------------------
// Name : ~CTerrainLayer () (Destructor)
// Desc : CTerrainLayer Class Destructor
//-----------------------------------------------------------------------------
CTerrainLayer::~CTerrainLayer()
{
    // Release flat arrays
    if ( m_pBlendMap ) delete []m_pBlendMap;

    // Reset pointers
    m_pBlendMap = NULL;
}

//-----------------------------------------------------------------------------
// Name : GetFilteredAlpha ()
// Desc : Retrieve the filtered alpha at the position specified.
//-----------------------------------------------------------------------------
UCHAR CTerrainLayer::GetFilteredAlpha( ULONG x, ULONG z )
{
    long Total, Sum, PosX, PosZ;

    // Validate Parameters
    if ( !m_pBlendMap ) return 0;

    // Loop through each neighbour
    PosX = x; PosZ = z;
    Total = m_pBlendMap[ PosX + PosZ * m_nLayerWidth ];
    Sum = 1;
    
    // Above Pixel
    PosX = x; PosZ = z - 1;        
    if ( PosZ >= 0 )
    {
        Total += m_pBlendMap[ PosX + PosZ * m_nLayerWidth ];
        Sum++;
    
    } // End if Not OOB
    
    // Right Pixel
    PosX = x + 1; PosZ = z;
    if ( PosX < (signed)m_nLayerWidth )
    {
        Total += m_pBlendMap[ PosX + PosZ * m_nLayerWidth ];
        Sum++;
    
    } // End if Not OOB
    
    // Bottom Pixel
    PosX = x; PosZ = z + 1;
    if ( PosZ < (signed)m_nLayerHeight )
    {
        Total += m_pBlendMap[ PosX + PosZ * m_nLayerWidth ];
        Sum++;
    
    } // End if Not OOB
    
    // Left Pixel
    PosX = x - 1; PosZ = z;
    if ( PosX >= 0 )
    {
        Total += m_pBlendMap[ PosX + PosZ * m_nLayerWidth ];
        Sum++;
    
    } // End if Not OOB
    
    // Return result
    return (UCHAR)(Total / Sum);
}