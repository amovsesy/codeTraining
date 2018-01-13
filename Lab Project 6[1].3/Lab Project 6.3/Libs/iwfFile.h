///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//          _ _ _    _____        _______   ____       _                     //
//         | (_) |__|_ _\ \      / /  ___| | __ )  ___| |_ __ _              //
//         | | | '_ \| | \ \ /\ / /| |_    |  _ \ / _ \ __/ _` |             //
//         | | | |_) | |  \ V  V / |  _|   | |_) |  __/ || (_| |             //
//         |_|_|_.__/___|  \_/\_/  |_|     |____/ \___|\__\__,_|             //
//                                                          v0.0.2           //
//                                                                           //
//---------------------------------------------------------------------------//
//                                                                           //
//                           File: iwfFile.h                                 //
//                                                                           //
//                           Date: 26/08/2002                                //
//                                                                           //
//            Desc: Performs complete file loading and storage.              //
//                                                                           //
//   Copyright (c) 1997-2002 Daedalus Developments. All rights reserved.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _IWFFILE_H_
#define _IWFFILE_H_

//-----------------------------------------------------------------------------
// iwfFile Specific Includes
//-----------------------------------------------------------------------------
#include "libIWF.h"
#include <vector>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class iwfMesh;
class iwfMaterial;
class iwfEntity;
class iwfGroup;

//-----------------------------------------------------------------------------
// Load Flag Definitions
//-----------------------------------------------------------------------------
#define IWF_RETAIN_GROUPS   1

//-----------------------------------------------------------------------------
// Typedefs Structures & Enumerators
//-----------------------------------------------------------------------------
typedef struct _PARENTINFO {         // Parent level information for recursion
    USHORT  ChunkID;                 // Chunk ID of parent level
    ULONG   Data1;                   // A user assignable custom 4 byte value
    ULONG   Data2;                   // A user assignable custom 4 byte value.
} PARENTINFO;

typedef std::vector<iwfGroup*>      vectorIWFGroup;
typedef std::vector<iwfMesh*>       vectorIWFMesh;
typedef std::vector<TEXTURE_REF*>   vectorIWFTexture;
typedef std::vector<SHADER_REF*>    vectorIWFShader;
typedef std::vector<iwfMaterial*>   vectorIWFMaterial;
typedef std::vector<iwfEntity*>     vectorIWFEntity;

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CFileIWF (Class)
// Desc : This is the IWF class, derived from the base IWF structure class,
//        which handles all our specific IWF reading & writing requirements.
// Note : Nasty Nasty Nasty hacked together save overloads :(
//-----------------------------------------------------------------------------
class CFileIWF : public CBaseIWF
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
             CFileIWF();
    virtual ~CFileIWF();

    //-------------------------------------------------------------------------
    // Public Functions for This Class.
    //-------------------------------------------------------------------------
    void            Load( LPCTSTR FileName, ULONG Flags = 0 );
    void            ClearObjects();

    //-------------------------------------------------------------------------
    // Public Variables for This Class.
    //-------------------------------------------------------------------------
    vectorIWFMesh      m_vpMeshList;       // A list of all meshes loaded.
    vectorIWFTexture   m_vpTextureList;    // A list of all textures loaded.
    vectorIWFMaterial  m_vpMaterialList;   // A list of all materials loaded.
    vectorIWFEntity    m_vpEntityList;     // A list of all entities loaded.
    vectorIWFShader    m_vpShaderList;     // A list of all shaders loaded.
    vectorIWFGroup     m_vpGroupList;      // A list of all groups loaded.

private:
    //-------------------------------------------------------------------------
    // Private Static CHUNKPROC Callbacks Registered for This Class.
    //-------------------------------------------------------------------------
    static void     ReadGroup    ( LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData );
    static void     ReadMesh     ( LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData );
    static void     ReadSurfaces ( LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData );
    static void     ReadVertices ( LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData );
    static void     ReadIndices  ( LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData );
    static void     ReadEntity   ( LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData );
    static void     ReadTextures ( LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData );
    static void     ReadMaterials( LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData );
    static void     ReadShaders  ( LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData );

    //-------------------------------------------------------------------------
    // Private Variables for This Class.
    //-------------------------------------------------------------------------
    ULONG           m_LoadFlags;                // Flags to use during loadup
    bool            m_bLoadedVNormals;          // Mesh uses this as an informant

};

#endif // _IWFFILE_H_