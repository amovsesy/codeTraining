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
//                            File: libIWF.h                                 //
//                                                                           //
//                           Date: 26/08/2002                                //
//                                                                           //
//          Desc: This file houses the base IWF structure support.           //
//                                                                           //
//   Copyright (c) 1997-2002 Daedalus Developments. All rights reserved.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#ifndef _LIBIWF_H_
#define _LIBIWF_H_

//-----------------------------------------------------------------------------
// libIWF Specific Includes
//-----------------------------------------------------------------------------
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>

//-----------------------------------------------------------------------------
// Miscellaneous Macros
//-----------------------------------------------------------------------------
#define MAKEVERSION( Major, Minor, Revision ) \
                    ((Major & 0xFF) << 24) +  \
                    ((Minor & 0xFF) << 16) +  \
                    (Revision & 0xFFFF)

//-----------------------------------------------------------------------------
// Definitions and Constants
//-----------------------------------------------------------------------------
const UCHAR FILE_SIG_SIZE            = 4;
const UCHAR FILE_SIG[FILE_SIG_SIZE]  = { 'I', 'I', 'W', 'F' };
const UCHAR HEAD_SIG_SIZE            = 4;
const UCHAR HEAD_SIG[HEAD_SIG_SIZE]  = { 'H', 'E', 'A', 'D' };
const UCHAR FOOT_SIG_SIZE            = 4;
const UCHAR FOOT_SIG[FOOT_SIG_SIZE]  = { 'F', 'O', 'O', 'T' };

//-----------------------------------------------------------------------------
// Error Code Definitions
//-----------------------------------------------------------------------------
#define FACILITY_IWF                        0x967              // IWF Error Facility Code
#define IWF_ERR_INVALIDFORMAT               MAKE_HRESULT(1, FACILITY_IWF, 7000)
#define IWF_ERR_INVALIDPROCESSMODE          MAKE_HRESULT(1, FACILITY_IWF, 7001)
#define IWF_ERR_CHUNKSEXHAUSTED             MAKE_HRESULT(1, FACILITY_IWF, 7002)
#define IWF_ERR_INVALIDSIGNATURE            MAKE_HRESULT(1, FACILITY_IWF, 7003)
#define IWF_ERR_NOCHILDREN                  MAKE_HRESULT(1, FACILITY_IWF, 7004)
#define IWF_ERR_MALFORMED                   MAKE_HRESULT(1, FACILITY_IWF, 7005)
#define IWF_ERR_CHUNKTYPELOCKED             MAKE_HRESULT(1, FACILITY_IWF, 7006)
#define IWF_ERR_INVALIDPARAMS               MAKE_HRESULT(1, FACILITY_IWF, 7007)
#define IWF_ERR_OUTOFMEMORY                 MAKE_HRESULT(1, FACILITY_IWF, 7008)
#define IWF_ERR_ACCESSDENIED                MAKE_HRESULT(1, FACILITY_IWF, 7009)
#define IWF_ERR_FILENOTOPEN                 MAKE_HRESULT(1, FACILITY_IWF, 7010)

//-----------------------------------------------------------------------------
// File Header Flags
//-----------------------------------------------------------------------------
#define IWF_COMPRESSED                      0x1     // File is compressed
#define IWF_FILESUMMARY                     0x2     // Contains summary chunk
#define IWF_CRYPT_ARCFOUR                   0x4     // Chunks use ARCfour Encryption

//-----------------------------------------------------------------------------
// Reserved Standards Chunks
//-----------------------------------------------------------------------------
#define CHUNK_ROOT                          0x0000
#define CHUNK_FILESUMMARY                   0x0001
#define CHUNK_CONTAINER                     0x0002
#define CHUNK_THUMBNAIL                     0x0003
#define CHUNK_GROUP                         0x0010
#define CHUNK_MESH                          0x0020
#define CHUNK_DECALMESH                     0x0021
#define CHUNK_SURFACES                      0x0030
#define CHUNK_INDICES                       0x0040
#define CHUNK_VERTEXCOMPONENTS              0x0050
#define CHUNK_ENTITY                        0x0080
#define CHUNK_MATERIALS                     0x0090
#define CHUNK_TEXTURES                      0x00A0
#define CHUNK_SHADERS                       0x00B0
#define CHUNK_COORDSYSINFO                  0x00C0

