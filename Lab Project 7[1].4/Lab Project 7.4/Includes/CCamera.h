//-----------------------------------------------------------------------------
// File: CCamera.h
//
// Desc: This file houses the camera related classes used by the framework to
//       control player movement etc.
//
// Copyright (c) 1997-2002 Daedalus Developments. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _CCAMERA_H_
#define _CCAMERA_H_

//-----------------------------------------------------------------------------
// CCamera Specific Includes
//-----------------------------------------------------------------------------
#include "Main.h"
#include "CPlayer.h"

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CCamera (Class)
// Desc : Camera class stores information about the camera position,
//        orientation, fov etc, and also generates transform matrices on
//        request.
//-----------------------------------------------------------------------------
class CCamera
{
public:
    //-------------------------------------------------------------------------
    // Enumerators
    //-------------------------------------------------------------------------
    enum CAMERA_MODE {
        MODE_FPS         = 1,
        MODE_THIRDPERSON = 2,
        MODE_SPACECRAFT  = 3,

        MODE_FORCE_32BIT = 0x7FFFFFFF
    };

    enum SCREEN_EFFECT {
        EFFECT_WATER        = 1,
        EFFECT_FORCE_32BIT  = 0x7FFFFFFF
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
             CCamera( const CCamera * pCamera );
             CCamera();
    virtual ~CCamera();

    //-------------------------------------------------------------------------
    // Public Functions for This Class.
    //-------------------------------------------------------------------------
    void                SetFOV           ( float FOV ) { m_fFOV = FOV; m_bProjDirty = true; }
    void                SetViewport      ( long Left, long Top, long Width, long Height, float NearClip, float FarClip, LPDIRECT3DDEVICE9 pDevice = NULL );
    void                UpdateRenderView ( LPDIRECT3DDEVICE9 pD3DDevice );
    void                UpdateRenderProj ( LPDIRECT3DDEVICE9 pD3DDevice );
    const D3DXMATRIX&   GetProjMatrix    ( );    
    float               GetFOV           ( ) const { return m_fFOV;  }
    float               GetNearClip      ( ) const { return m_fNearClip; }
    float               GetFarClip       ( ) const { return m_fFarClip; }
    const D3DVIEWPORT9& GetViewport      ( ) const { return m_Viewport; }
    CPlayer *           GetPlayer        ( ) const { return m_pPlayer;  }

    const D3DXVECTOR3&  GetPosition      ( ) const { return m_vecPos;   }
    const D3DXVECTOR3&  GetLook          ( ) const { return m_vecLook;  }
    const D3DXVECTOR3&  GetUp            ( ) const { return m_vecUp;    }
    const D3DXVECTOR3&  GetRight         ( ) const { return m_vecRight; }
    const D3DXMATRIX&   GetViewMatrix    ( );
    
    void                SetVolumeInfo    ( const VOLUME_INFO& Volume );
    const VOLUME_INFO&  GetVolumeInfo    ( ) const;

    void                RenderScreenEffect ( LPDIRECT3DDEVICE9 pD3DDevice, SCREEN_EFFECT Effect, ULONG Value );

    //-------------------------------------------------------------------------
    // Public Virtual Functions for This Class.
    //-------------------------------------------------------------------------
    virtual void        AttachToPlayer   ( CPlayer * pPlayer );
    virtual void        DetachFromPlayer ( );
    virtual void        SetPosition      ( const D3DXVECTOR3& Position ) { m_vecPos = Position; m_bViewDirty = true; m_bFrustumDirty = true; }
    virtual void        Move             ( const D3DXVECTOR3& vecShift ) { m_vecPos += vecShift; m_bViewDirty = true; m_bFrustumDirty = true; }
    virtual void        Rotate           ( float x, float y, float z )   {}
    virtual void        Update           ( float TimeScale, float Lag )  {}
    virtual void        SetCameraDetails ( const CCamera * pCamera )     {}

