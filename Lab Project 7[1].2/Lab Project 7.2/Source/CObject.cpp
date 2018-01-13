//-----------------------------------------------------------------------------
// File: CObject.cpp
//
// Desc: This file houses the various object / mesh related classes.
//
// Copyright (c) 1997-2002 Adam Hoult & Gary Simmons. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CObject Specific Includes
//-----------------------------------------------------------------------------
#include "..\\Includes\\CObject.h"

//-----------------------------------------------------------------------------
// Name : CObject () (Constructor)
// Desc : CObject Class Constructor
//-----------------------------------------------------------------------------
CObject::CObject()
{
	// Reset / Clear all required values
    m_pMesh = NULL;
    D3DXMatrixIdentity( &m_mtxWorld );
}

//-----------------------------------------------------------------------------
// Name : CObject () (Alternate Constructor)
// Desc : CObject Class Constructor, sets the internal mesh object
//-----------------------------------------------------------------------------
CObject::CObject( CMesh * pMesh )
{
	// Reset / Clear all required values
    D3DXMatrixIdentity( &m_mtxWorld );

    // Set Mesh
    m_pMesh = pMesh;
}

//-----------------------------------------------------------------------------
// Name : CMesh () (Constructor)
// Desc : CMesh Class Constructor
//-----------------------------------------------------------------------------
CMesh::CMesh()
{
	// Reset / Clear all required values
    m_pVertex         = NULL;
    m_pIndex          = NULL;
    m_nVertexCount    = 0;
    m_nIndexCount     = 0;
    m_nVertexCapacity = 0;
    m_nIndexCapacity  = 0;

    m_pVertexBuffer   = NULL;
    m_pIndexBuffer    = NULL;

}

//-----------------------------------------------------------------------------
// Name : CMesh () (Alternate Constructor)
// Desc : CMesh Class Constructor, adds specified number of vertices / indices
//-----------------------------------------------------------------------------
CMesh::CMesh( ULONG VertexCount, ULONG IndexCount )
{
	// Reset / Clear all required values
    m_pVertex         = NULL;
    m_pIndex          = NULL;
    m_nVertexCount    = 0;
    m_nIndexCount     = 0;
    m_nVertexCapacity = 0;
    m_nIndexCapacity  = 0;

    m_pVertexBuffer   = NULL;
    m_pIndexBuffer    = NULL;

    // Add Vertices & indices if required
    if ( VertexCount > 0 ) AddVertex( VertexCount );
    if ( IndexCount  > 0 ) AddIndex( IndexCount );
}

//-----------------------------------------------------------------------------
// Name : ~CMesh () (Destructor)
// Desc : CMesh Class Destructor
//-----------------------------------------------------------------------------
CMesh::~CMesh()
{
    Release();
}

//-----------------------------------------------------------------------------
// Name : Release ()
// Desc : Release any allocated memory for this device.
// Note : Added to allow for clean up prior to rebuilding without delete call.
//-----------------------------------------------------------------------------
void CMesh::Release()
{
    // Release our mesh components
    if ( m_pVertex ) delete []m_pVertex;
    if ( m_pIndex  ) delete []m_pIndex;
    
    if ( m_pVertexBuffer ) m_pVertexBuffer->Release();
    if ( m_pIndexBuffer  ) m_pIndexBuffer->Release();

    // Clear variables
    m_pVertex         = NULL;
    m_pIndex          = NULL;
    m_nVertexCount    = 0;
    m_nIndexCount     = 0;
    m_nVertexCapacity = 0;
    m_nIndexCapacity  = 0;

    m_pVertexBuffer = NULL;
    m_pIndexBuffer  = NULL;
}

//-----------------------------------------------------------------------------
// Name : SetVertexFormat ()
// Desc : Inform the mesh class about which format the vertices are assumed to
//        be, and also the size of each individual vertex.
//-----------------------------------------------------------------------------
void CMesh::SetVertexFormat( ULONG FVFCode, UCHAR Stride )
{
    // Store the values
    m_nFVFCode = FVFCode;
    m_nStride  = Stride;
}

//-----------------------------------------------------------------------------
// Name : AddVertex()
// Desc : Adds a vertex, or multiple vertices, to this mesh.
// Note : Returns the index for the first vertex added, or -1 on failure.
// Note : This function works slightly differently than before. It can no
//        longer assume that a fixed vertex size is used, and must therefore
//        take into account the specified stride.
//-----------------------------------------------------------------------------
long CMesh::AddVertex( ULONG Count )
{
    UCHAR * pVertexBuffer = NULL;
    
    if ( m_nVertexCount + Count > m_nVertexCapacity )
    {
        // Adjust our vertex capacity (resize 100 at a time)
        for ( ; m_nVertexCapacity < (m_nVertexCount + Count) ; ) m_nVertexCapacity += 100;

        // Allocate new resized array
        if (!( pVertexBuffer = new UCHAR[ m_nVertexCapacity * m_nStride] )) return -1;

        // Existing Data?
        if ( m_pVertex )
        {
            // Copy old data into new buffer
            memcpy( pVertexBuffer, m_pVertex, m_nVertexCount * m_nStride );

            // Release old buffer
            delete []m_pVertex;

        } // End if

        // Store pointer for new buffer
        m_pVertex = pVertexBuffer;
    
    } // End if a resize is required

    // Increase vertex count
    m_nVertexCount += Count;
    
    // Return first vertex
    return m_nVertexCount - Count;
}

