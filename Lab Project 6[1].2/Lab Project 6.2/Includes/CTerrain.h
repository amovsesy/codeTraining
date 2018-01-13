//-----------------------------------------------------------------------------
// File: CTerrain.h
//
// Desc: This class loads, builds & stores the individual terrain block meshes
//       and essentially wraps the rendering loop required.
//
// Copyright (c) 1997-2002 Daedalus Developments. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _CTERRAIN_H_
#define _CTERRAIN_H_

//-----------------------------------------------------------------------------
// CTerrain Specific Includes
//-----------------------------------------------------------------------------
#include "Main.h"

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class CMesh;
class CPlayer;
class CCamera;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CTerrain (Class)
// Desc : Game Timer class, queries performance hardware if available, and 
//        calculates all the various values required for frame rate based
//        vector / value scaling.
//-----------------------------------------------------------------------------
class CTerrain
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class
    //-------------------------------------------------------------------------
	         CTerrain();
	virtual ~CTerrain();

	//-------------------------------------------------------------------------
	// Public Functions For This Class
	//-------------------------------------------------------------------------
    void            SetRenderMode ( bool bSinglePass );
    void            SetD3DDevice  ( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL );
    bool            LoadHeightMap ( LPCTSTR FileName, ULONG Width, ULONG Height );
    float           GetHeight     ( float x, float z, bool ReverseQuad = false );
    void            Render        ( CCamera * pCamera );
    void            Release       ( );

    //-------------------------------------------------------------------------
	// Public Static Functions For This Class
	//-------------------------------------------------------------------------
    static void     UpdatePlayer  ( LPVOID pContext, CPlayer * pPlayer, float TimeScale );
    static void     UpdateCamera  ( LPVOID pContext, CCamera * pCamera, float TimeScale );

private:
	//-------------------------------------------------------------------------
	// Private Variables For This Class
	//-------------------------------------------------------------------------
    D3DXVECTOR3         m_vecScale;         // Amount to scale the terrain meshes
    UCHAR              *m_pHeightMap;       // The physical heightmap data loaded
    ULONG               m_nHeightMapWidth;  // Width of the 2D heightmap data
    ULONG               m_nHeightMapHeight; // Height of the 2D heightmap data
    
    CMesh             **m_pMesh;            // Simple array of mesh pointers
    ULONG               m_nMeshCount;       // Number of meshes stored here
    LPDIRECT3DDEVICE9   m_pD3DDevice;       // D3D Device to use for creation / rendering.
    bool                m_bHardwareTnL;     // Used hardware vertex processing ?
    bool                m_bSinglePass;      // Use single pass rendering method (multiple stage)

    ULONG               m_nPrimitiveCount;  // Pre-Calculated. Number of primitives for D3D Render

    LPDIRECT3DTEXTURE9  m_pBaseTexture;     // Base terrain texture
    LPDIRECT3DTEXTURE9  m_pDetailTexture;   // Terrain detail texture.
    
	//-------------------------------------------------------------------------
	// Private Functions For This Class
	//-------------------------------------------------------------------------
    long            AddMesh             ( ULONG Count = 1 );
    bool            BuildMeshes         ( );
    D3DXVECTOR3     GetHeightMapNormal  ( ULONG x, ULONG z );
    
    
};

#endif // _CTERRAIN_H_