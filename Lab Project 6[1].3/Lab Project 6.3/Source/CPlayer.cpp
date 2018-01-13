//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//
// Desc: This file stores the player object class. This class performs tasks
//       such as player movement, some minor physics as well as rendering.
//
// Copyright (c) 1997-2002 Adam Hoult & Gary Simmons. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CPlayer Specific Includes
//-----------------------------------------------------------------------------
#include "..\\Includes\\CPlayer.h"
#include "..\\Includes\\CCamera.h"
#include "..\\Includes\\CObject.h"

//-----------------------------------------------------------------------------
// Name : CPlayer () (Constructor)
// Desc : CPlayer Class Constructor
//-----------------------------------------------------------------------------
CPlayer::CPlayer()
{
    // Clear any required variables
    m_pCamera            = NULL;
    m_p3rdPersonObject   = NULL;
    m_CameraMode         = 0;
    m_nUpdatePlayerCount = 0;
    m_nUpdateCameraCount = 0;

    // Players position & orientation (independant of camera)
    m_vecPos             = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    m_vecRight           = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
    m_vecUp              = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
    m_vecLook            = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );

    // Camera offset values (from the players origin)
    m_vecCamOffset       = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    m_fCameraLag         = 0.0f;

    // The following force related values are used in conjunction with 'Update' only
    m_vecVelocity        = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    m_vecGravity         = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    m_fMaxVelocityXZ     = 125.0f;
    m_fMaxVelocityY      = 125.0f;
    m_fFriction          = 250.0f;

    // Default volume information
    m_Volume.Min         = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    m_Volume.Max         = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    
}

//-----------------------------------------------------------------------------
// Name : ~CPlayer () (Destructor)
// Desc : CPlayer Class Destructor
//-----------------------------------------------------------------------------
CPlayer::~CPlayer()
{
    // Release any allocated memory
    if ( m_pCamera ) delete m_pCamera;
    
    // Clear required values
    m_pCamera           = NULL;
    m_p3rdPersonObject  = NULL;

}

//-----------------------------------------------------------------------------
// Name : Move ()
// Desc : Move the camera in the specified direction for the specified distance
// Note : Specify 'true' to velocity if you wish the move to affect the cameras
//        velocity rather than it's absolute position
//-----------------------------------------------------------------------------
void CPlayer::Move( ULONG Direction, float Distance, bool Velocity )
{
    D3DXVECTOR3 vecShift = D3DXVECTOR3( 0, 0, 0 );

    // Which direction are we moving ?
    if ( Direction & DIR_FORWARD  ) vecShift += m_pCamera->GetLook() * Distance;
    if ( Direction & DIR_BACKWARD ) vecShift -= m_pCamera->GetLook() * Distance;
    if ( Direction & DIR_RIGHT    ) vecShift += m_vecRight * Distance;
    if ( Direction & DIR_LEFT     ) vecShift -= m_vecRight * Distance;
    if ( Direction & DIR_UP       ) vecShift += m_vecUp    * Distance;
    if ( Direction & DIR_DOWN     ) vecShift -= m_vecUp    * Distance;

    // Update camera vectors
    if ( Direction ) Move( vecShift, Velocity );
}

//-----------------------------------------------------------------------------
// Name : Move ()
// Desc : Move the camera by the specified amount based on the vector passed.
//-----------------------------------------------------------------------------
void CPlayer::Move( const D3DXVECTOR3& vecShift, bool Velocity )
{
    // Update velocity or actual position ?
    if ( Velocity )
    {
        m_vecVelocity += vecShift;
    
    } // End if Velocity
    else
    {
        m_vecPos += vecShift;
        m_pCamera->Move( vecShift );
    
    } // End if !Velocity
    
}

//-----------------------------------------------------------------------------
// Name : SetCamOffset ()
// Desc : Set the offset, relative to the player, at which the camera will be 
//        positioned.
//-----------------------------------------------------------------------------
void CPlayer::SetCamOffset( const D3DXVECTOR3& Offset )
{
    m_vecCamOffset = Offset;

    if (!m_pCamera) return;
    m_pCamera->SetPosition( m_vecPos + Offset );
}