//-----------------------------------------------------------------------------
// Name : AddIndex()
// Desc : Adds an index, or multiple indices, to this mesh.
// Note : Returns the index for the first vertex index added, or -1 on failure.
//-----------------------------------------------------------------------------
long CMesh::AddIndex( ULONG Count )
{
    USHORT * pIndexBuffer = NULL;
    
    if ( m_nIndexCount + Count > m_nIndexCapacity )
    {
        // Adjust our Index capacity (resize 100 at a time)
        for ( ; m_nIndexCapacity < (m_nIndexCount + Count) ; ) m_nIndexCapacity += 100;

        // Allocate new resized array
        if (!( pIndexBuffer = new USHORT[ m_nIndexCapacity ] )) return -1;

        // Existing Data?
        if ( m_pIndex )
        {
            // Copy old data into new buffer
            memcpy( pIndexBuffer, m_pIndex, m_nIndexCount * sizeof(USHORT) );

            // Release old buffer
            delete []m_pIndex;

        } // End if

        // Store pointer for new buffer
        m_pIndex = pIndexBuffer;
    
    } // End if a resize is required

    // Increase Index count
    m_nIndexCount += Count;
    
    // Return first Index
    return m_nIndexCount - Count;
}

//-----------------------------------------------------------------------------
// Name : BuildBuffers()
// Desc : Instructs the mesh to build a set of index / vertex buffers from the
//        data currently stored within the mesh object.
// Note : By passing in true to the 'ReleaseOriginals' parameter, the original
//        buffers will be destroyed.
//-----------------------------------------------------------------------------
HRESULT CMesh::BuildBuffers( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL, bool ReleaseOriginals )
{
    HRESULT     hRet    = S_OK;
    UCHAR      *pVertex = NULL;
    USHORT     *pIndex  = NULL;
    ULONG       ulUsage = D3DUSAGE_WRITEONLY;

    // Should we use software vertex processing ?
    if ( !HardwareTnL ) ulUsage |= D3DUSAGE_SOFTWAREPROCESSING;

    // Release any previously allocated vertex / index buffers
    if ( m_pVertexBuffer ) m_pVertexBuffer->Release();
    if ( m_pIndexBuffer  ) m_pIndexBuffer->Release();
    m_pVertexBuffer = NULL;
    m_pIndexBuffer  = NULL;

    // Create our vertex buffer
    hRet = pD3DDevice->CreateVertexBuffer( m_nStride * m_nVertexCount, ulUsage, m_nFVFCode,
                                             D3DPOOL_MANAGED, &m_pVertexBuffer, NULL );
    if ( FAILED( hRet ) ) return hRet;

    // Lock the vertex buffer ready to fill data
    hRet = m_pVertexBuffer->Lock( 0, m_nStride * m_nVertexCount, (void**)&pVertex, 0 );
    if ( FAILED( hRet ) ) return hRet;

    // Copy over the vertex data
    memcpy( pVertex, m_pVertex, m_nStride * m_nVertexCount );

    // We are finished with the vertex buffer
    m_pVertexBuffer->Unlock();

    
    // Create our index buffer
    hRet = pD3DDevice->CreateIndexBuffer( sizeof(USHORT) * m_nIndexCount, ulUsage, D3DFMT_INDEX16,
                                            D3DPOOL_MANAGED, &m_pIndexBuffer, NULL );
    if ( FAILED( hRet ) ) return hRet;

    // Lock the index buffer ready to fill data
    hRet = m_pIndexBuffer->Lock( 0, sizeof(USHORT) * m_nIndexCount, (void**)&pIndex, 0 );
    if ( FAILED( hRet ) ) return hRet;

    // Copy over the index data
    memcpy( pIndex, m_pIndex, sizeof(USHORT) * m_nIndexCount );

    // We are finished with the indexbuffer
    m_pIndexBuffer->Unlock();
    
    // Release old data if requested
    if ( ReleaseOriginals )
    {
        // Release our mesh components
        if ( m_pVertex ) delete []m_pVertex;
        if ( m_pIndex  ) delete []m_pIndex;

        // Clear variables
        m_pVertex       = NULL;
        m_pIndex        = NULL;

    } // End if ReleaseOriginals

    return S_OK;
}