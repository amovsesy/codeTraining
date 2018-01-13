//-----------------------------------------------------------------------------
// File: CScene.cpp
//
// Desc: Contains scene related data such as meshes, lights, materials etc.
//
// Copyright (c) 1997-2002 Adam Hoult & Gary Simmons. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CScene Specific Includes
//-----------------------------------------------------------------------------
#include "..\\Includes\\CScene.h"
#include "..\\Includes\\CObject.h"
#include "..\\Includes\\CTimer.h"

//-----------------------------------------------------------------------------
// IWF File Reading includes
//-----------------------------------------------------------------------------
#include "..\\Libs\\libIWF.h"
#include "..\\Libs\\iwfFile.h"
#include "..\\Libs\\iwfObjects.h"

//-----------------------------------------------------------------------------
// Module Local Constants
//-----------------------------------------------------------------------------
namespace
{
    const LPCSTR TexturePath = "Data\\";    // Location of texture data.
};

//-----------------------------------------------------------------------------
// Name : CScene () (Constructor)
// Desc : CScene Class Constructor
//-----------------------------------------------------------------------------
CScene::CScene()
{
	// Reset / Clear all required values
    m_nTextureCount    = 0;
    m_pTextureList     = NULL;
    m_pD3DDevice       = NULL;
    m_bHardwareTnL     = false;
    m_ppMeshList       = NULL;
    m_nMeshCount       = 0;
}

//-----------------------------------------------------------------------------
// Name : ~CScene () (Destructor)
// Desc : CScene Class Destructor
//-----------------------------------------------------------------------------
CScene::~CScene()
{
    // Release allocated resources
    Release();

}

//-----------------------------------------------------------------------------
// Name : Release ()
// Desc : Release all active resources
//-----------------------------------------------------------------------------
void CScene::Release( )
{
    ULONG i;

    // Release any allocated textures 
    if ( m_pTextureList )
    {
        for ( i = 0; i < m_nTextureCount; i++ )
        {
            if ( m_pTextureList[i] ) m_pTextureList[i]->Release();
    
        } // Next Light Group

        delete []m_pTextureList;
    
    } // End if Textures

    // Release any loaded meshes
    if ( m_ppMeshList )
    {
        for ( i = 0; i < m_nMeshCount; i++)
        {
            if ( m_ppMeshList[i] ) delete m_ppMeshList[i];
        
        } // Next Mesh

        delete []m_ppMeshList;
    }

    // Release Direct3D Objects
    if ( m_pD3DDevice ) m_pD3DDevice->Release();

    // Clear Variables
    m_pTextureList     = NULL;
    m_pD3DDevice       = NULL;
    m_bHardwareTnL     = false;
    m_ppMeshList       = NULL;
    m_nMeshCount       = 0;
}

//-----------------------------------------------------------------------------
// Name : SetD3DDevice()
// Desc : Sets the D3D Device that will be used for buffer creation and renering
//-----------------------------------------------------------------------------
void CScene::SetD3DDevice( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL )
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
// Name : SetTextureFormat()
// Desc : Informs our scene manager with which format standard textures should
//        be created.
//-----------------------------------------------------------------------------
void CScene::SetTextureFormat( const D3DFORMAT & Format, const D3DFORMAT & AlphaFormat )
{
    // Store texture format
    m_fmtTexture = Format;
    m_fmtAlpha   = AlphaFormat;
}

