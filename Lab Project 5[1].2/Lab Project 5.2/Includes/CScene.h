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
#include <vector>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class CFileIWF;
class iwfMesh;
class iwfSurface;
class CLightGroup;
class CPropertyGroup;
class CVertex;

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
    void            SetD3DDevice( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL );
    bool            LoadScene   ( TCHAR * strFileName, ULONG LightLimit = 0, ULONG LightReservedCount = 0 );
    void            Release     ( );
    void            Render      ( );
    
    //-------------------------------------------------------------------------
    // Public Variables for This Class
    //-------------------------------------------------------------------------
    D3DMATERIAL9   *m_pMaterialList;    // Array of material structures.
    D3DLIGHT9      *m_pLightList;       // Array of light structures
    D3DLIGHT9       m_DynamicLight;     // Single dynamic light for testing.
    CLightGroup   **m_ppLightGroupList; // Array of individual lighting groups
    ULONG           m_nMaterialCount;   // Number of materials stored
    ULONG           m_nLightCount;      // Number lights stored here
    ULONG           m_nLightGroupCount; // Number of light groups stored here.
    
private:
    //-------------------------------------------------------------------------
    // Private FUnctions for This Class
    //-------------------------------------------------------------------------
    bool            ProcessMeshes        ( CFileIWF & pFile );
    bool            ProcessVertices      ( CLightGroup * pLightGroup, CPropertyGroup *pProperty, iwfSurface * pFilePoly );
    bool            ProcessIndices       ( CLightGroup * pLightGroup, CPropertyGroup *pProperty, iwfSurface * pFilePoly );
    bool            ProcessMaterials     ( const CFileIWF& File );
    bool            ProcessEntities      ( const CFileIWF& File );
    float           GetLightContribution ( iwfSurface * pSurface, D3DLIGHT9 * pLight );
    long            AddLightGroup        ( ULONG Count );
    bool            BuildLightGroups     ( std::vector<iwfSurface*> & SurfaceList, long MaterialIndex );

    //-------------------------------------------------------------------------
    // Private Variables for This Class
    //-------------------------------------------------------------------------
    ULONG               m_nReservedLights;  // Number of light slots to leave empty
    ULONG               m_nLightLimit;      // Number of device lights available.
    LPDIRECT3DDEVICE9   m_pD3DDevice;       // Direct3D Device used for rendering / initialization
    bool                m_bHardwareTnL;     // Objects should be build taking into account TnL
};

//-----------------------------------------------------------------------------
// Name : CLightGroup (Class)
// Desc : Stores information about which meshes are affected by which lights.
//-----------------------------------------------------------------------------
class CLightGroup
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
     CLightGroup( );
    ~CLightGroup( );

    //-------------------------------------------------------------------------
    // Public Functions for This Class
    //-------------------------------------------------------------------------
    bool            SetLights        ( ULONG LightCount, ULONG LightList[] );
    bool            GroupMatches     ( ULONG LightCount, ULONG LightList[] ) const;
    long            AddPropertyGroup ( USHORT Count = 1 );
    long            AddVertex        ( USHORT Count = 1 );
    bool            BuildBuffers     ( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL, bool ReleaseOriginals = false );
    
    //-------------------------------------------------------------------------
    // Public Variables for This Class
    //-------------------------------------------------------------------------
    ULONG           m_nLightCount;              // Number of lights in this group
    ULONG          *m_pLightList;               // Lights to be set active in this group.

    USHORT          m_nPropertyGroupCount;      // Number of property groups stored
    USHORT          m_nVertexCount;             // Number of vertices stored.
    CPropertyGroup**m_pPropertyGroup;           // Simple array of property groups.
    CVertex        *m_pVertex;                  // Simple vertex array

    LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;    // Vertex Buffer

};

//-----------------------------------------------------------------------------
// Name : CPropertyGroup (Class)
// Desc : Property group class stores rendering data grouped by property.
//-----------------------------------------------------------------------------
class CPropertyGroup
{
public:
    //-------------------------------------------------------------------------
	// Enumerators for this class
	//-------------------------------------------------------------------------
    enum PROPERTY_TYPE { PROPERTY_NONE = 0, PROPERTY_MATERIAL = 1, PROPERTY_TEXTURE = 2 };

    //-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
	                CPropertyGroup();
	virtual        ~CPropertyGroup();

	//-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
    long            AddPropertyGroup ( USHORT Count = 1 );
    long            AddIndex         ( USHORT Count = 1 );
    bool            BuildBuffers     ( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL, bool ReleaseOriginals = false );

    //-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
    PROPERTY_TYPE    m_PropertyType;         // Type of property this is.
    ULONG            m_nPropertyData;        // 32 bit property data value.
    USHORT           m_nIndexCount;          // Number of indices stored
    USHORT           m_nPropertyGroupCount;  // Number of child properties
    USHORT          *m_pIndex;               // Simple index array
    CPropertyGroup **m_pPropertyGroup;       // Array of child properties.
    USHORT           m_nVertexStart;         // First vertex used in the mesh vertex array
    USHORT           m_nVertexCount;         // Number of vertices used in the mesh vertex array

    LPDIRECT3DINDEXBUFFER9  m_pIndexBuffer;  // Direct3D Index Buffer
};

#endif // !_CSCENE_H_