    virtual CAMERA_MODE GetCameraMode    ( ) const = 0;

    bool                BoundsInFrustum  ( const D3DXVECTOR3 & Min, const D3DXVECTOR3 & Max );

protected:
    //-------------------------------------------------------------------------
    // Protected Functions for This Class.
    //-------------------------------------------------------------------------
    void                CalcFrustumPlanes( );

    //-------------------------------------------------------------------------
    // Protected Variables for This Class.
    //-------------------------------------------------------------------------
    CPlayer       * m_pPlayer;              // The player object we are attached to
    VOLUME_INFO     m_Volume;               // Stores information about cameras collision volume
    D3DXMATRIX      m_mtxView;              // Cached view matrix
    D3DXMATRIX      m_mtxProj;              // Cached projection matrix
    D3DXPLANE       m_Frustum[6];           // The 6 planes of our frustum.

    bool            m_bViewDirty;           // View matrix dirty ?
    bool            m_bProjDirty;           // Proj matrix dirty ?
    bool            m_bFrustumDirty;        // Are the frustum planes dirty ?

    // Perspective Projection parameters
    float           m_fFOV;                 // FOV Angle.
    float           m_fNearClip;            // Near Clip Plane Distance
    float           m_fFarClip;             // Far Clip Plane Distance
    D3DVIEWPORT9    m_Viewport;             // The viewport details into which we are rendering.

    // Cameras current position & orientation
    D3DXVECTOR3     m_vecPos;               // Camera Position
    D3DXVECTOR3     m_vecUp;                // Camera Up Vector
    D3DXVECTOR3     m_vecLook;              // Camera Look Vector
    D3DXVECTOR3     m_vecRight;             // Camera Right Vector

};

//-----------------------------------------------------------------------------
// Name : CCam1stPerson (Class)
// Desc : Camera object which exhibits first person shooter style behaviour. 
//-----------------------------------------------------------------------------
class CCam1stPerson : public CCamera
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    CCam1stPerson( const CCamera * pCamera );
    CCam1stPerson();

    //-------------------------------------------------------------------------
    // Public Functions for This Class.
    //-------------------------------------------------------------------------
    CAMERA_MODE         GetCameraMode    ( ) const { return MODE_FPS; }
    void                Rotate           ( float x, float y, float z );
    void                SetCameraDetails ( const CCamera * pCamera );

};

//-----------------------------------------------------------------------------
// Name : CCam3rdPerson (Class)
// Desc : Camera object which exhibits third person, over shoulder behaviour. 
//-----------------------------------------------------------------------------
class CCam3rdPerson : public CCamera
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    CCam3rdPerson( const CCamera * pCamera );
    CCam3rdPerson();

    //-------------------------------------------------------------------------
    // Public Functions for This Class.
    //-------------------------------------------------------------------------
    CAMERA_MODE         GetCameraMode    ( ) const { return MODE_THIRDPERSON; }
    void                Move             ( const D3DXVECTOR3& vecShift ) {};
    void                Rotate           ( float x, float y, float z )   {};
    void                Update           ( float TimeScale, float Lag );
    void                SetCameraDetails ( const CCamera * pCamera );
    void                SetLookAt        ( const D3DXVECTOR3& vecLookAt );

};

//-----------------------------------------------------------------------------
// Name : CCamSpaceCraft (Class)
// Desc : Camera object which exhibts space craft style behaviour.
//-----------------------------------------------------------------------------
class CCamSpaceCraft : public CCamera
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
    CCamSpaceCraft( const CCamera * pCamera );
    CCamSpaceCraft();

    //-------------------------------------------------------------------------
    // Public Functions for This Class.
    //-------------------------------------------------------------------------
    CAMERA_MODE         GetCameraMode    ( ) const { return MODE_SPACECRAFT; }
    void                Rotate           ( float x, float y, float z );
    void                SetCameraDetails ( const CCamera * pCamera );

};

#endif // _CCAMERA_H_