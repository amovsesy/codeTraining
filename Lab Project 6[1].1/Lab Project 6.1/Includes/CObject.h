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
#define VERTEX_FVF      D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1

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
    CVertex( float fX, float fY, float fZ, ULONG ulDiffuse = 0xFF000000, float ftu = 0.0f, float ftv = 0.0f ) 
        { x = fX; y = fY; z = fZ; Diffuse = ulDiffuse; tu = ftu; tv = ftv; }
    
    CVertex() 
        { x = 0.0f; y = 0.0f; z = 0.0f; Diffuse = 0xFF000000; tu = 0.0f; tv = 0.0f; }

    //-------------------------------------------------------------------------
    // Public Variables for This Class
    //-------------------------------------------------------------------------
    float       x;          // Vertex Position X Component
    float       y;          // Vertex Position Y Component
    float       z;          // Vertex Position Z Component
    ULONG       Diffuse;    // Diffuse vertex colour component
    float       tu;         // Texture u coordinate
    float       tv;         // Texture v coordinate
    
};

//-----------------------------------------------------------------------------
// Name : CObject (Class)
// Desc : Container class used to store instances of renderable objects.
//-----------------------------------------------------------------------------
class CObject
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
             CObject( LPDIRECT3DVERTEXBUFFER9 pVertexBuffer );
	         CObject( );
    virtual ~CObject( );

    //-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
    void    SetVertexBuffer ( LPDIRECT3DVERTEXBUFFER9 pVertexBuffer );

	//-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
    D3DXMATRIX              m_mtxWorld;         // Objects world matrix
    LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;    // Vertex Buffer we are instancing

};


#endif // !_COBJECT_H_