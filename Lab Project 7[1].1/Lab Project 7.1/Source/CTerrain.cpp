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
// Module Local Constants
//-----------------------------------------------------------------------------
namespace
{
    const USHORT BlockWidth  = 17;                  // Number of vertices in a terrain block (X)
    const USHORT BlockHeight = 17;                  // Number of vertices in a terrain block (Z)
    const USHORT QuadsWide   = BlockWidth - 1;      // Number of quads in a terrain block (X)
    const USHORT QuadsHigh   = BlockHeight - 1;     // Number of quads in a terrain block (Z)
    const float  WaterLevel  = 54.0f;               // Height of the water level (will be multiplied by Y scale)
    const LPCSTR BaseTextureName   = "Data\\Base_Texture_BigPoint.jpg";
    const LPCSTR DetailTextureName = "Data\\Detail_Texture.jpg";
    const LPCSTR WaterTextureName  = "Data\\Water_Texture.jpg";
};

//-----------------------------------------------------------------------------
// Name : CTerrain () (Constructor)
// Desc : CTerrain Class Constructor
//-----------------------------------------------------------------------------
CTerrain::CTerrain()
{
    // Reset all required values
    m_pD3DDevice        = NULL;
    m_pBaseTexture      = NULL;
    m_pDetailTexture    = NULL;
    m_pWaterTexture     = NULL;
    m_bSinglePass       = true;

    m_pHeightMap        = NULL;
    m_nHeightMapWidth   = 0;
    m_nHeightMapHeight  = 0;

    m_pMesh             = NULL;
    m_nMeshCount        = 0;

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

    // Release Meshes
    if ( m_pMesh ) 
    {
        // Delete all individual meshes in the array.
        for ( ULONG i = 0; i < m_nMeshCount; i++ )
        {
            if ( m_pMesh[i] ) delete m_pMesh[i];
        
        } // Next Mesh

        // Free up the array itself
        delete []m_pMesh;
    
    } // End if

    // Release our D3D Object ownership
    if ( m_pWaterTexture  ) m_pWaterTexture->Release();
    if ( m_pBaseTexture   ) m_pBaseTexture->Release();
    if ( m_pDetailTexture ) m_pDetailTexture->Release();
    if ( m_pD3DDevice     ) m_pD3DDevice->Release();

    // Clear Variables
    m_pHeightMap        = NULL;
    m_nHeightMapWidth   = 0;
    m_nHeightMapHeight  = 0;
    m_pMesh             = NULL;
    m_nMeshCount        = 0;
    m_pD3DDevice        = NULL;
    m_pBaseTexture      = NULL;
    m_pDetailTexture    = NULL;
    m_pWaterTexture     = NULL;
}

//-----------------------------------------------------------------------------
// Name : SetTextureFormat()
// Desc : Informs our scene manager with which format standard textures should
//        be created.
//-----------------------------------------------------------------------------
void CTerrain::SetTextureFormat( const D3DFORMAT & Format )
{
    // Store texture format
    m_fmtTexture = Format;
}