//-----------------------------------------------------------------------------
// Name : LoadScene ()
// Desc : Loads in the specified IWF scene file.
//-----------------------------------------------------------------------------
bool CScene::LoadScene( TCHAR * strFileName )
{
    CFileIWF File;

    // File loading may throw an exception
    try
    {
        // Attempt to load the file
        File.Load( strFileName );

        // Copy over the textures we want from the file
        if (!ProcessTextures( File )) return false;

        // Now process the meshes and extract the required data
        if (!ProcessMeshes( File )) return false;

        // Allow file loader to release any active objects
        File.ClearObjects();
        
    } // End Try Block

    // Catch any exceptions
    catch (...)
    {
        return false;
    
    } // End Catch Block

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : ProcessTextures () (Private)
// Desc : Processes the textures stored inside the file object passed
//-----------------------------------------------------------------------------
bool CScene::ProcessTextures( const CFileIWF& File )
{
    ULONG i;
    char  FileName[MAX_PATH];
    
    // Allocate enough room for all of our textures
    m_pTextureList = new LPDIRECT3DTEXTURE9[ File.m_vpTextureList.size() ];
    if ( !m_pTextureList ) return false;
    m_nTextureCount = File.m_vpTextureList.size();

    // Loop through and build our textures
    ZeroMemory( m_pTextureList, m_nTextureCount * sizeof(LPDIRECT3DTEXTURE9));
    for ( i = 0; i < File.m_vpTextureList.size(); i++ )
    {
        // Retrieve pointer to file texture
        TEXTURE_REF * pFileTexture = File.m_vpTextureList[i];

        // Skip if this is an internal texture (not supported by this demo)
        if ( pFileTexture->TextureSource != TEXTURE_EXTERNAL ) continue;

        // Build the final texture path
        strcpy( FileName, TexturePath );
        strcat( FileName, pFileTexture->Name );
        
        // Load the texture from file
        D3DXCreateTextureFromFileEx( m_pD3DDevice, FileName, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
                                     0, m_fmtAlpha, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 
                                     0, NULL, NULL, &m_pTextureList[i] );
        
    } // Next Texture

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : ProcessMeshes () (Private)
// Desc : Processes the meshes stored inside the file object passed
//-----------------------------------------------------------------------------
bool CScene::ProcessMeshes( CFileIWF & pFile )
{
    ULONG i, j;

    // Note : This demo assumes that we will be using the provided 'planet.iwf'
    //        and makes certain assumptions such as each mesh uses a single texture
    //        and that there will be two meshes in the file. For this reason
    //        this demo uses the vertex-buffer version of our CMesh object.
    if ( pFile.m_vpMeshList.size() != 2 ) return false;

    for ( i = 0; i < 2; i++ )
    {
        iwfMesh * pFileMesh = pFile.m_vpMeshList[i];

        // Allocate a new mesh object
        CMesh * pMesh = new CMesh;
        if (!pMesh) return false;

        // Set mesh's vertex format
        pMesh->SetVertexFormat( VERTEX_FVF, sizeof(CVertex) );

        // Loop through each surface of the file mesh
        for ( j = 0; j < pFileMesh->SurfaceCount; j++ )
        {
            iwfSurface * pSurface = pFileMesh->Surfaces[j];

            // Determine the texture indices we are using.
            long TextureIndex  = -1;
            if ( (pSurface->Components & SCOMPONENT_TEXTURES ) && pSurface->ChannelCount > 0 ) TextureIndex  = pSurface->TextureIndices[0];    
            pMesh->m_nTextureIndex = TextureIndex;

            // Store vertices & indices.
            if (!ProcessIndices ( pMesh, pSurface ) ) { delete pMesh; return false; }
            if (!ProcessVertices( pMesh, pSurface ) ) { delete pMesh; return false; }

        } // Next Surface

        // Store the mesh
        if (AddMesh() < 0) { delete pMesh; return false; }
        m_ppMeshList[ m_nMeshCount - 1 ] = pMesh;

        // Build the mesh's buffers
        pMesh->BuildBuffers( m_pD3DDevice, m_bHardwareTnL );
        
    } // Next file mesh

    // Store these two meshes as our internal objects
    m_pObject[0].m_pMesh = m_ppMeshList[0];
    m_pObject[1].m_pMesh = m_ppMeshList[1];

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : ProcessIndices () (Private)
// Desc : Processes the indices stored inside the polygon object passed
// Note : This performs the brunt of the conversion between primitive types.
//-----------------------------------------------------------------------------
bool CScene::ProcessIndices( CMesh * pMesh, iwfSurface * pFilePoly )
{
    ULONG i, Counter, VertexStart, IndexCount;
    
    // Store current property vertex start and index start
    VertexStart = pMesh->m_nVertexCount;
    IndexCount  = pMesh->m_nIndexCount;

    // Generate indices
    if ( pFilePoly->IndexCount > 0 )
    {
        ULONG IndexType = pFilePoly->IndexFlags & INDICES_MASK_TYPE;
    
        // Interpret indices (we want them in tri-list format)
        switch ( IndexType )
        {
            case INDICES_TRILIST:
            
                // We can do a straight copy (converting from 32bit to 16bit)
                if ( pMesh->AddIndex( pFilePoly->IndexCount ) < 0 ) return false;
                for ( i = 0; i < pFilePoly->IndexCount; i++ ) pMesh->m_pIndex[i + IndexCount] = pFilePoly->Indices[i] + VertexStart;
                break;

            case INDICES_TRISTRIP:
            
                // Index in strip order
                if ( pMesh->AddIndex( (pFilePoly->IndexCount - 2) * 3 ) < 0 ) return false;
                for ( Counter = IndexCount, i = 0; i < pFilePoly->IndexCount - 2; i++ )
                {
                    // Starting with triangle 0.
                    // Is this an 'Odd' or 'Even' triangle
                    if ( (i % 2) == 0 )
                    {
                        pMesh->m_pIndex[ Counter++ ] = pFilePoly->Indices[ i ] + VertexStart;
                        pMesh->m_pIndex[ Counter++ ] = pFilePoly->Indices[ i + 1 ] + VertexStart;
                        pMesh->m_pIndex[ Counter++ ] = pFilePoly->Indices[ i + 2 ] + VertexStart;
                    
                    } // End if 'Even' triangle
                    else
                    {
                        pMesh->m_pIndex[ Counter++ ] = pFilePoly->Indices[ i ] + VertexStart;
                        pMesh->m_pIndex[ Counter++ ] = pFilePoly->Indices[ i + 2 ] + VertexStart;
                        pMesh->m_pIndex[ Counter++ ] = pFilePoly->Indices[ i + 1 ] + VertexStart;

                    } // End if 'Odd' triangle

                } // Next vertex

                break;

            case INDICES_TRIFAN:

                // Index in fan order.
                if ( pMesh->AddIndex( (pFilePoly->IndexCount - 2 ) * 3 ) < 0 ) return false;
                for ( Counter = IndexCount, i = 1; i < pFilePoly->VertexCount - 1; i++ )
                {
                    pMesh->m_pIndex[ Counter++ ] = pFilePoly->Indices[ 0 ] + VertexStart;
                    pMesh->m_pIndex[ Counter++ ] = pFilePoly->Indices[ i ] + VertexStart;
                    pMesh->m_pIndex[ Counter++ ] = pFilePoly->Indices[ i + 1 ] + VertexStart;

                } // Next Triangle

                break;

        } // End Switch

    } // End if Indices Stored
    else
    {
        // We are going to try and build the indices ourselves
        ULONG VertexType = pFilePoly->VertexFlags & VERTICES_MASK_TYPE;

        // Interpret vertices (we want our indices in tri-list format)
        switch ( VertexType )
        {
            case VERTICES_TRILIST:

                // Straight fill
                if ( pMesh->AddIndex( pFilePoly->VertexCount ) < 0 ) return false;
                for ( i = 0; i < pFilePoly->VertexCount; i++ ) pMesh->m_pIndex[i + IndexCount] = i + VertexStart;

                break;

            case VERTICES_TRISTRIP:

                // Index in strip order
                if ( pMesh->AddIndex( (pFilePoly->VertexCount - 2) * 3 ) < 0 ) return false;
                for ( Counter = IndexCount, i = 0; i < pFilePoly->VertexCount - 2; i++ )
                {
                    // Starting with triangle 0.
                    // Is this an 'Odd' or 'Even' triangle
                    if ( (i % 2) == 0 )
                    {
                        pMesh->m_pIndex[ Counter++ ] = i + VertexStart;
                        pMesh->m_pIndex[ Counter++ ] = i + 1 + VertexStart;
                        pMesh->m_pIndex[ Counter++ ] = i + 2 + VertexStart;
                    
                    } // End if 'Even' triangle
                    else
                    {
                        pMesh->m_pIndex[ Counter++ ] = i + VertexStart;
                        pMesh->m_pIndex[ Counter++ ] = i + 2 + VertexStart;
                        pMesh->m_pIndex[ Counter++ ] = i + 1 + VertexStart;

                    } // End if 'Odd' triangle

                } // Next vertex

                break;

            case VERTICES_TRIFAN:
            
                // Index in fan order.
                if ( pMesh->AddIndex( (pFilePoly->VertexCount - 2 ) * 3 ) < 0 ) return false;
                for ( Counter = IndexCount, i = 1; i < pFilePoly->VertexCount - 1; i++ )
                {
                    pMesh->m_pIndex[ Counter++ ] = VertexStart;
                    pMesh->m_pIndex[ Counter++ ] = i + VertexStart;
                    pMesh->m_pIndex[ Counter++ ] = i + 1 + VertexStart;

                } // Next Triangle

                break;

        } // End Switch

    } // End if no Indices stored
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : ProcessVertices () (Private)
// Desc : Processes the vertices stored inside the polygon object passed
//-----------------------------------------------------------------------------
bool CScene::ProcessVertices( CMesh * pMesh, iwfSurface * pFilePoly )
{
    ULONG i, VertexStart = pMesh->m_nVertexCount;
    CVertex * pVertex;

    // Allocate enough vertices
    if ( pMesh->AddVertex( pFilePoly->VertexCount ) < 0 ) return false;
    pVertex = (CVertex*)pMesh->m_pVertex;

    // Loop through each vertex and copy required data.
    for ( i = 0; i < pFilePoly->VertexCount; i++ )
    {
        // Copy over vertex data
        pVertex[i + VertexStart].x       = pFilePoly->Vertices[i].x;
        pVertex[i + VertexStart].y       = pFilePoly->Vertices[i].y;
        pVertex[i + VertexStart].z       = pFilePoly->Vertices[i].z;
        
        // If we have any texture coordinates, set them
        if ( pFilePoly->TexChannelCount > 0 && pFilePoly->TexCoordSize[0] == 2 )
        {
            pVertex[i + VertexStart].tu = pFilePoly->Vertices[i].TexCoords[0][0];
            pVertex[i + VertexStart].tv = pFilePoly->Vertices[i].TexCoords[0][1];

        } // End if has tex coordinates

    } // Next Vertex

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : AddMesh() (Private)
// Desc : Adds a mesh, or multiple meshes, to this scene.
// Note : Returns the index for the first mesh added, or -1 on failure.
//-----------------------------------------------------------------------------
long CScene::AddMesh( ULONG Count )
{
    CMesh ** pMeshBuffer = NULL;
    
    // Allocate new resized array
    if (!( pMeshBuffer = new CMesh*[ m_nMeshCount + Count ] )) return -1;

    // Clear out slack pointers
    ZeroMemory( &pMeshBuffer[ m_nMeshCount ], Count * sizeof( CMesh* ) );

    // Existing Data?
    if ( m_ppMeshList )
    {
        // Copy old data into new buffer
        memcpy( pMeshBuffer, m_ppMeshList, m_nMeshCount * sizeof( CMesh* ) );

        // Release old buffer
        delete []m_ppMeshList;

    } // End if
    
    // Store pointer for new buffer
    m_ppMeshList = pMeshBuffer;
    m_nMeshCount += Count;

    // Return first Mesh
    return m_nMeshCount - Count;
}

//-----------------------------------------------------------------------------
// Name : AnimateObjects () (Private)
// Desc : Animates the objects we currently have loaded.
//-----------------------------------------------------------------------------
void CScene::AnimateObjects( CTimer & Timer )
{
    D3DXMATRIX mtxRotate;
    D3DXMatrixRotationY( &mtxRotate, D3DXToRadian(25.0f * Timer.GetTimeElapsed()) );
    D3DXMatrixMultiply( &m_pObject[0].m_mtxWorld, &mtxRotate, &m_pObject[0].m_mtxWorld );

    D3DXMatrixRotationY( &mtxRotate, D3DXToRadian(-5.0f * Timer.GetTimeElapsed()) );
    D3DXMatrixMultiply( &m_pObject[1].m_mtxWorld, &mtxRotate, &m_pObject[1].m_mtxWorld );
}

//-----------------------------------------------------------------------------
// Name : Render ()
// Desc : Render the scene
//-----------------------------------------------------------------------------
void CScene::Render( )
{
    // We render in reverse in our example to ensure that the opaque
    // inner core gets rendererd first.
    for ( long i = 1; i >= 0; i-- )
    {
        CMesh * pMesh = m_pObject[i].m_pMesh;

        // Set transformation matrix
        m_pD3DDevice->SetTransform( D3DTS_WORLD, &m_pObject[i].m_mtxWorld );

        // Set vertex stream
        m_pD3DDevice->SetStreamSource( 0, pMesh->m_pVertexBuffer, 0, pMesh->m_nStride);

        // Set Properties
        ULONG TextureIndex = pMesh->m_nTextureIndex;
        if ( TextureIndex >= 0 ) 
        {
            m_pD3DDevice->SetTexture( 0, m_pTextureList[ TextureIndex ] );
        
        } // End if has texture
        else
        {
            m_pD3DDevice->SetTexture( 0, NULL );
        
        } // End if has no texture

        // Set indices, and render
        m_pD3DDevice->SetIndices( pMesh->m_pIndexBuffer );
        m_pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, pMesh->m_nVertexCount, 0, pMesh->m_nIndexCount / 3 );

    } // Next Mesh
}