//-----------------------------------------------------------------------------
// Name : Update ()
// Desc : Update the players position based on the current velocity / gravity
//        settings. These will be scaled by the TimeScale factor passed in.
//-----------------------------------------------------------------------------
void CPlayer::Update( float TimeScale )
{
    D3DXVECTOR3 vecDirection;
    float fScale;
    ULONG i;
    

    // Add on our gravity vector
    m_vecVelocity += m_vecGravity * TimeScale;

    // Retrieve the movement direction for max velocity clamping
    D3DXVec3Normalize( &vecDirection, &m_vecVelocity );
    fScale = fabsf(vecDirection.y);

    // Calculate the speed at which we should clamp based on 
    // the direction of movement, used to scale. This provides
    // in essence, and ellipsoid shaped clamping region.
    fScale = asinf( fScale ) / 1.57079f;
    float MaxVelocity = m_fMaxVelocityXZ + ( (m_fMaxVelocityY - m_fMaxVelocityXZ) * fScale );

    // Clamp the velocity to our max velocity vector
    float Length = D3DXVec3Length( &m_vecVelocity );
    if ( Length > MaxVelocity ) m_vecVelocity *= ( MaxVelocity / Length );
        
    // Move our player (will also move the camera if required)
    Move( m_vecVelocity * TimeScale, false );

    // Allow all our registered callbacks to update the player position
    for ( i =0; i < m_nUpdatePlayerCount; i++ )
    {
        UPDATEPLAYER UpdatePlayer = (UPDATEPLAYER)m_pUpdatePlayer[i].pFunction;
        UpdatePlayer( m_pUpdatePlayer[i].pContext, this, TimeScale );

    } // Next Player Callback

    // Let our camera update if required
    m_pCamera->Update( TimeScale, m_fCameraLag );

    // Allow all our registered callbacks to update the camera position
    for ( i =0; i < m_nUpdateCameraCount; i++ )
    {
        UPDATECAMERA UpdateCamera = (UPDATECAMERA)m_pUpdateCamera[i].pFunction;
        UpdateCamera( m_pUpdateCamera[i].pContext, m_pCamera, TimeScale );

    } // Next Camera Callback

    // Calculate the reverse of the velocity direction
    D3DXVECTOR3 vecDec = -m_vecVelocity;
    D3DXVec3Normalize( &vecDec, &vecDec );

    // Retrieve the actual velocity length
    Length = D3DXVec3Length( &m_vecVelocity );

    // Calculate total deceleration based on friction values
    float Dec = (m_fFriction * TimeScale);
    if ( Dec > Length ) Dec = Length;

    // Apply the friction force
    m_vecVelocity += vecDec * Dec;

}

//-----------------------------------------------------------------------------
// Name : SetCameraMode ()
// Desc : Sets the camera type we are using to view the player.
// Note : Only returns false on a fatal error of some type.
//-----------------------------------------------------------------------------
bool CPlayer::SetCameraMode( ULONG Mode )
{
    CCamera * pNewCamera = NULL;

    // Check for a no-op
    if ( m_pCamera && m_CameraMode == Mode ) return true;

    // Which mode are we switching into
    switch ( Mode )
    {
        case CCamera::MODE_FPS_FLY:
            
            if ( !(pNewCamera = new CCamFPSFly( m_pCamera ))) return false;
            break;
    
    } // End Switch

    // Validate
    if (!pNewCamera) return false;

    // Store new mode
    m_CameraMode = Mode;

    // Attach the new camera to 'this' player object
    pNewCamera->AttachToPlayer( this );

    // Destroy our old camera and replace with our new one
    if ( m_pCamera ) delete m_pCamera;
    m_pCamera = pNewCamera;

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : Rotate ()
// Desc : Rotate the player around the players local axis.
//-----------------------------------------------------------------------------
void CPlayer::Rotate( float x, float y, float z )
{
    D3DXMATRIX mtxRotate;

    // Validate requirements
    if (!m_pCamera) return;

    // Are we in FPS mode ?
    if ( m_CameraMode == CCamera::MODE_FPS_FLY )
    {

        // Update & Clamp pitch / roll / Yaw values
        if ( x )
        {
            // Make sure we don't overstep our pitch boundaries
            m_fPitch += x;
            if ( m_fPitch >  89.0f ) { x -= (m_fPitch - 89.0f); m_fPitch = 89.0f; }
            if ( m_fPitch < -89.0f ) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }

        } // End if any Pitch

        if ( y )
        {
            // Ensure yaw (in degrees) wraps around between 0 and 360
            m_fYaw += y;
            if ( m_fYaw >  360.0f ) m_fYaw -= 360.0f;
            if ( m_fYaw <  0.0f   ) m_fYaw += 360.0f;

        } // End if any yaw

        // Roll is purely a statistical value, no player rotation actually occurs
        if ( z ) 
        {
            // Make sure we don't overstep our roll boundaries
            m_fRoll += z;
            if ( m_fRoll >  20.0f ) { z -= (m_fRoll - 20.0f); m_fRoll = 20.0f; }
            if ( m_fRoll < -20.0f ) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }

        } // End if any roll

        // Allow camera to rotate prior to updating our axis
        m_pCamera->Rotate( x, y, z );

        // Now rotate our axis
        if ( y )
        {
            // Build rotation matrix
            D3DXMatrixRotationAxis( &mtxRotate, &m_vecUp, D3DXToRadian( y ) );
            
            // Update our vectors
            D3DXVec3TransformNormal( &m_vecLook, &m_vecLook, &mtxRotate );
            D3DXVec3TransformNormal( &m_vecRight, &m_vecRight, &mtxRotate );

        } // End if any yaw
    
    } // End if MODE_FPS_FLY

    // Because many rotations will cause floating point errors, the axis will eventually become
    // non-perpendicular to one other causing all hell to break loose. Therefore, we must
    // perform base vector regeneration to ensure that all vectors remain unit length and
    // perpendicular to one another. This need not be done on EVERY call to rotate (i.e. you
    // could do this once every 50 calls for instance).
    D3DXVec3Normalize( &m_vecLook, &m_vecLook );
    D3DXVec3Cross( &m_vecRight, &m_vecUp, &m_vecLook );
    D3DXVec3Normalize( &m_vecRight, &m_vecRight );
    D3DXVec3Cross( &m_vecUp, &m_vecLook, &m_vecRight );
    D3DXVec3Normalize( &m_vecUp, &m_vecUp );

}

