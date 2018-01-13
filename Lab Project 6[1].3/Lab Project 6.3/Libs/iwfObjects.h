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
//                          File: iwfObjects.h                               //
//                                                                           //
//                           Date: 26/08/2002                                //
//                                                                           //
//    Desc: This file houses the various object / mesh related classes.      //
//                                                                           //
//   Copyright (c) 1997-2002 Daedalus Developments. All rights reserved.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _IWFOBJECTS_H_
#define _IWFOBJECTS_H_

//-----------------------------------------------------------------------------
// IWFObjects Specific Includes
//-----------------------------------------------------------------------------
#include "libIWF.h"

//-----------------------------------------------------------------------------
// Typedefs, Structures and Enumerators
//-----------------------------------------------------------------------------
struct LIGHTENTITY
{
    long        LightType;					 
	
	float       DiffuseRed	 ;
	float       DiffuseGreen ; 
	float       DiffuseBlue  ; 
	float       DiffuseAlpha ; 
	
	float       SpecularRed  ;
	float       SpecularGreen;
	float       SpecularBlue ;
	float       SpecularAlpha;
	
	float       AmbientRed	 ; 
	float       AmbientGreen ;
	float       AmbientBlue  ;
	float       AmbientAlpha ;

	float		Range        ; 
	float		FallOff      ; 
	float       Attenuation0 ;
	float       Attenuation1 ;
	float		Attenuation2 ;

	float       Theta		 ;
	float		Phi          ;
												
};

//-----------------------------------------------------------------------------
// Name : VECTOR3 (struct)
// Desc : Three dimensional vector storage structure.
//-----------------------------------------------------------------------------
typedef struct _VECTOR3
{                   
    //-------------------------------------------------------------------------
    // Public Variables for This Struct
    //-------------------------------------------------------------------------
    float       x;                          // Vector X coordinate component
    float       y;                          // Vector Y coordinate component
    float       z;                          // Vector Z coordinate component
    
    //-------------------------------------------------------------------------
    // Name : _VECTOR3 () (Constructor)
    // Desc : VECTOR3 structure constructor
    //-------------------------------------------------------------------------
    _VECTOR3( )
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;

    } // End Function

    //-------------------------------------------------------------------------
    // Name : _VECTOR3 () (Alternate Constructor)
    // Desc : VECTOR3 structure constructor. Sets struct values.
    //-------------------------------------------------------------------------
    _VECTOR3( float _x, float _y, float _z )
    {
        x = _x;
        y = _y;
        z = _z;

    } // End Function

} VECTOR3;