//-----------------------------------------------------------------------------
// Reserved Flags for Standards Chunks
//-----------------------------------------------------------------------------
#define MCOMPONENT_OMATRIX                  0x1
#define MCOMPONENT_BOUNDS                   0x2
#define MCOMPONENT_STYLE                    0x4
#define MCOMPONENT_CUSTOMDATA               0x8
#define MCONPONENT_ALL                      0xF

#define MESH_DETAIL                         0x1
#define MESH_DYNAMIC                        0x2
#define MESH_DESTRUCTIBLE                   0x4

#define SCOMPONENT_NORMAL                   0x1
#define SCOMPONENT_ZBIAS                    0x2
#define SCOMPONENT_EFFECT                   0x4
#define SCOMPONENT_STYLE                    0x8
#define SCOMPONENT_TEXTURES                 0x10
#define SCOMPONENT_MATERIALS                0x20
#define SCOMPONENT_SHADERS                  0x40
#define SCOMPONENT_BLENDMODES               0x80
#define SCOMPONENT_CUSTOMDATA               0x100
#define SCOMPONENT_ALL                      0x1FF

#define SURFACE_INVISIBLE					0x1
#define SURFACE_FULLBRIGHT			        0x2
#define SURFACE_SKYTYPE					    0x4
#define SURFACE_TWO_SIDED                   0x8

#define VPOOL_NONE                          0x0
#define VPOOL_SURFACE                       0x1
#define VPOOL_MESH                          0x2
#define VPOOL_GLOBAL                        0x3

#define INDICES_NONE                        0x0
#define INDICES_16BIT                       0x1
#define INDICES_32BIT                       0x2
#define INDICES_TRISTRIP                    0x4
#define INDICES_TRILIST                     0x8
#define INDICES_TRIFAN                      0x10
#define INDICES_LINESTRIP                   0x20
#define INDICES_LINELIST                    0x40
#define INDICES_POINTLIST                   0x80

#define INDICES_MASK_TYPE                   0xFC

#define VERTICES_NONE                       0x0
#define VERTICES_NON_PLANAR                 0x1
#define VERTICES_INDEXED                    0x2
#define VERTICES_TRISTRIP                   0x4
#define VERTICES_TRILIST                    0x8
#define VERTICES_TRIFAN                     0x10
#define VERTICES_LINESTRIP                  0x20
#define VERTICES_LINELIST                   0x40
#define VERTICES_POINTLIST                  0x80

#define VERTICES_MASK_TYPE                  0xFE

#define VCOMPONENT_XYZ                      0x1
#define VCOMPONENT_RHW                      0x2
#define VCOMPONENT_XYZRHW                   0x3
#define VCOMPONENT_NORMAL                   0x4
#define VCOMPONENT_POINTSIZE                0x8
#define VCOMPONENT_DIFFUSE                  0x10
#define VCOMPONENT_SPECULAR                 0x20
#define VCOMPONENT_TEXCOORDS                0x40
#define VCOMPONENT_TEXCOORDLAYOUT           0x80
#define VCOMPONENT_ALL                      0xFF

#define SHCOMPONENT_VSHADER					0x1
#define SHCOMPONENT_PSHADER					0x2

#define SCRIPT_EXTERNAL                     0x1
#define SCRIPT_INTERNAL                     0x2
#define SCRIPT_COMPILED                     0x4

#define TEXTURE_EXTERNAL                    0x1
#define TEXTURE_INTERNAL                    0x2