//-----------------------------------------------------------------------------
// Name : SetVolumeInfo ()
// Desc : Set the players collision volume information
//-----------------------------------------------------------------------------
void CPlayer::SetVolumeInfo( const VOLUME_INFO& Volume )
{
    m_Volume = Volume;
}

//-----------------------------------------------------------------------------
// Name : GetVolumeInfo ()
// Desc : Retrieve the players collision volume information
//-----------------------------------------------------------------------------
const VOLUME_INFO& CPlayer::GetVolumeInfo( ) const
{
    return m_Volume;
}

//-----------------------------------------------------------------------------
// Name : AddPlayerCallback ()
// Desc : Add an 'UpdatePlayer' callback ready for calling during the players
//        update procedure.
//-----------------------------------------------------------------------------
void CPlayer::AddPlayerCallback( UPDATEPLAYER pFunc, LPVOID pContext )
{
    // Store callback details
    m_pUpdatePlayer[m_nUpdatePlayerCount].pFunction = (LPVOID)pFunc;
    m_pUpdatePlayer[m_nUpdatePlayerCount].pContext  = pContext;
    m_nUpdatePlayerCount++;
}

//-----------------------------------------------------------------------------
// Name : AddCameraCallback ()
// Desc : Add an 'UpdateCamera' callback ready for calling during the players
//        update procedure.
//-----------------------------------------------------------------------------
void CPlayer::AddCameraCallback( UPDATECAMERA pFunc, LPVOID pContext )
{
    // Store callback details
    m_pUpdateCamera[m_nUpdateCameraCount].pFunction = (LPVOID)pFunc;
    m_pUpdateCamera[m_nUpdateCameraCount].pContext  = pContext;
    m_nUpdateCameraCount++;
}

//-----------------------------------------------------------------------------
// Name : RemovePlayerCallback ()
// Desc : Remove an 'UpdatePlayer' callback from the list
//-----------------------------------------------------------------------------
void CPlayer::RemovePlayerCallback( UPDATEPLAYER pFunc, LPVOID pContext )
{
    int i;

    // Search for the specified callback data
    for ( i = 0; i < m_nUpdatePlayerCount; i++ )
    {
        // Break out if this matches
        if ( m_pUpdatePlayer[i].pFunction == pFunc && m_pUpdatePlayer[i].pContext == pContext ) break;

    } // Next Player Callback

    // Bail if we didn't find one
    if ( i == m_nUpdatePlayerCount ) return;

    // We found one so remove this from the list
    if ( i < (m_nUpdatePlayerCount - 1))
    {
        memmove( &m_pUpdatePlayer[i], &m_pUpdatePlayer[i + 1], (m_nUpdatePlayerCount - 2) * sizeof(CALLBACK_FUNC) );
    
    } // End if

    // Reduce player callback count
    m_nUpdatePlayerCount--;
}

//-----------------------------------------------------------------------------
// Name : RemoveCameraCallback ()
// Desc : Remove an 'UpdateCamera' callback from the list
//-----------------------------------------------------------------------------
void CPlayer::RemoveCameraCallback( UPDATECAMERA pFunc, LPVOID pContext )
{
    int i;

    // Search for the specified callback data
    for ( i = 0; i < m_nUpdateCameraCount; i++ )
    {
        // Break out if this matches
        if ( m_pUpdateCamera[i].pFunction == pFunc && m_pUpdateCamera[i].pContext == pContext ) break;

    } // Next Camera Callback

    // Bail if we didn't find one
    if ( i == m_nUpdateCameraCount ) return;

    // We found one so remove this from the list
    if ( i < (m_nUpdateCameraCount - 1))
    {
        memmove( &m_pUpdateCamera[i], &m_pUpdateCamera[i + 1], (m_nUpdateCameraCount - 2) * sizeof(CALLBACK_FUNC) );
    
    } // End if

    // Reduce Camera callback count
    m_nUpdateCameraCount--;
}

//-----------------------------------------------------------------------------
// Name : Render ()
// Desc : Render the representation of this player.
//-----------------------------------------------------------------------------
void CPlayer::Render( LPDIRECT3DDEVICE9 pDevice )
{

    // There is no player rendering required in this demo.
    
}