//-----------------------------------------------------------------------------
// Name : LoadHeightMap ()
// Desc : Load in the heightmap and build the meshes required to render it.
// Note : You are required to pass in the width and height of the 2d data
//        that you are loading from the raw heightmap file. These values must
//        be multiples of QuadsWide / QuadsHigh after subtracting one.
//-----------------------------------------------------------------------------
bool CTerrain::LoadHeightMap( LPCTSTR FileName, ULONG Width, ULONG Height )
{
    HRESULT hRet;
    FILE  * pFile = NULL;

    // Cannot load if already allocated (must be explicitly released for reuse)
    if ( m_pMesh ) return false;

    // Must have an already set D3D Device
    if ( !m_pD3DDevice ) return false;
    
    // First of all store the information passed
    m_nHeightMapWidth  = Width;
    m_nHeightMapHeight = Height;

    // A scale of 4 is roughly the best size for a 512 x 512 quad terrain.
    // Using the following forumla, lowering the size of the terrain 
    // simply lowers the vertex resolution but maintains the map size.
    m_vecScale.x = 4.0f * (512 / (m_nHeightMapWidth - 1));
    m_vecScale.y = 2.0f;
    m_vecScale.z = 4.0f * (512 / (m_nHeightMapHeight - 1));

    // Attempt to allocate space for this heightmap information
    m_pHeightMap = new UCHAR[Width * Height];
    if (!m_pHeightMap) return false;

    // Open up the heightmap file
    pFile = _tfopen( FileName, _T("rb") );
    if (!pFile) return false;

    // Read the heightmap data (grayscale)
    fread( m_pHeightMap, Width * Height, 1, pFile );
    
    // Finish up
    fclose( pFile );

    // Load in the textures used for rendering the terrain
    hRet = D3DXCreateTextureFromFileEx( m_pD3DDevice, BaseTextureName, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
                                        0, m_fmtTexture, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 
                                        0, NULL, NULL, &m_pBaseTexture );
    if ( FAILED(hRet) ) return false;

    hRet = D3DXCreateTextureFromFileEx( m_pD3DDevice, DetailTextureName, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
                                        0, m_fmtTexture, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 
                                        0, NULL, NULL, &m_pDetailTexture );
    if ( FAILED(hRet) ) return false;

    hRet = D3DXCreateTextureFromFileEx( m_pD3DDevice, WaterTextureName, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
                                        0, m_fmtTexture, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 
                                        0, NULL, NULL, &m_pWaterTexture );
    if ( FAILED(hRet) ) return false;

    // Allocate enough meshes to store the separate blocks of this terrain
    if ( AddMesh( ((Width - 1) / QuadsWide) * ((Height - 1) / QuadsHigh) ) < 0 ) return false;

    // Build the mesh data itself
    return BuildMeshes( );
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
	y1 = (float)m_pHeightMap[HMIndex] * m_vecScale.y;
	y2 = (float)m_pHeightMap[HMIndex + HMAddX] * m_vecScale.y; 
	y3 = (float)m_pHeightMap[HMIndex + HMAddZ] * m_vecScale.y;
			
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
        fTopLeft     = (float)m_pHeightMap[ix + iz * m_nHeightMapWidth] * m_vecScale.y;
        fBottomRight = (float)m_pHeightMap[(ix + 1) + (iz + 1) * m_nHeightMapWidth] * m_vecScale.y;

        // Which triangle of the quad are we in ?
        if ( fPercentX < fPercentZ )
        {
            fBottomLeft = (float)m_pHeightMap[ix + (iz + 1) * m_nHeightMapWidth] * m_vecScale.y;
		    fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
        
        } // End if Left Triangle
        else
        {
            fTopRight   = (float)m_pHeightMap[(ix + 1) + iz * m_nHeightMapWidth] * m_vecScale.y;
		    fBottomLeft = fTopLeft + (fBottomRight - fTopRight);

        } // End if Right Triangle
    
    } // End if Quad is reversed
    else
    {
        // First retrieve the height of each point in the dividing edge
        fTopRight   = (float)m_pHeightMap[(ix + 1) + iz * m_nHeightMapWidth] * m_vecScale.y;
        fBottomLeft = (float)m_pHeightMap[ix + (iz + 1) * m_nHeightMapWidth] * m_vecScale.y;

        // Calculate which triangle of the quad are we in ?
        if ( fPercentX < (1.0f - fPercentZ)) 
        {
            fTopLeft = (float)m_pHeightMap[ix + iz * m_nHeightMapWidth] * m_vecScale.y;
            fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
        
        } // End if Left Triangle
        else
        {
            fBottomRight = (float)m_pHeightMap[(ix + 1) + (iz + 1) * m_nHeightMapWidth] * m_vecScale.y;
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
// Name : BuildMeshes()
// Desc : Build all of the mesh data required to render the terrain.
//-----------------------------------------------------------------------------
bool CTerrain::BuildMeshes( )
{
    long x, z, vx, vz, Counter, StartX, StartZ;
    long BlocksWide = (m_nHeightMapWidth  - 1) / QuadsWide;
    long BlocksHigh = (m_nHeightMapHeight - 1) / QuadsHigh;

    D3DXVECTOR3 VertexPos, LightDir = D3DXVECTOR3( 0.650945f, -0.390567f, 0.650945f );

    // Note : The following formulas are expanded for clarity. They could be reduced.
    // Calculate IndexCount.... ( Number required for quads ) + ( Extra Degenerates verts (one per quad row except last) )
    ULONG IndexCount  = ((BlockWidth * 2) * QuadsHigh) + ( QuadsHigh - 1 );
    
    // Calculate Primitive Count (( Number of quads ) * 2) + ( 3 degenerate tris per quad row except last )
    m_nPrimitiveCount = ((QuadsWide * QuadsHigh) * 2) + ((QuadsHigh - 1) * 3);

    // Loop through and generate the mesh data
    for ( z = 0; z < BlocksHigh; z++ )
    {
        for ( x = 0; x < BlocksWide; x++ )
        {
            CMesh * pMesh = m_pMesh[ x + z * BlocksWide ];

            // Allocate all the vertices & indices required for this mesh
            pMesh->SetVertexFormat( VERTEX_FVF, sizeof(CVertex));
            if ( pMesh->AddVertex( BlockWidth * BlockHeight ) < 0 ) return false;
            if ( pMesh->AddIndex( IndexCount ) < 0 ) return false;

            // Store pointer for filling
            CVertex * pVertex = (CVertex*)pMesh->m_pVertex;

            // Calculate Vertex Positions
            Counter = 0;
            StartX  = x * (BlockWidth  - 1);
            StartZ  = z * (BlockHeight - 1);
            for ( vz = StartZ; vz < StartZ + BlockHeight; vz++ )
            {
                for ( vx = StartX; vx < StartX + BlockWidth; vx++ )
                {
                    // Calculate and Set The vertex data.
                    pVertex[ Counter ].x  = (float)vx * m_vecScale.x;
                    pVertex[ Counter ].y  = (float)m_pHeightMap[ vx + vz * m_nHeightMapWidth ] * m_vecScale.y;
                    pVertex[ Counter ].z  = (float)vz * m_vecScale.z;
                    pVertex[ Counter ].tu = (float)vx / (m_nHeightMapWidth - 1);
                    pVertex[ Counter ].tv = (float)vz / (m_nHeightMapHeight - 1);
                    pVertex[ Counter ].tu2 = (float)vx / 6.0f;
                    pVertex[ Counter ].tv2 = (float)vz / 6.0f;
                    Counter++;
                
                } // Next Vertex Column
            
            } // Next Vertex Row

            Counter = 0;
            // Calculate the indices for the terrain block tri-strip 
            for ( vz = 0; vz < BlockHeight - 1; vz++ )
            {
                // Is this an odd or even row ?
                if ( (vz % 2) == 0 )
                {
                    for ( vx = 0; vx < BlockWidth; vx++ )
                    {
                        // Force insert winding order switch degenerate ?
                        if ( vx == 0 && vz > 0 ) pMesh->m_pIndex[ Counter++ ] = (USHORT)(vx + vz * BlockWidth);

                        // Insert next two indices
                        pMesh->m_pIndex[ Counter++ ] = (USHORT)(vx + vz * BlockWidth);
                        pMesh->m_pIndex[ Counter++ ] = (USHORT)((vx + vz * BlockWidth) + BlockWidth);

                    } // Next Index Column
                    
                } // End if even row
                else
                {
                    for ( vx = BlockWidth - 1; vx >= 0; vx--)
                    {
                        // Force insert winding order switch degenerate ?
                        if ( vx == (BlockWidth - 1) ) pMesh->m_pIndex[ Counter++ ] = (USHORT)(vx + vz * BlockWidth);

                        // Insert next two indices
                        pMesh->m_pIndex[ Counter++ ] = (USHORT)(vx + vz * BlockWidth);
                        pMesh->m_pIndex[ Counter++ ] = (USHORT)((vx + vz * BlockWidth) + BlockWidth);

                    } // Next Index Column

                } // End if odd row
            
            } // Next Index Row

            // Instruct mesh to build buffers
            if ( FAILED(pMesh->BuildBuffers( m_pD3DDevice, m_bHardwareTnL )) ) return false;

        } // Next Block Column
    
    } // Next Block Row

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : Render()
// Desc : Renders all of the meshes stored within this terrain object.
//-----------------------------------------------------------------------------
void CTerrain::Render( CCamera * pCamera )
{
    ULONG i;

    // Validate parameters
    if( !m_pD3DDevice ) return;
    
    // Set our base texture in stage 0
    m_pD3DDevice->SetTexture( 0, m_pBaseTexture );
    
    // Set detail texture in stage 1 if supported
    if ( m_bSinglePass ) m_pD3DDevice->SetTexture( 1, m_pDetailTexture );

    // Set the FVF code for the terrain meshes, these will always
    // be identical between each mesh stored here, so we can simply
    // use the first.
    if ( m_nMeshCount > 0 ) m_pD3DDevice->SetFVF( m_pMesh[0]->m_nFVFCode );

    // Render Each Mesh
    for ( i = 0; i < m_nMeshCount; i++ )
    {
        // Skip if mesh is not within the viewing frustum
        if ( pCamera && (!pCamera->BoundsInFrustum( m_pMesh[i]->m_BoundsMin, m_pMesh[i]->m_BoundsMax )) ) continue;

        // Set the stream sources
        m_pD3DDevice->SetStreamSource( 0, m_pMesh[i]->m_pVertexBuffer, 0, m_pMesh[i]->m_nStride );
        m_pD3DDevice->SetIndices( m_pMesh[i]->m_pIndexBuffer );

        // Render the vertex buffer
        m_pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, BlockWidth * BlockHeight, 0, m_nPrimitiveCount );

    } // Next Mesh

    // If we are not using single pass we must render the detail in a second pass
    if ( !m_bSinglePass )
    {
        // Set our detail texture in stage 0, use 2nd texture coordinates.
        m_pD3DDevice->SetTexture( 0, m_pDetailTexture  );
        m_pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 1 );

        // Enable alpha blending to blend the dest and src colors together
        m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND , D3DBLEND_DESTCOLOR );
        m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );
        m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
        
        // Render Each Mesh
        for ( i = 0; i < m_nMeshCount; i++ )
        {
            // Skip if mesh is not within the viewing frustum
            if ( pCamera && (!pCamera->BoundsInFrustum( m_pMesh[i]->m_BoundsMin, m_pMesh[i]->m_BoundsMax )) ) continue;

            // Set the stream sources
            m_pD3DDevice->SetStreamSource( 0, m_pMesh[i]->m_pVertexBuffer, 0, m_pMesh[i]->m_nStride );
            m_pD3DDevice->SetIndices( m_pMesh[i]->m_pIndexBuffer );

            // Render the vertex buffer
            m_pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, BlockWidth * BlockHeight, 0, m_nPrimitiveCount );

        } // Next Mesh

        // Reset states for next call
        m_pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
        m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    
    } // End if requires second pass

    // Render the water
    RenderWater( pCamera );
}

