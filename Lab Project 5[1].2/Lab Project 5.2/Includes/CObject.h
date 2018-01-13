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
// Definitions, constants and enumerators
//-----------------------------------------------------------------------------
#define VERTEX_FVF      D3DFVF_XYZ | D3DFVF_NORMAL

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
// Name : CPolygon (Class)
// Desc : Basic polygon class used to store this polygons vertex data.
//-----------------------------------------------------------------------------
class CPolygon
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
             CPolygon( USHORT VertexCount, USHORT IndexCount = 0 );
	         CPolygon();
	virtual ~CPolygon();

	//-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
    long        AddVertex( USHORT Count = 1 );
    long        AddIndex( USHORT Count = 1 );

    //-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
    short       m_nMaterial;            // Material index to use for this poly (-1 for none)
    USHORT      m_nVertexCount;         // Number of vertices stored.
    USHORT      m_nIndexCount;          // Number of indices stored
    USHORT     *m_pIndex;               // Simple index array
    CVertex    *m_pVertex;              // Simple vertex array
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
             CMesh( ULONG Count );
	         CMesh();
	virtual ~CMesh();

	//-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
    long        AddPolygon( ULONG Count = 1 );

    //-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
    ULONG       m_nPolygonCount;        // Number of polygons stored
    CPolygon  **m_pPolygon;             // Simply polygon array.
    
    CMesh      *m_pNext;                // Linked list connectivity (if required)

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