//-----------------------------------------------------------------------------
// Name : MATRIX4 (struct)
// Desc : 4x4 Matrix storage structure
//-----------------------------------------------------------------------------
typedef struct _MATRIX4
{
    
    //-------------------------------------------------------------------------
    // Public Variables for This Struct
    //-------------------------------------------------------------------------
    union  {
        struct {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        }; // End Struct
        float m[4][4];
    }; // End Union

    //-------------------------------------------------------------------------
    // Name : _MATRIX4 () (Constructor)
    // Desc : MATRIX4 structure constructor
    //-------------------------------------------------------------------------
    _MATRIX4( ) 
    {
        _11 = 1.0f; _12 = 0.0f; _13 = 0.0f; _14 = 0.0f;
        _21 = 0.0f; _22 = 1.0f; _23 = 0.0f; _24 = 0.0f;
        _31 = 0.0f; _32 = 0.0f; _33 = 1.0f; _34 = 0.0f;
        _41 = 0.0f; _42 = 0.0f; _43 = 0.0f; _44 = 1.0f;

    } // End Function

    //-------------------------------------------------------------------------
    // Name : _MATRIX4 () (Alternate Constructor)
    // Desc : MATRIX4 structure constructor. Sets struct values.
    //-------------------------------------------------------------------------
    _MATRIX4( float _m11, float _m12, float _m13, float _m14,
                float _m21, float _m22, float _m23, float _m24,
                float _m31, float _m32, float _m33, float _m34,
                float _m41, float _m42, float _m43, float _m44) 
    {
        _11 = _m11; _12 = _m12; _13 = _m13; _14 = _m14;
        _21 = _m21; _22 = _m22; _23 = _m23; _24 = _m24;
        _31 = _m31; _32 = _m32; _33 = _m33; _34 = _m34;
        _41 = _m41; _42 = _m42; _43 = _m43; _44 = _m44;

    } // End Function

    //-------------------------------------------------------------------------
    // Name : operator * ()
    // Desc : Multiplication operator
    //-------------------------------------------------------------------------
    operator *( const _MATRIX4& mtx )
    {
        _MATRIX4( _11 * mtx._11 + _12 * mtx._21 + _13 * mtx._31 + _14 * mtx._41,
	                _11 * mtx._12 + _12 * mtx._22 + _13 * mtx._32 + _14 * mtx._42,
	                _11 * mtx._13 + _12 * mtx._23 + _13 * mtx._33 + _14 * mtx._43,
                    _11 * mtx._14 + _12 * mtx._24 + _13 * mtx._34 + _14 * mtx._44,

  	                _21 * mtx._11 + _22 * mtx._21 + _23 * mtx._31 + _24 * mtx._41,
	                _21 * mtx._12 + _22 * mtx._22 + _23 * mtx._32 + _24 * mtx._42,
	                _21 * mtx._13 + _22 * mtx._23 + _23 * mtx._33 + _24 * mtx._43,
                    _21 * mtx._14 + _22 * mtx._24 + _23 * mtx._34 + _24 * mtx._44,

                    _31 * mtx._11 + _32 * mtx._21 + _33 * mtx._31 + _34 * mtx._41,
	                _31 * mtx._12 + _32 * mtx._22 + _33 * mtx._32 + _34 * mtx._42,
	                _31 * mtx._13 + _32 * mtx._23 + _33 * mtx._33 + _34 * mtx._43,
                    _31 * mtx._14 + _32 * mtx._24 + _33 * mtx._34 + _34 * mtx._44,

                    _41 * mtx._11 + _42 * mtx._21 + _43 * mtx._31 + _44 * mtx._41,
	                _41 * mtx._12 + _42 * mtx._22 + _43 * mtx._32 + _44 * mtx._42,
	                _41 * mtx._13 + _42 * mtx._23 + _43 * mtx._33 + _44 * mtx._43,
                    _41 * mtx._14 + _42 * mtx._24 + _43 * mtx._34 + _44 * mtx._44 );

    } // End Operator

    //-------------------------------------------------------------------------
    // Name : operator *= ()
    // Desc : Multiplication assign operator
    //-------------------------------------------------------------------------
    _MATRIX4 & operator *= ( const _MATRIX4& mtx )
    {
        float a1, a2, a3, a4;
        
         a1 = _11; a2 = _12; a3 = _13; a4 = _14;
        _11 = a1 * mtx._11 + a2 * mtx._21 + a3 * mtx._31 + a4 * mtx._41;
	    _12 = a1 * mtx._12 + a2 * mtx._22 + a3 * mtx._32 + a4 * mtx._42;
	    _13 = a1 * mtx._13 + a2 * mtx._23 + a3 * mtx._33 + a4 * mtx._43;
        _14 = a1 * mtx._14 + a2 * mtx._24 + a3 * mtx._34 + a4 * mtx._44;

         a1 = _21; a2 = _22; a3 = _23; a4 = _24;
  	    _21 = a1 * mtx._11 + a2 * mtx._21 + a3 * mtx._31 + a4 * mtx._41;
	    _22 = a1 * mtx._12 + a2 * mtx._22 + a3 * mtx._32 + a4 * mtx._42;
	    _23 = a1 * mtx._13 + a2 * mtx._23 + a3 * mtx._33 + a4 * mtx._43;
        _24 = a1 * mtx._14 + a2 * mtx._24 + a3 * mtx._34 + a4 * mtx._44;

         a1 = _31; a2 = _32; a3 = _33; a4 = _34;
        _31 = a1 * mtx._11 + a2 * mtx._21 + a3 * mtx._31 + a4 * mtx._41;
	    _32 = a1 * mtx._12 + a2 * mtx._22 + a3 * mtx._32 + a4 * mtx._42;
	    _33 = a1 * mtx._13 + a2 * mtx._23 + a3 * mtx._33 + a4 * mtx._43;
        _34 = a1 * mtx._14 + a2 * mtx._24 + a3 * mtx._34 + a4 * mtx._44;

         a1 = _41; a2 = _42; a3 = _43; a4 = _44;
        _41 = a1 * mtx._11 + a2 * mtx._21 + a3 * mtx._31 + a4 * mtx._41;
	    _42 = a1 * mtx._12 + a2 * mtx._22 + a3 * mtx._32 + a4 * mtx._42;
	    _43 = a1 * mtx._13 + a2 * mtx._23 + a3 * mtx._33 + a4 * mtx._43;
        _44 = a1 * mtx._14 + a2 * mtx._24 + a3 * mtx._34 + a4 * mtx._44;

        return *this;

    } // End Operator

} MATRIX4;