//-----------------------------------------------------------------------------
// Name : RenderWater() (Private)
// Desc : Clips and renders the water so that we get a nice seam when
//        transitioning above / below.
//-----------------------------------------------------------------------------
void CTerrain::RenderWater( CCamera * pCamera )
{
    CLitVertex  Points[5];
    int         PointCount = 0;

    // Retrieve floating point water height
    float WaterHeight = WaterLevel * m_vecScale.y;
    
    // If we are close enough to the water, we need to clip
    if ( pCamera ) //&& (pCamera->GetPosition().y - 10.0f) < WaterHeight )  
    {
        // Build a combined projection / view matrix
        D3DXMATRIX mtxCombined = pCamera->GetViewMatrix() * pCamera->GetProjMatrix();
    
        // Extract the near clipping plane.
        D3DXPLANE NearPlane;
        NearPlane.a = -(mtxCombined._13);
        NearPlane.b = -(mtxCombined._23);
        NearPlane.c = -(mtxCombined._33);
        NearPlane.d = -(mtxCombined._43);
        D3DXPlaneNormalize( &NearPlane, &NearPlane );

        // Build initial 4 corner vectors
        CLitVertex vecWaterPoints[4];
        vecWaterPoints[0] = CLitVertex( 0.0f, WaterHeight, 0.0f, 0xBFFFFFFF, 0.0f, 0.0f );
        vecWaterPoints[1] = CLitVertex( 0.0f,  WaterHeight, m_nHeightMapHeight * m_vecScale.z, 0xBFFFFFFF, 0.0f, 1.0f );
        vecWaterPoints[2] = CLitVertex( m_nHeightMapWidth * m_vecScale.x, WaterHeight,  m_nHeightMapHeight * m_vecScale.z, 0xAFFFFFFF, 1.0f, 1.0f );
        vecWaterPoints[3] = CLitVertex( m_nHeightMapWidth * m_vecScale.x, WaterHeight, 0.0f, 0xBFFFFFFF, 1.0f, 0.0f );

        // Clip this quad against the plane, discard anything in front
        for ( int v1 = 0; v1 < 4; v1++ )
        {
            int v2 = (v1 + 1) % 4;
        
            // Classify each point in the edge
            int Location1 = 0, Location2 = 0;
			
            float result = D3DXPlaneDotCoord( &NearPlane, (D3DXVECTOR3*)&vecWaterPoints[v1] );
            if ( result < -1e-5f ) Location1 = -1; // Behind
            if ( result >  1e-5f ) Location1 =  1; // In Front

			// Keep it if it's on plane
            if ( Location1 == 0 )
            {
                Points[ PointCount++ ] = vecWaterPoints[v1];
                continue; // Skip to next vertex
        
            } // End if on plane

            result = D3DXPlaneDotCoord( &NearPlane, (D3DXVECTOR3*)&vecWaterPoints[v2] );
            if ( result < -1e-5f ) Location2 = -1; // Behind
            if ( result >  1e-5f ) Location2 =  1; // In Front

            // If its not in front, keep it.
            if ( Location1 != 1 ) Points[ PointCount++ ] = vecWaterPoints[v1];
        
		    // If the next vertex is not causing us to span the plane then continue
		    if ( Location2 == 0 || Location2 == Location1 ) continue;
	    
		    // Calculate the intersection point
            D3DXVECTOR3 vecIntersection;
            D3DXPlaneIntersectLine( &vecIntersection, &NearPlane, (D3DXVECTOR3*)&vecWaterPoints[v1], (D3DXVECTOR3*)&vecWaterPoints[v2] );

            // This is our new point
            Points[PointCount].x = vecIntersection.x;
            Points[PointCount].y = vecIntersection.y;
            Points[PointCount].z = vecIntersection.z;
            Points[PointCount].Diffuse = 0xBFFFFFFF;

            // Calculate the texture coordinates.
            float LineLength = D3DXVec3Length( &((D3DXVECTOR3&)vecWaterPoints[v2] - (D3DXVECTOR3&)vecWaterPoints[v1]) );
            float Distance   = D3DXVec3Length( &(vecIntersection - (D3DXVECTOR3&)vecWaterPoints[v1]) );
            Points[PointCount].tu = vecWaterPoints[v1].tu + ((vecWaterPoints[v2].tu - vecWaterPoints[v1].tu) * (Distance / LineLength));
            Points[PointCount].tv = vecWaterPoints[v1].tv + ((vecWaterPoints[v2].tv - vecWaterPoints[v1].tv) * (Distance / LineLength));
            PointCount++;
            
        } // Next Vertex

    } // End if Clip water
    else
    {
        Points[ PointCount++ ] = CLitVertex( 0.0f, WaterHeight, 0.0f, 0xBFFFFFFF, 0.0f, 0.0f );
        Points[ PointCount++ ] = CLitVertex( 0.0f,  WaterHeight, m_nHeightMapHeight * m_vecScale.z, 0xBFFFFFFF, 0.0f, 1.0f );
        Points[ PointCount++ ] = CLitVertex( m_nHeightMapWidth * m_vecScale.x, WaterHeight,  m_nHeightMapHeight * m_vecScale.z, 0xAFFFFFFF, 1.0f, 1.0f );
        Points[ PointCount++ ] = CLitVertex( m_nHeightMapWidth * m_vecScale.x, WaterHeight, 0.0f, 0xBFFFFFFF, 1.0f, 0.0f );

    } // End if Just Build

    // Disable second texture stage if in use
    if ( m_bSinglePass ) m_pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
    
    if ( PointCount > 2 )
    {
        // Setup alpha states for rendering water
        m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
        m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
        m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND , D3DBLEND_SRCALPHA );
        m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
        m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

        // Set our water texture into stage 0
        m_pD3DDevice->SetTexture( 0, m_pWaterTexture );
     
        // Set the FVF code for the water mesh.
        m_pD3DDevice->SetFVF( LITVERTEX_FVF );
        
        // Disable back face culling (so we see it from both sides) and zwriting
        m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
        m_pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

        // Render polygon
        m_pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, PointCount - 2, Points, sizeof(CLitVertex));

        // Reset states
        m_pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
        m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
        m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );

    } // End if not totally clipped

    // Render alpha blended quad if we are underwater
    if ( pCamera && (pCamera->GetPosition().y - 10.0f) < WaterHeight )  
    {
        pCamera->RenderScreenEffect( m_pD3DDevice, CCamera::EFFECT_WATER, *(ULONG*)(&WaterHeight) );
        
    } // End if render water effect

    // Re-enable second texture stage if required
    if ( m_bSinglePass ) m_pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, GetGameApp()->GetColorOp() );
}

