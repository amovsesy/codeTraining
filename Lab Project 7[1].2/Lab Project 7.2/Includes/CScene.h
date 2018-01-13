//-----------------------------------------------------------------------------
// File: CScene.h
//
// Desc: Contains scene related data such as meshes, lights, materials etc.
//
// Copyright (c) 1997-2002 Adam Hoult & Gary Simmons. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _CSCENE_H_
#define _CSCENE_H_

//-----------------------------------------------------------------------------
// CScene Specific Includes
//-----------------------------------------------------------------------------
#include "Main.h"
#include "CObject.h"
#include <vector>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class CFileIWF;
class iwfSurface;
class CTimer;
class CMesh;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CScene (Class)
// Desc : Scene class used to store and manipulate scene specific data.
//-----------------------------------------------------------------------------
class CScene
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
     CScene( );
    ~CScene( );

    //-------------------------------------------------------------------------
    // Public Functions for This Class
    //-------------------------------------------------------------------------
    void                SetD3DDevice    ( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL );
    void                SetTextureFormat( const D3DFORMAT & Format, const D3DFORMAT & AlphaFormat );
    bool                LoadScene       ( TCHAR * strFileName );
    void                Release         ( );
    void                AnimateObjects  ( CTimer & Timer );
    void                Render          ( );
    
    //-------------------------------------------------------------------------
    // Public Variables for This Class
    //-------------------------------------------------------------------------
    LPDIRECT3DTEXTURE9 *m_pTextureList;     // Array of texture pointers
    ULONG               m_nTextureCount;    // Number of textures stored
    CMesh             **m_ppMeshList;       // A list of all loaded meshes
    ULONG               m_nMeshCount;       // Number of meshes loaded.
    CObject             m_pObject[2];       // Store our 2 objects

private:
    //-------------------------------------------------------------------------
    // Private FUnctions for This Class
    //-------------------------------------------------------------------------
    bool                ProcessMeshes        ( CFileIWF & pFile );
    bool                ProcessVertices      ( CMesh * pMesh, iwfSurface * pFilePoly );
    bool                ProcessIndices       ( CMesh * pMesh, iwfSurface * pFilePoly );
    bool                ProcessTextures      ( const CFileIWF& File );
    long                AddMesh              ( ULONG Count = 1 );

    //-------------------------------------------------------------------------
    // Private Variables for This Class
    //-------------------------------------------------------------------------
    LPDIRECT3DDEVICE9   m_pD3DDevice;       // Direct3D Device used for rendering / initialization
    bool                m_bHardwareTnL;     // Objects should be build taking into account TnL
    D3DFORMAT           m_fmtTexture;       // Texture format to use when building textures.
    D3DFORMAT           m_fmtAlpha;         // Alpha texture format to use when building textures.
};

#endif // !_CSCENE_H_