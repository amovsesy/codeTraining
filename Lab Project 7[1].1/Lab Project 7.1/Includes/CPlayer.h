//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//
// Desc: This file stores the player object class. This class performs tasks
//       such as player movement, some minor physics as well as rendering.
//
// Copyright (c) 1997-2002 Adam Hoult & Gary Simmons. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _CPLAYER_H_
#define _CPLAYER_H_

//-----------------------------------------------------------------------------
// CPlayer Specific Includes
//-----------------------------------------------------------------------------
#include "Main.h"

//-----------------------------------------------------------------------------
// Typedefs, structures and Enumerators
//-----------------------------------------------------------------------------
typedef struct _VOLUME_INFO     // Stores information about our object volume
{
    D3DXVECTOR3 Min;            // Minimum object space extents of the volume
    D3DXVECTOR3 Max;            // Maximum object space extents of the volume

} VOLUME_INFO;

typedef struct _CALLBACK_FUNC   // Stores details for a callback
{
    LPVOID  pFunction;          // Function Pointer
    LPVOID  pContext;           // Context to pass to the function

} CALLBACK_FUNC;

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class CObject;
class CCamera;
class CPlayer;

//-----------------------------------------------------------------------------
// Typedefs for update / collision callbacks.
//-----------------------------------------------------------------------------
typedef void (*UPDATEPLAYER)(LPVOID pContext, CPlayer * pPlayer, float TimeScale);
typedef void (*UPDATECAMERA)(LPVOID pContext, CCamera * pCamera, float TimeScale);

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CPlayer (Class)
// Desc : Player class handles all player manipulation, update and management.
//-----------------------------------------------------------------------------
class CPlayer
{
public:
    //-------------------------------------------------------------------------
    // Enumerators
    //-------------------------------------------------------------------------
    enum DIRECTION { 
        DIR_FORWARD     = 1, 
        DIR_BACKWARD    = 2, 
        DIR_LEFT        = 4, 
        DIR_RIGHT       = 8, 
        DIR_UP          = 16, 
        DIR_DOWN        = 32,

        DIR_FORCE_32BIT = 0x7FFFFFFF
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
             CPlayer();
    virtual ~CPlayer();

    //-------------------------------------------------------------------------
    // Public Functions for This Class.
    //-------------------------------------------------------------------------
    bool                SetCameraMode      ( ULONG Mode );
    void                Update             ( float TimeScale );
    
    void                AddPlayerCallback    ( UPDATEPLAYER pFunc, LPVOID pContext );
    void                AddCameraCallback    ( UPDATECAMERA pFunc, LPVOID pContext );
    void                RemovePlayerCallback ( UPDATEPLAYER pFunc, LPVOID pContext );
    void                RemoveCameraCallback ( UPDATECAMERA pFunc, LPVOID pContext );

    void                Set3rdPersonObject ( CObject * pObject           ) { m_p3rdPersonObject = pObject; }
    void                SetFriction        ( float Friction              ) { m_fFriction = Friction; }
    void                SetGravity         ( const D3DXVECTOR3& Gravity  ) { m_vecGravity = Gravity; }
    void                SetMaxVelocityXZ   ( float MaxVelocity           ) { m_fMaxVelocityXZ = MaxVelocity; }
    void                SetMaxVelocityY    ( float MaxVelocity           ) { m_fMaxVelocityY = MaxVelocity; }
    void                SetVelocity        ( const D3DXVECTOR3& Velocity ) { m_vecVelocity = Velocity; }
    void                SetCamLag          ( float CamLag )                { m_fCameraLag = CamLag; }
    void                SetCamOffset       ( const D3DXVECTOR3& Offset );
    void                SetVolumeInfo      ( const VOLUME_INFO& Volume );
    const VOLUME_INFO&  GetVolumeInfo      ( ) const;

    CCamera           * GetCamera          ( ) const { return m_pCamera; }
    const D3DXVECTOR3 & GetVelocity        ( ) const { return m_vecVelocity; }
    const D3DXVECTOR3 & GetCamOffset       ( ) const { return m_vecCamOffset; }

    const D3DXVECTOR3 & GetPosition        ( ) const { return m_vecPos; }
    const D3DXVECTOR3 & GetLook            ( ) const { return m_vecLook; }
    const D3DXVECTOR3 & GetUp              ( ) const { return m_vecUp; }
    const D3DXVECTOR3 & GetRight           ( ) const { return m_vecRight; }
    
    float               GetYaw             ( ) const { return m_fYaw; }
    float               GetPitch           ( ) const { return m_fPitch; }
    float               GetRoll            ( ) const { return m_fRoll; }

    void                SetPosition        ( const D3DXVECTOR3& Position ) { Move( Position - m_vecPos, false ); }

    void                Move               ( ULONG Direction, float Distance, bool Velocity = false );
    void                Move               ( const D3DXVECTOR3& vecShift, bool Velocity = false );
    void                Rotate             ( float x, float y, float z );

    void                Render             ( LPDIRECT3DDEVICE9 pDevice );

private:
    //-------------------------------------------------------------------------
    // Private Variables for This Class.
    //-------------------------------------------------------------------------
    CCamera       * m_pCamera;              // Our current camera object
    CObject       * m_p3rdPersonObject;     // Object to be displayed in third person mode
    VOLUME_INFO     m_Volume;               // Stores information about players collision volume
    ULONG           m_CameraMode;           // Stored camera mode.

    // Players position and orientation values
    D3DXVECTOR3     m_vecPos;               // Player Position
    D3DXVECTOR3     m_vecUp;                // Player Up Vector
    D3DXVECTOR3     m_vecRight;             // Player Right Vector
    D3DXVECTOR3     m_vecLook;              // Player Look Vector
    D3DXVECTOR3     m_vecCamOffset;         // Camera offset
    float           m_fPitch;               // Player pitch
    float           m_fRoll;                // Player roll
    float           m_fYaw;                 // Player yaw
    
    // Force / Player Update Variables
    D3DXVECTOR3     m_vecVelocity;          // Movement velocity vector
    D3DXVECTOR3     m_vecGravity;           // Gravity vector
    float           m_fMaxVelocityXZ;       // Maximum camera velocity on XZ plane
    float           m_fMaxVelocityY;        // Maximum camera velocity on Y Axis
    float           m_fFriction;            // The amount of friction causing the camera to slow
    float           m_fCameraLag;           // Amount of camera lag in seconds (0 to disable)

    // Stored collision callbacks
    CALLBACK_FUNC   m_pUpdatePlayer[255];   // Array of 'UpdatePlayer' callbacks
    CALLBACK_FUNC   m_pUpdateCamera[255];   // Array of 'UpdateCamera' callbacks
    USHORT          m_nUpdatePlayerCount;   // Number of 'UpdatePlayer' callbacks stored
    USHORT          m_nUpdateCameraCount;   // Number of 'UpdateCamera' callbacks stored

};

#endif // _CPLAYER_H_