#define TEXFORMAT_RAW						0x1
#define TEXFORMAT_CUSTOM					0x2
#define TEXFORMAT_BMP						0x3
#define TEXFORMAT_JPEG						0x4
#define TEXFORMAT_TGA						0x5
#define TEXFORMAT_PNG						0x6
#define TEXFORMAT_PCX						0x7
#define TEXFORMAT_GIF						0x8
#define TEXFORMAT_PSD						0x9
#define TEXFORMAT_TIFF						0xA
#define TEXFORMAT_PPM						0xB

#define ENTITY_LIGHT                        0x0010
#define ENTITY_SPAWN_POINT                  0x0030
#define ENTITY_SIMPLE_POINT                 0x0031

#define LIGHTTYPE_POINT						0x0
#define LIGHTTYPE_SPOT						0x1
#define LIGHTTYPE_DIRECTIONAL				0x2
#define LIGHTTYPE_AMBIENT					0x3

#define SPAWNTYPE_RESPAWNABLE				0x1
#define SPAWNTYPE_SINGLEPLAYER				0x2
#define SPAWNTYPE_MULTIPLAYER				0x4
#define SPAWNTYPE_NON_PLAYER				0x8
#define SPAWNTYPE_ITEM						0x10

#define SPCOMPONENT_RESPAWN					0x1
#define SPCOMPONENT_REFNAME					0x2
#define SPCOMPONENT_SCRIPT					0x4
#define SPCOMPONENT_ALL						0x7

//-----------------------------------------------------------------------------
// Reserved End Of Chunk Codes.
//-----------------------------------------------------------------------------
#define CHUNK_EOC                           0xf000
#define CHUNK_EOC_EXIT                      0xf001

//-----------------------------------------------------------------------------
// Typedefs, Structures & Enumerators
//-----------------------------------------------------------------------------
typedef struct _IWFFILEHEADER {             // IWF File Header Chunk.
    UCHAR       Signature[FILE_SIG_SIZE];   // File type signature 
    ULONG       Version;                    // File types version ID.
    ULONG       Flags;                      // File Flags (i.e. IWF_COMPRESSED etc )
    ULONG       Checksum;                   // File data checksum
    ULONG       Reserved1;                  // Four bytes of reserved data
    ULONG       Reserved2;                  // Four bytes of reserved data
} IWFFILEHEADER;

typedef struct _CHUNKHEADER {               // Main IWF File chunk header
    UCHAR       Signature[HEAD_SIG_SIZE];   // Chunk header validation signature
    USHORT      ChunkTypeID;                // Type Identifier for the chunk data
    ULONG       ChunkLength;                // Length of this specific chunk (excluding header).
    long        DataLength;                 // Length of the non child chunk data stored in this chunk
    UCHAR       AuthorIDLength;             // Length of the author ID code
    UCHAR       AuthorID[255];              // Actual author ID data.
} CHUNKHEADER;

typedef struct _CHUNKFOOTER {               // Main IWF File chunk footer
    UCHAR       Signature[FOOT_SIG_SIZE];   // Chunk footer validation signature
    USHORT      EOCValue;                   // End of chunk 'signal code' value
} CHUNKFOOTER;

typedef struct _CHUNKDETAILS {              // Information for the chunks in process (manual stacking)
    CHUNKHEADER     Header;                 // This particular chunks header
    __int64         Position;               // The position within the stream the chunk starts
    __int64         LastEOCPos;             // The last child EOC written to file (if any)
    LPVOID          CustomData;             // Custom data pointer assigned by user while stepping in.
    _CHUNKDETAILS  *Child;                  // The child chunk detail item in the 'linked' list
    _CHUNKDETAILS  *Parent;                 // The parent chunk detail item in the 'linked' list.
} CHUNKDETAILS;

typedef void (*CHUNKPROC)( LPVOID pContext, CHUNKHEADER& Header, LPVOID pCustomData );

