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
    m_nPolygonCount = 0;
    m_pPolygon      = NULL;
    m_pNext         = NULL;

}

//-----------------------------------------------------------------------------
// Name : CMesh () (Alternate Constructor)
// Desc : CMesh Class Constructor, adds specified number of polygons
//-----------------------------------------------------------------------------
CMesh::CMesh( ULONG Count )
{
	// Reset / Clear all required values
    m_nPolygonCount = 0;
    m_pPolygon      = NULL;
    m_pNext         = NULL;

    // Add Polygons
    AddPolygon( Count );
}

//-----------------------------------------------------------------------------
// Name : ~CMesh () (Destructor)
// Desc : CMesh Class Destructor
//-----------------------------------------------------------------------------
CMesh::~CMesh()
{
	// Release our mesh components
    if ( m_pPolygon ) 
    {
        // Delete all individual polygons in the array.
        for ( ULONG i = 0; i < m_nPolygonCount; i++ )
        {
            if ( m_pPolygon[i] ) delete m_pPolygon[i];
        
        } // Next Polygon

        // Free up the array itself
        delete []m_pPolygon;
    
    } // End if

    // Clear variables
    m_pPolygon      = NULL;
    m_nPolygonCount = 0;
}

//-----------------------------------------------------------------------------
// Name : AddPolygon()
// Desc : Adds a polygon, or multiple polygons, to this mesh.
// Note : Returns the index for the first polygon added, or -1 on failure.
//-----------------------------------------------------------------------------
long CMesh::AddPolygon( ULONG Count )
{

    CPolygon ** pPolyBuffer = NULL;
    
    // Allocate new resized array
    if (!( pPolyBuffer = new CPolygon*[ m_nPolygonCount + Count ] )) return -1;

    // Clear out slack pointers
    ZeroMemory( &pPolyBuffer[ m_nPolygonCount ], Count * sizeof( CPolygon* ) );

    // Existing Data?
    if ( m_pPolygon )
    {
        // Copy old data into new buffer
        memcpy( pPolyBuffer, m_pPolygon, m_nPolygonCount * sizeof( CPolygon* ) );

        // Release old buffer
        delete []m_pPolygon;

    } // End if
    
    // Store pointer for new buffer
    m_pPolygon = pPolyBuffer;

    // Allocate new polygon pointers
    for ( UINT i = 0; i < Count; i++ )
    {
        // Allocate new poly
        if (!( m_pPolygon[ m_nPolygonCount ] = new CPolygon() )) return -1;

        // Increase overall poly count
        m_nPolygonCount++;

    } // Next Polygon
    
    // Return first polygon
    return m_nPolygonCount - Count;
}

//-----------------------------------------------------------------------------
// Name : CPolygon () (Constructor)
// Desc : CPolygon Class Constructor
//-----------------------------------------------------------------------------
CPolygon::CPolygon()
{
	// Reset / Clear all required values
    m_pVertex       = NULL;
    m_nVertexCount  = 0;
    m_pIndex        = NULL;
    m_nIndexCount   = 0;
    m_nMaterial     = -1;

}

//-----------------------------------------------------------------------------
// Name : CPolygon () (Alternate Constructor)
// Desc : CPolygon Class Constructor, adds specified number of vertices
//-----------------------------------------------------------------------------
CPolygon::CPolygon( USHORT VertexCount, USHORT IndexCount )
{
	// Reset / Clear all required values
    m_pVertex       = NULL;
    m_nVertexCount  = 0;
    m_pIndex        = NULL;
    m_nIndexCount   = 0;
    m_nMaterial     = -1;

    // Add vertices & Indices
    if ( VertexCount > 0 ) AddVertex( VertexCount );
    if ( IndexCount  > 0 ) AddIndex( IndexCount );
}

//-----------------------------------------------------------------------------
// Name : ~CPolygon () (Destructor)
// Desc : CPolygon Class Destructor
//-----------------------------------------------------------------------------
CPolygon::~CPolygon()
{
	// Release our vertices & Indices
    if ( m_pVertex ) delete []m_pVertex;
    if ( m_pIndex  ) delete []m_pIndex;
    
    // Clear variables
    m_pVertex       = NULL;
    m_nVertexCount  = 0;
    m_pIndex        = NULL;
    m_nIndexCount   = 0;
    m_nMaterial     = -1;
}

//-----------------------------------------------------------------------------
// Name : AddVertex()
// Desc : Adds a vertex, or multiple vertices, to this polygon.
// Note : Returns the index for the first vertex added, or -1 on failure.
//-----------------------------------------------------------------------------
long CPolygon::AddVertex( USHORT Count )
{
    CVertex * pVertexBuffer = NULL;
    
    // Allocate new resized array
    if (!( pVertexBuffer = new CVertex[ m_nVertexCount + Count ] )) return -1;

    // Existing Data?
    if ( m_pVertex )
    {
        // Copy old data into new buffer
        memcpy( pVertexBuffer, m_pVertex, m_nVertexCount * sizeof(CVertex) );

        // Release old buffer
        delete []m_pVertex;

    } // End if

    // Store pointer for new buffer
    m_pVertex = pVertexBuffer;
    m_nVertexCount += Count;

    // Return first vertex
    return m_nVertexCount - Count;
}

//-----------------------------------------------------------------------------
// Name : AddIndex()
// Desc : Adds an index, or multiple indices, to this polygon.
// Note : Returns the index for the first vertex index added, or -1 on failure.
//-----------------------------------------------------------------------------
long CPolygon::AddIndex( USHORT Count )
{
    USHORT * pIndexBuffer = NULL;
    
    // Allocate new resized array
    if (!( pIndexBuffer = new USHORT[ m_nIndexCount + Count ] )) return -1;

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
    m_nIndexCount += Count;

    // Return first Index
    return m_nIndexCount - Count;
}