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
    m_pVertexBuffer = NULL;
    D3DXMatrixIdentity( &m_mtxWorld );
}

//-----------------------------------------------------------------------------
// Name : CObject () (Alternate Constructor)
// Desc : CObject Class Constructor, sets the internal mesh object
//-----------------------------------------------------------------------------
CObject::CObject( LPDIRECT3DVERTEXBUFFER9 pVertexBuffer )
{
	// Reset / Clear all required values
    D3DXMatrixIdentity( &m_mtxWorld );

    
}

//-----------------------------------------------------------------------------
// Name : CObject () (Destructor)
// Desc : CObject Class Destructor
//-----------------------------------------------------------------------------
CObject::~CObject( )
{
    // Release our vertex buffer (de-reference)
    if ( m_pVertexBuffer ) m_pVertexBuffer->Release();
    m_pVertexBuffer = NULL;
}

//-----------------------------------------------------------------------------
// Name : SetVertexBuffer ()
// Desc : Allows us to set the vertex buffer referenced by this object
//-----------------------------------------------------------------------------
void CObject::SetVertexBuffer( LPDIRECT3DVERTEXBUFFER9 pVertexBuffer )
{
    // Release previous vertex buffer
    if ( m_pVertexBuffer ) m_pVertexBuffer->Release();

    // Set the nwq vertex Buffer
    m_pVertexBuffer = pVertexBuffer;

    // Add a reference to it if we didn't pass in NULL
    if ( m_pVertexBuffer ) m_pVertexBuffer->AddRef();
}