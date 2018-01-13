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
#define VERTEX_FVF     D3DFVF_XYZ | D3DFVF_TEX2
#define LITVERTEX_FVF  D3DFVF_XYZ | D3DFVF_DIFFUSE

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
    CVertex( float fX, float fY, float fZ, float ftu = 0.0f, float ftv = 0.0f ) 
        { x = fX; y = fY; z = fZ; tu = ftu; tv = ftv; }
    
    CVertex() 
        { x = 0.0f; y = 0.0f; z = 0.0f; tu = 0.0f; tv = 0.0f; }

    //-------------------------------------------------------------------------
    // Public Variables for This Class
    //-------------------------------------------------------------------------
    float       x;          // Vertex Position X Component
    float       y;          // Vertex Position Y Component
    float       z;          // Vertex Position Z Component
    float       tu;         // Texture u coordinate
    float       tv;         // Texture v coordinate
    float       tu2;        // 2nd Texture U coordinate
    float       tv2;        // 2nd Texture V coordinate
};

//-----------------------------------------------------------------------------
// Name : CLitVertex (Class)
// Desc : Vertex class used to construct & store vertex components.
//-----------------------------------------------------------------------------
class CLitVertex
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    CLitVertex( float fX, float fY, float fZ, ULONG ulDiffuse = 0xFF000000 ) 
        { x = fX; y = fY; z = fZ; Diffuse = ulDiffuse; }
    
    CLitVertex() 
        { x = 0.0f; y = 0.0f; z = 0.0f; Diffuse = 0xFF000000; }

    //-------------------------------------------------------------------------
    // Public Variables for This Class
    //-------------------------------------------------------------------------
    float       x;          // Vertex Position X Component
    float       y;          // Vertex Position Y Component
    float       z;          // Vertex Position Z Component
    ULONG       Diffuse;    // Diffuse vertex colour component
    
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
    void        SetVertexFormat ( ULONG FVFCode, UCHAR Stride );
    long        AddVertex       ( ULONG Count = 1 );
    long        AddIndex        ( ULONG Count = 1 );
    HRESULT     BuildBuffers    ( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL, bool ReleaseOriginals = true );

    //-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
    ULONG                   m_nVertexCount;     // Number of vertices stored
    UCHAR                  *m_pVertex;          // Simple temporary vertex array of any format
    ULONG                   m_nIndexCount;      // Number of indices stored
    USHORT                 *m_pIndex;           // Simple temporary index array
    LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;    // Vertex Buffer to be Rendered
    LPDIRECT3DINDEXBUFFER9  m_pIndexBuffer;     // Index Buffer to be Rendered
    UCHAR                   m_nStride;          // The stride of each individual vertex
    ULONG                   m_nFVFCode;         // The flexible vertex format code.

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