#define OBJECT_TYPE_UNKNOWN      0
#define OBJECT_TYPE_MESH         1
#define OBJECT_TYPE_DECALMESH    2
#define OBJECT_TYPE_ENTITY       3
#define OBJECT_TYPE_GROUP        4

//-----------------------------------------------------------------------------
// Name : AA_BBOX (struct)
// Desc : Simple axis aligned bounding box.
//-----------------------------------------------------------------------------
typedef struct _AA_BBOX
{
    //-------------------------------------------------------------------------
    // Public Variables for This Struct
    //-------------------------------------------------------------------------
    VECTOR3     Min;        // The minimum extents of the bounding volume
    VECTOR3     Max;        // The maximum extents of the bounding volume
    
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    _AA_BBOX( const VECTOR3& _Min, const VECTOR3& _Max ) { Min = _Min; Max = _Max; }
    _AA_BBOX() { Min = VECTOR3(  9999999.0f,  9999999.0f,  9999999.0f ); 
                 Max = VECTOR3( -9999999.0f, -9999999.0f, -9999999.0f ); }

} AA_BBOX;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : IIWFObject (Abstract Class / Interface)
// Desc : All groupable objects are derived from this.
//-----------------------------------------------------------------------------
class IIWFObject
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    virtual ~IIWFObject( ) { };
    
    //-------------------------------------------------------------------------
    // Public Functions for This Class
    //-------------------------------------------------------------------------
    UCHAR   QueryType( ) { return OBJECT_TYPE_UNKNOWN; }
};

//-----------------------------------------------------------------------------
// Name : iwfMaterial (Class)
// Desc : Simple material class, used to store material data.
//-----------------------------------------------------------------------------
class iwfMaterial
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
     iwfMaterial( );
    ~iwfMaterial( );
    
    //-------------------------------------------------------------------------
    // Public Variables for This Class
    //-------------------------------------------------------------------------
    char           *Name;           // The character array containing the reference name.
    COLOUR_VALUE    Diffuse;        // Diffuse reflection component
    COLOUR_VALUE    Ambient;        // Ambient reflection component
    COLOUR_VALUE    Emissive;       // Emissive reflection component
    COLOUR_VALUE    Specular;       // Specular reflection component
    float           Power;          // Specular reflection power ratio
};

//-----------------------------------------------------------------------------
// Name : iwfEntity (Class)
// Desc : Base entity class, container for the data area loaded from file.
//-----------------------------------------------------------------------------
class iwfEntity : public IIWFObject
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
             iwfEntity( ULONG Size );
             iwfEntity( );
    virtual ~iwfEntity( );
    
    //-------------------------------------------------------------------------
    // Public Functions for This Class
    //-------------------------------------------------------------------------
    UCHAR           QueryType( ) { return OBJECT_TYPE_ENTITY; }
    bool            AllocateData( ULONG Size );

    //-------------------------------------------------------------------------
    // Public Variables for This Class
    //-------------------------------------------------------------------------
    USHORT          EntityTypeID;   // The Type Identifier for the entity.
    char           *Name;           // The character array containing the reference name.
    MATRIX4         ObjectMatrix;   // The matrix assigned to the entity.
    ULONG           DataSize;       // The size of the entities data area
    UCHAR          *DataArea;       // The entities data area.
};

//-----------------------------------------------------------------------------
// Name : iwfVertex (Class)
// Desc : Vertex class used to construct & store vertex components.
//-----------------------------------------------------------------------------
class iwfVertex
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
     iwfVertex( float fX, float fY, float fZ ) { x = fX; y = fY; z = fZ; TexCoords = NULL; TexChanCount = 0; }
     iwfVertex() { x = 0.0f; y = 0.0f; z = 0.0f; TexCoords = NULL; TexChanCount = 0; }
    ~iwfVertex();

    //-------------------------------------------------------------------------
    // Public Functions for This Class
    //-------------------------------------------------------------------------
    bool        AllocateTexCoords( UCHAR ChannelCount, UCHAR ChannelSizes[] );
    float       GetTexCoord( UCHAR Channel, UCHAR Component ) const;

    //-------------------------------------------------------------------------
    // Public Variables for This Class
    //-------------------------------------------------------------------------
    float       x;              // Vertex Position X Component
    float       y;              // Vertex Position Y Component
    float       z;              // Vertex Position Z Component
    float       rhw;            // Vertex position w component
    VECTOR3     Normal;         // Vertex Normal
    float       PointSize;      // Vertex point size
    ULONG       Diffuse;        // Diffuse vertex colour
    ULONG       Specular;       // Specular vertex colour
    UCHAR       TexChanCount;   // Number of tex coord channels
    float     **TexCoords;      // Texture coordinates.

};