typedef struct _CHUNKCALLBACK {             // Stores information about the registered chunk callbacks
    USHORT          ChunkTypeID;            // The TypeID associated with this chunk.
    CHUNKPROC       pCallback;              // This particular chunks header.
    LPVOID          pContext;               // The context value sent to the callback
    _CHUNKCALLBACK *Next;                   // Linked list next pointer.
} CHUNKCALLBACK;

//-----------------------------------------------------------------------------
// Typedefs, Structures & Enumerators for Standard Chunks
//-----------------------------------------------------------------------------
typedef struct _PAL_ENTRY {                 // Individual Palette Entry 
    UCHAR       Red;                        // Red Colour Component
    UCHAR       Green;                      // Green Colour Component
    UCHAR       Blue;                       // Blue Colour Component
} PAL_ENTRY;

typedef struct _SCRIPT_REF {                // Structure to reference a script
    UCHAR       ScriptSource;               // Information about where the script is
    USHORT      ScriptSize;                 // Size of the ScriptData array
    char        *ScriptData;                // The data being referenced
} SCRIPT_REF;

typedef struct _BLEND_MODE {                // Specifies the blending mode to use for rendering
    UCHAR       SrcBlendMode;               // Source Blend Mode
    UCHAR       DestBlendMode;              // Destination Blend Mode
} BLEND_MODE;

typedef struct _TEXTURE_REF {               // Structure to reference a texture
    UCHAR       TextureSource;              // Information about where the texture is
    USHORT      NameLength;                 // The length of the file or reference name
    char       *Name;                       // The filename or reference name for this texture
    UCHAR       TextureFormat;              // The format of the internal texture if used
    USHORT      TextureSize;                // Size of the TextureData array
    UCHAR      *TextureData;                // The data being referenced
} TEXTURE_REF;

typedef struct _SHADER_REF {                // Structure to reference a texture
    ULONG       Components;                 // Components contained in this shader reference
    SCRIPT_REF  VertexShader;               // Vertex shader script reference
    SCRIPT_REF  PixelShader;                // Pixel shader script reference
} SHADER_REF;

typedef struct _COLOUR_VALUE {              // Used to store material / light colour values
    float       r;                          // The red colour component
    float       g;                          // The green colour component
    float       b;                          // The blue colour component
    float       a;                          // The alpha colour component
} COLOUR_VALUE;

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CBaseIWF (Class)
// Desc : This is the base IWF class. It provides all the base IWF structure
//        support for both reading and writing. This is designed for you to
//        derive from so that you can insert all your chunk specific code.
//-----------------------------------------------------------------------------
class CBaseIWF
{
public:
    //-------------------------------------------------------------------------
	// Enumerator Definitions for This Class
	//-------------------------------------------------------------------------
    enum IWFPROCESSMODE { MODE_CLOSED = 0, MODE_READ = 1, MODE_WRITE = 2 };

    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
             CBaseIWF( LPCTSTR FileName, IWFPROCESSMODE Mode, IWFFILEHEADER * pHeader = NULL );
             CBaseIWF( );
    virtual ~CBaseIWF( );

    //-------------------------------------------------------------------------
    // Public Functions for This Class.
    //-------------------------------------------------------------------------
    void            SetAuthorID         ( LPVOID pAuthor, UCHAR Length );
    
    void            ProcessIWF          ( LPVOID pCustomData = NULL );
    void            ReadChunkHeader     ( CHUNKHEADER * pHeader );
    void            ReadChunkFooter     ( CHUNKFOOTER * pFooter );

    void            OpenChunk           ( USHORT ChunkID );
    void            CloseChunk          ( );

    void            WriteChunkHeader    ( CHUNKHEADER * pHeader );
    void            WriteChunkFooter    ( CHUNKFOOTER * pFooter );
    void            WriteFileSummary    ( );

