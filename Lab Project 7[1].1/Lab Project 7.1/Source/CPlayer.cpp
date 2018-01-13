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
    if ( Direction & DIR_FORWARD  ) vecShift += m_vecLook  * Distance;
    if ( Direction & DIR_BACKWARD ) vecShift -= m_vecLook  * Distance;
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
    ULONG i;

    // Add on our gravity vector
    m_vecVelocity += m_vecGravity * TimeScale;

    /*
    
    // Note : The following clamping method is not what we require in all situations
    
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
    Length = D3DXVec3Length( &m_vecVelocity );

    */

    // Clamp the XZ velocity to our max velocity vector
    float Length = sqrtf(m_vecVelocity.x * m_vecVelocity.x + m_vecVelocity.z * m_vecVelocity.z);
    if ( Length > m_fMaxVelocityXZ )
    {
        m_vecVelocity.x *= ( m_fMaxVelocityXZ / Length );
        m_vecVelocity.z *= ( m_fMaxVelocityXZ / Length );
    
    } // End if clamp XZ velocity

    // Clamp the Y velocity to our max velocity vector
    Length = sqrtf(m_vecVelocity.y * m_vecVelocity.y);
    if ( Length > m_fMaxVelocityY )
    {
        m_vecVelocity.y *= ( m_fMaxVelocityY / Length );
    
    } // End if clamp Y velocity
        
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
        case CCamera::MODE_FPS:
            
            if ( !(pNewCamera = new CCam1stPerson( m_pCamera ))) return false;
            break;

        case CCamera::MODE_THIRDPERSON:

            if ( !(pNewCamera = new CCam3rdPerson( m_pCamera ))) return false;
            break;

        case CCamera::MODE_SPACECRAFT:
            
            if ( !(pNewCamera = new CCamSpaceCraft( m_pCamera ))) return false;
            break;
    
    } // End Switch

    // Validate
    if (!pNewCamera) return false;

    // If our old mode was SPACECRAFT we need to sort out some things
    if ( m_CameraMode == CCamera::MODE_SPACECRAFT )
    {
        // Flatten out the vectors
        m_vecUp      = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
        m_vecRight.y = 0.0f;
        m_vecLook.y  = 0.0f;

        // Finally, normalize them
        D3DXVec3Normalize( &m_vecRight, &m_vecRight );
        D3DXVec3Normalize( &m_vecLook, &m_vecLook );

        // Reset our pitch / yaw / roll values
        m_fPitch = 0.0f;
        m_fRoll  = 0.0f;
        m_fYaw   = D3DXToDegree( acosf( D3DXVec3Dot( &D3DXVECTOR3( 0.0f, 0.0f, 1.0f ), &m_vecLook ) ) );
        if ( m_vecLook.x < 0.0f ) m_fYaw = -m_fYaw;

    } // End if Old Mode was spacecraft
    else if ( m_pCamera && Mode == CCamera::MODE_SPACECRAFT )
    {
        // We are switching TO spacecraft mode. Set player vectors to match old camera vectors
        m_vecRight = m_pCamera->GetRight();
        m_vecLook  = m_pCamera->GetLook();
        m_vecUp    = m_pCamera->GetUp();

    } // End if MODE_SPACECRAFT

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
    if ( m_CameraMode == CCamera::MODE_FPS || m_CameraMode == CCamera::MODE_THIRDPERSON )
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
    
    } // End if MODE_FPS
    else if ( m_CameraMode == CCamera::MODE_SPACECRAFT )
    {
        // Allow camera to rotate prior to updating our axis
        m_pCamera->Rotate( x, y, z );

        if ( x != 0 ) 
        {
            // Build rotation matrix
            D3DXMatrixRotationAxis( &mtxRotate, &m_vecRight, D3DXToRadian( x ) );
            D3DXVec3TransformNormal( &m_vecLook, &m_vecLook, &mtxRotate );
            D3DXVec3TransformNormal( &m_vecUp, &m_vecUp, &mtxRotate );

        } // End if Pitch

        if ( y != 0 ) 
        {
            // Build rotation matrix
            D3DXMatrixRotationAxis( &mtxRotate, &m_vecUp, D3DXToRadian( y ) );
            D3DXVec3TransformNormal( &m_vecLook, &m_vecLook, &mtxRotate );
            D3DXVec3TransformNormal( &m_vecRight, &m_vecRight, &mtxRotate );
        
        } // End if Yaw

        if ( z != 0 ) 
        {
            // Build rotation matrix
            D3DXMatrixRotationAxis( &mtxRotate, &m_vecLook, D3DXToRadian( z ) );
            D3DXVec3TransformNormal( &m_vecUp, &m_vecUp, &mtxRotate );
            D3DXVec3TransformNormal( &m_vecRight, &m_vecRight, &mtxRotate );
        
        } // End if Roll

    
    } // End if other modes

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
    CObject * pObject = NULL;

    // Validate Parameters
    if (!pDevice) return;

    // Select which object to render
    if ( m_pCamera )
    {
        if ( m_CameraMode == CCamera::MODE_THIRDPERSON ) pObject = m_p3rdPersonObject;
    
    } // End if camera available
    else
    {
        // Select the 3rd person object (viewed from outside)
        pObject = m_p3rdPersonObject;
    
    } // End if no camera available
    
    // Validate
    if (!pObject) return;
    
    // Update our object's world matrix
    D3DXMATRIX * pMatrix = &pObject->m_mtxWorld;
    pMatrix->_11 = m_vecRight.x; pMatrix->_21 = m_vecUp.x; pMatrix->_31 = m_vecLook.x;
	pMatrix->_12 = m_vecRight.y; pMatrix->_22 = m_vecUp.y; pMatrix->_32 = m_vecLook.y;
	pMatrix->_13 = m_vecRight.z; pMatrix->_23 = m_vecUp.z; pMatrix->_33 = m_vecLook.z;

    pMatrix->_41 = m_vecPos.x;
    pMatrix->_42 = m_vecPos.y - 10.0f;
    pMatrix->_43 = m_vecPos.z;

    // Render our player mesh object
    CMesh * pMesh = pObject->m_pMesh;
    pDevice->SetTransform( D3DTS_WORLD, &pObject->m_mtxWorld );
    pDevice->SetFVF( pMesh->m_nFVFCode );
    pDevice->SetStreamSource( 0, pMesh->m_pVertexBuffer, 0, pMesh->m_nStride );
    pDevice->SetIndices( pMesh->m_pIndexBuffer );
    pDevice->SetTexture( 0, NULL );
    pDevice->SetTexture( 1, NULL );
    pDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, 8, 0, 14 );
    
}