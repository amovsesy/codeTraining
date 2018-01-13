//-----------------------------------------------------------------------------
// File: CObject.h
//
// Desc: This file houses the various object / mesh related classes.
//
// Copyright (c) 1997-2002 Adam Hoult & Gary Simmons. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _COBJECT_H_
#define _COBJECT_H_

//-----------------------------------------------------------------------------
// CObject Specific Includes
//-----------------------------------------------------------------------------
#include "Main.h"

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CVertex (Class)
// Desc : Vertex class used to construct & store vertex components.
//-----------------------------------------------------------------------------
class CVertex
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    CVertex( float fX, float fY, float fZ, const D3DXVECTOR3& vecNormal ) 
        { x = fX; y = fY; z = fZ; Normal = vecNormal; }
    
    CVertex() 
        { x = 0.0f; y = 0.0f; z = 0.0f; Normal = D3DXVECTOR3( 0, 0, 0 ); }

    //-------------------------------------------------------------------------
    // Public Variables for This Class
    //-------------------------------------------------------------------------
    float       x;          // Vertex Position X Component
    float       y;          // Vertex Position Y Component
    float       z;          // Vertex Position Z Component
    D3DXVECTOR3 Normal;     // Vertex Normal
    
};

//-----------------------------------------------------------------------------
// Name : CMesh (Class)
// Desc : Basic mesh class used to store individual mesh data.
//-----------------------------------------------------------------------------
class CMesh
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
             CMesh( ULONG VertexCount, ULONG IndexCount );
	         CMesh();
	virtual ~CMesh();

	//-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
    long        AddVertex    ( ULONG Count = 1 );
    long        AddIndex     ( ULONG Count = 1 );
    HRESULT     BuildBuffers ( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL, bool ReleaseOriginals = true );

    //-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
    ULONG                   m_nVertexCount;     // Number of vertices stored
    CVertex                *m_pVertex;          // Simple temporary vertex array.
    ULONG                   m_nIndexCount;      // Number of indices stored
    USHORT                 *m_pIndex;           // Simple temporary index array
    LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;    // Vertex Buffer to be Rendered
    LPDIRECT3DINDEXBUFFER9  m_pIndexBuffer;     // Index Buffer to be Rendered

    D3DXVECTOR3             m_BoundsMin;        // Bounding box minimum extents
    D3DXVECTOR3             m_BoundsMax;        // Bounding box maximum extents

};

//-----------------------------------------------------------------------------
// Name : CObject (Class)
// Desc : Mesh container class used to store instances of meshes.
//-----------------------------------------------------------------------------
class CObject
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
     CObject( CMesh * pMesh );
	 CObject();

	//-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
    D3DXMATRIX  m_mtxWorld;             // Objects world matrix
    CMesh      *m_pMesh;                // Mesh we are instancing

};


#endif // !_COBJECT_H_