    bool            RegisterChunkProc   ( USHORT ChunkID, CHUNKPROC pFunc, LPVOID pContext );
    void            UnregisterChunkProc ( USHORT ChunkID );
    void            ClearChunkProcs     ( );

    //-------------------------------------------------------------------------
    // Public Virtual Functions for This Class.
    //-------------------------------------------------------------------------
    virtual void    Open                ( LPCTSTR FileName, IWFPROCESSMODE Mode, IWFFILEHEADER * pHeader = NULL );
    virtual void    ProcessChunk        ( CHUNKHEADER& Header, LPVOID pCustomData );
    virtual void    Close               ( );
    
    //-------------------------------------------------------------------------
    // Public Static Functions for This Class.
    //-------------------------------------------------------------------------
    static bool     IsValid             ( LPCTSTR FileName );
    static bool     IsStandardChunk     ( USHORT ChunkTypeID );
    static bool     IsLockedChunk       ( USHORT ChunkTypeID );
    static USHORT   GetHeaderLength     ( CHUNKHEADER * pHeader );

    //-------------------------------------------------------------------------
    // Public Stream Operation Wrappers
    //-------------------------------------------------------------------------
    // Note : The stream IO functions shown here are not final, and they do
    //        not support large file sizes as a rule. In future versions, the
    //        library will expose additional functionality to allow you to
    //        specify your own read / write routines.
    //-------------------------------------------------------------------------    
    ULONG           Write               ( LPVOID pBuffer, ULONG BufferSize );
    ULONG           Read                ( LPVOID pBuffer, ULONG BufferSize );
    int             Seek                ( long Offset, int Origin = SEEK_CUR );
    LPTSTR          ReadString          ( LPTSTR  strData, ULONG BufferSize );
    int             WriteString         ( LPCTSTR strData );
    bool            SetLength           ( long Length );
    long            GetPosition         ( );
    long            GetLength           ( );
    int             Flush               ( );
    bool            IsOpen              ( ) const;
    bool            IsEOF               ( );

private:
    //-------------------------------------------------------------------------
    // Private Functions for This Class.
    //-------------------------------------------------------------------------
    CHUNKDETAILS   *GetOpenChunkHead    ( ) const;
    CHUNKDETAILS   *GetOpenChunkTail    ( ) const;
    void            RemoveOpenChunk     ( CHUNKDETAILS * pDetails );
    void            AddOpenChunk        ( CHUNKDETAILS * pDetails );
    bool            NextChunk           ( CHUNKHEADER * pHeader = NULL, bool StepInto = false );

protected:
    //-------------------------------------------------------------------------
    // Protected Functions for This Class.
    //-------------------------------------------------------------------------
    bool            CheckValidAuthor    ( LPVOID pAuthor, UCHAR Length ) const;
    void            StepInto            ( LPVOID pCustomData = NULL );
    void            ReadName            ( UCHAR LengthSize, char ** Name );
    void            ReadScriptRef       ( SCRIPT_REF * pScript );
    void            SeekName            ( UCHAR LengthSize );
    void            SeekScriptRef       ( );
    void            WriteName           ( UCHAR LengthSize, char * Name );

    //-------------------------------------------------------------------------
    // Protected Variables for This Class.
    //-------------------------------------------------------------------------
    IWFPROCESSMODE  m_CurrentMode;      // Are we reading or writing this IWF.
    FILE           *m_pFile;            // Main stream object we are accessing.
    CHUNKDETAILS   *m_pOpenChunks;      // List of all the chunks currently open for processing.
    unsigned char   m_pAuthor[255];     // Duplicate of the currently set author data.
    unsigned char   m_AuthorLength;     // Length of the stored author data.
    __int64         m_StreamStartPos;   // The initial starting position of the stream
    __int64         m_LastEOCPos;       // Last End Of Chunk written for the root level of the file
    CHUNKCALLBACK  *m_pChunkProc;       // Registered chunk callback procedures (Linked List)

};

#endif // _LIBIWF_H_