//-----------------------------------------------------------------------------
// Name : iwfSurface (Class)
// Desc : Basic surface class used to store an individual surfaces data.
//-----------------------------------------------------------------------------
class iwfSurface
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
             iwfSurface( USHORT VertexCount );
	         iwfSurface();
	virtual ~iwfSurface();

	//-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
    long        AddVertices( USHORT Count = 1 );
    long        AddIndices( USHORT Count = 1 );
    long        AddChannels( UCHAR Count = 1 );

    //-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
    ULONG       Components;             // Which components are valid
    VECTOR3     Normal;                 // Surface Normal
    UCHAR       ZBias;                  // Surface ZBias
    ULONG       Style;                  // Style flags
    SCRIPT_REF  RenderEffect;           // The effect to apply to this surface
    UCHAR       ChannelCount;           // Number of shader channels
    short      *TextureIndices;         // Simple array of texture indices
    short      *MaterialIndices;        // Simple array of material indices
    short      *ShaderIndices;          // Simple array of shader indices
    BLEND_MODE *BlendModes;             // Simple array of blend modes
    USHORT      CustomDataSize;         // The size of the custom data area
    UCHAR      *CustomData;             // The data stored in the mesh custom data area
    
    ULONG       VertexComponents;       // The components stored for each vertex
    UCHAR       VertexFlags;            // Various flags relating to the verts stored
    USHORT      VertexCount;            // Number of vertices stored.
    UCHAR       TexChannelCount;        // Number of vertex texture channels
    UCHAR      *TexCoordSize;           // How many dimensions each texture coordinate has.

    UCHAR       IndexVPool;             // Vertex pool from to which the indices point
    UCHAR       IndexFlags;             // Specifies the format of the indices (i.e. INDICES_TRILIST)
    USHORT      IndexCount;             // Number of indices stored.
    
    iwfVertex  *Vertices;               // Simple vertex array
    ULONG      *Indices;                // Simple index array (32 bits per index capacity)

};

//-----------------------------------------------------------------------------
// Name : iwfMesh (Class)
// Desc : Basic mesh class used to store individual mesh data.
//-----------------------------------------------------------------------------
class iwfMesh : public IIWFObject
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
             iwfMesh( ULONG Count );
	         iwfMesh();
	virtual ~iwfMesh();

	//-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
    UCHAR       QueryType( ) { return OBJECT_TYPE_MESH; }
    long        AddSurfaces( ULONG Count = 1 );

    //-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
    char       *Name;                   // The mesh name itself
    ULONG       Components;             // Which components are valid
    MATRIX4     ObjectMatrix;           // The mesh's transformation matrix
    AA_BBOX     Bounds;                 // The mesh's world space bounding box
    ULONG       Style;                  // Mesh's Style
    USHORT      CustomDataSize;         // The size of the custom data area
    UCHAR      *CustomData;             // The data stored in the mesh custom data area
    ULONG       SurfaceCount;           // Number of polygons stored
    iwfSurface**Surfaces;               // Simply surface array.


};

//-----------------------------------------------------------------------------
// Name : iwfGroup (Class)
// Desc : Stores information about a grouped object.
//-----------------------------------------------------------------------------
class iwfGroup : public IIWFObject
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
	         iwfGroup();
	virtual ~iwfGroup();

	//-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
    UCHAR       QueryType( ) { return OBJECT_TYPE_GROUP; }
    long        AddChild( ULONG Count = 1 );

    //-------------------------------------------------------------------------
	// Public Variables for This Class
	//-------------------------------------------------------------------------
    char       *Name;                   // The mesh name itself
    ULONG       ChildCount;             // Number of children stored
    IIWFObject**Children;               // Array of child objects


};

#endif // !_IWFOBJECTS_H_