//-----------------------------------------------------------------------------
// Name : AddMesh()
// Desc : Adds a mesh, or multiple meshes, to this object.
// Note : Returns the index for the first mesh added, or -1 on failure.
//-----------------------------------------------------------------------------
long CTerrain::AddMesh( ULONG Count )
{

    CMesh ** pMeshBuffer = NULL;
    
    // Allocate new resized array
    if (!( pMeshBuffer = new CMesh*[ m_nMeshCount + Count ] )) return -1;

    // Clear out slack pointers
    ZeroMemory( &pMeshBuffer[ m_nMeshCount ], Count * sizeof( CMesh* ) );

    // Existing Data?
    if ( m_pMesh )
    {
        // Copy old data into new buffer
        memcpy( pMeshBuffer, m_pMesh, m_nMeshCount * sizeof( CMesh* ) );

        // Release old buffer
        delete []m_pMesh;

    } // End if
    
    // Store pointer for new buffer
    m_pMesh = pMeshBuffer;

    // Allocate new mesh pointers
    for ( UINT i = 0; i < Count; i++ )
    {
        // Allocate new mesh
        if (!( m_pMesh[ m_nMeshCount ] = new CMesh() )) return -1;

        // Increase overall mesh count
        m_nMeshCount++;

    } // Next Polygon
    
    // Return first mesh
    return m_nMeshCount - Count;
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
// Name : SetRenderMode()
// Desc : Informs the terrain of how it should render.
//-----------------------------------------------------------------------------
void CTerrain::SetRenderMode( bool bSinglePass )
{
    m_bSinglePass = bSinglePass;
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
    bool        ReverseQuad = false;

    // Determine which row we are on
    int PosZ = (int)(Position.z / ((CTerrain*)pContext)->m_vecScale.z);
    if ( (PosZ % 2) != 0 ) ReverseQuad = true;

    // Retrieve the height of the terrain at this position
    float fHeight = ((CTerrain*)pContext)->GetHeight( Position.x, Position.z, ReverseQuad ) - Volume.Min.y;

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

    // If we are under water and in FPS mode, adjust our values
    CCamera::CAMERA_MODE CameraMode = pPlayer->GetCamera()->GetCameraMode();
    if ( CameraMode == CCamera::MODE_FPS )
    {
        if ( Position.y < (WaterLevel * ((CTerrain*)pContext)->m_vecScale.y) )
        {
            pPlayer->SetFriction( 250.0f ); // Per Second
            pPlayer->SetGravity( D3DXVECTOR3( 0, -260.0f, 0 ) );
            pPlayer->SetMaxVelocityXZ( 40.0f );
            pPlayer->SetMaxVelocityY ( 60.0f );
        
        } // End if below water
        else
        {
            pPlayer->SetFriction( 250.0f ); // Per Second
            pPlayer->SetGravity( D3DXVECTOR3( 0, -500.0f, 0 ) );
            pPlayer->SetMaxVelocityXZ( 125.0f );
            pPlayer->SetMaxVelocityY ( 400.0f );
    
        } // End if above water
    
    } // End if FPS mode

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

    // Determine which row we are on
    ULONG PosZ = (ULONG)(Position.z / pTerrain->m_vecScale.z);
    if ( (PosZ % 2) != 0 ) ReverseQuad = true; else ReverseQuad = false;

    float fHeight = pTerrain->GetHeight( Position.x, Position.z, ReverseQuad ) - Volume.Min.y;

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