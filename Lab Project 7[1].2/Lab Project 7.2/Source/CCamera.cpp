//-----------------------------------------------------------------------------
// File: CCamera.cpp
//
// Desc: This file houses the camera related classes used by the framework to
//       control player movement etc.
//
// Copyright (c) 1997-2002 Adam Hoult & Gary Simmons. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CCamera Specific Includes
//-----------------------------------------------------------------------------
#include "..\\Includes\\CCamera.h"
#include "..\\Includes\\CPlayer.h"

//-----------------------------------------------------------------------------
// CCamera Member Functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CCamera () (Constructor)
// Desc : CCamera Class Constructor
//-----------------------------------------------------------------------------
CCamera::CCamera()
{
    // Reset / Clear all required values
    m_pPlayer         = NULL;
    m_vecRight        = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
    m_vecUp           = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
    m_vecLook         = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
    m_vecPos          = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );

    m_fFOV            = 60.0f;
    m_fNearClip       = 1.0f;
    m_fFarClip        = 100.0f;
    m_Viewport.X      = 0;
    m_Viewport.Y      = 0;
    m_Viewport.Width  = 640;
    m_Viewport.Height = 480;
    m_Viewport.MinZ   = 0.0f;
    m_Viewport.MaxZ   = 1.0f;

    // Set matrices to identity
    D3DXMatrixIdentity( &m_mtxView );
    D3DXMatrixIdentity( &m_mtxProj );
}

//-----------------------------------------------------------------------------
// Name : CCamera () (Alternate Constructor)
// Desc : CCamera Class Constructor, extracts values from the passed camera.
//-----------------------------------------------------------------------------
CCamera::CCamera( const CCamera * pCamera )
{

    // Reset / Clear all required values
    m_pPlayer        = NULL;
    m_vecRight       = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
    m_vecUp          = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
    m_vecLook        = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
    m_vecPos         = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );

    m_fFOV            = 60.0f;
    m_fNearClip       = 1.0f;
    m_fFarClip        = 100.0f;
    m_Viewport.X      = 0;
    m_Viewport.Y      = 0;
    m_Viewport.Width  = 640;
    m_Viewport.Height = 480;
    m_Viewport.MinZ   = 0.0f;
    m_Viewport.MaxZ   = 1.0f;

    // Set matrices to identity
    D3DXMatrixIdentity( &m_mtxView );
    D3DXMatrixIdentity( &m_mtxProj );

}

//-----------------------------------------------------------------------------
// Name : ~CCamera () (Destructor)
// Desc : CCamera Class Destructor
//-----------------------------------------------------------------------------
CCamera::~CCamera()
{

}

//-----------------------------------------------------------------------------
// Name : AttachToPlayer ()
// Desc : Attach the camera to the specified player object
//-----------------------------------------------------------------------------
void CCamera::AttachToPlayer( CPlayer * pPlayer )
{
    // Attach ourslves to the player
    m_pPlayer = pPlayer;

}

//-----------------------------------------------------------------------------
// Name : DetachFromPlayer ()
// Desc : Detach the camera from the current player object
//-----------------------------------------------------------------------------
void CCamera::DetachFromPlayer()
{
    m_pPlayer = NULL;
}

//-----------------------------------------------------------------------------
// Name : SetViewport ()
// Desc : Sets the camera viewport values.
//-----------------------------------------------------------------------------
void CCamera::SetViewport( long Left, long Top, long Width, long Height, float NearClip, float FarClip, LPDIRECT3DDEVICE9 pDevice )
{
    // Set viewport sizes
    m_Viewport.X      = Left;
    m_Viewport.Y      = Top;
    m_Viewport.Width  = Width;
    m_Viewport.Height = Height;
    m_Viewport.MinZ   = 0.0f;
    m_Viewport.MaxZ   = 1.0f;
    m_fNearClip       = NearClip;
    m_fFarClip        = FarClip;
    m_bProjDirty      = true;

    // Update device if requested
    if ( pDevice ) pDevice->SetViewport( &m_Viewport );
}

//-----------------------------------------------------------------------------
// Name : GetProjMatrix ()
// Desc : Return the current projection matrix.
//-----------------------------------------------------------------------------
const D3DXMATRIX& CCamera::GetProjMatrix()
{
    // Only update matrix if something has changed
    if ( m_bProjDirty ) 
    {     
        float fAspect = (float)m_Viewport.Width / (float)m_Viewport.Height;

        // Set the perspective projection matrix
        D3DXMatrixPerspectiveFovLH( &m_mtxProj, D3DXToRadian( m_fFOV / 2.0f ), fAspect, m_fNearClip, m_fFarClip );
            
        // Proj Matrix has been updated
        m_bProjDirty = false; 

    } // End If Projection matrix needs updating

    // Return the projection matrix.
    return m_mtxProj;
}


//-----------------------------------------------------------------------------
// Name : GetViewMatrix ()
// Desc : Return the current view matrix.
//-----------------------------------------------------------------------------
const D3DXMATRIX& CCamera::GetViewMatrix()
{
    // Only update matrix if something has changed
    if ( m_bViewDirty ) 
    {
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

        // Set view matrix values
        m_mtxView._11 = m_vecRight.x; m_mtxView._12 = m_vecUp.x; m_mtxView._13 = m_vecLook.x;
	    m_mtxView._21 = m_vecRight.y; m_mtxView._22 = m_vecUp.y; m_mtxView._23 = m_vecLook.y;
	    m_mtxView._31 = m_vecRight.z; m_mtxView._32 = m_vecUp.z; m_mtxView._33 = m_vecLook.z;
	    m_mtxView._41 =- D3DXVec3Dot( &m_vecPos, &m_vecRight );
	    m_mtxView._42 =- D3DXVec3Dot( &m_vecPos, &m_vecUp    );
	    m_mtxView._43 =- D3DXVec3Dot( &m_vecPos, &m_vecLook  );

        // View Matrix has been updated
        m_bViewDirty = false;

    } // End If View Dirty

    // Return the view matrix.
    return m_mtxView;
}

//-----------------------------------------------------------------------------
// Name : UpdateRenderView ()
// Desc : Update the render device with the updated view matrix etc.
//-----------------------------------------------------------------------------
void CCamera::UpdateRenderView( LPDIRECT3DDEVICE9 pD3DDevice )
{  
    // Validate Parameters
    if (!pD3DDevice) return;

    // Update the device with this matrix.
    pD3DDevice->SetTransform( D3DTS_VIEW, &GetViewMatrix() );
}

//-----------------------------------------------------------------------------
// Name : UpdateRenderProj ()
// Desc : Update the render device with the updated projection matrix etc.
//-----------------------------------------------------------------------------
void CCamera::UpdateRenderProj( LPDIRECT3DDEVICE9 pD3DDevice )
{  
    // Validate Parameters
    if (!pD3DDevice) return;

    // Update the device with this matrix.
    pD3DDevice->SetTransform( D3DTS_PROJECTION, &GetProjMatrix() );
}

//-----------------------------------------------------------------------------
// Name : SetVolumeInfo ()
// Desc : Set the players collision volume information
//-----------------------------------------------------------------------------
void CCamera::SetVolumeInfo( const VOLUME_INFO& Volume )
{
    m_Volume = Volume;
}

//-----------------------------------------------------------------------------
// Name : GetVolumeInfo ()
// Desc : Retrieve the players collision volume information
//-----------------------------------------------------------------------------
const VOLUME_INFO& CCamera::GetVolumeInfo( ) const
{
    return m_Volume;
}

//-----------------------------------------------------------------------------
// CCam1stPerson Member Functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CCam1stPerson () (Alternate Constructor)
// Desc : CCam1stPerson Class Constructor, extracts values from the passed camera.
//-----------------------------------------------------------------------------
CCamFPSFly::CCamFPSFly( const CCamera * pCamera )
{
    // Update the camera from the camera passed
    SetCameraDetails( pCamera );
}

//-----------------------------------------------------------------------------
// Name : SetCameraDetails ()
// Desc : Sets this camera up based upon the camera passed in.
//-----------------------------------------------------------------------------
void CCamFPSFly::SetCameraDetails( const CCamera * pCamera )
{
    // Validate Parameters
    if (!pCamera) return;

    // Reset / Clear all required values
    m_vecPos    = pCamera->GetPosition();
    m_vecRight  = pCamera->GetRight();
    m_vecLook   = pCamera->GetLook();
    m_vecUp     = pCamera->GetUp();
    m_fFOV      = pCamera->GetFOV();
    m_fNearClip = pCamera->GetNearClip();
    m_fFarClip  = pCamera->GetFarClip();
    m_Viewport  = pCamera->GetViewport();
    m_Volume    = pCamera->GetVolumeInfo();

    // Rebuild both matrices
    m_bViewDirty = true;
    m_bProjDirty = true;

}

//-----------------------------------------------------------------------------
// Name : Rotate ()
// Desc : Rotate the camera about the players local axis.
//-----------------------------------------------------------------------------
void CCamFPSFly::Rotate( float x, float y, float z )
{
    D3DXMATRIX mtxRotate;

    // Validate requirements
    if (!m_pPlayer) return;

    if ( x != 0 ) 
    {
        // Build rotation matrix
        D3DXMatrixRotationAxis( &mtxRotate, &m_vecRight, D3DXToRadian( x ) );
        
        // Update our vectors
        D3DXVec3TransformNormal( &m_vecLook, &m_vecLook, &mtxRotate );
        D3DXVec3TransformNormal( &m_vecUp, &m_vecUp, &mtxRotate );
        D3DXVec3TransformNormal( &m_vecRight, &m_vecRight, &mtxRotate );

    } // End if Pitch

    if ( y != 0 ) 
    {
        // Build rotation matrix
        D3DXMatrixRotationAxis( &mtxRotate, &m_pPlayer->GetUp(), D3DXToRadian( y ) );
        
        // Update our vectors
        D3DXVec3TransformNormal( &m_vecLook, &m_vecLook, &mtxRotate );
        D3DXVec3TransformNormal( &m_vecUp, &m_vecUp, &mtxRotate );
        D3DXVec3TransformNormal( &m_vecRight, &m_vecRight, &mtxRotate );
    
    } // End if Yaw

    if ( z != 0 ) 
    {
        // Build rotation matrix
        D3DXMatrixRotationAxis( &mtxRotate, &m_pPlayer->GetLook(), D3DXToRadian( z ) );
        
        // Adjust camera position
        m_vecPos -= m_pPlayer->GetPosition();
        D3DXVec3TransformCoord ( &m_vecPos, &m_vecPos, &mtxRotate );
        m_vecPos += m_pPlayer->GetPosition();

        // Update our vectors
        D3DXVec3TransformNormal( &m_vecLook, &m_vecLook, &mtxRotate );
        D3DXVec3TransformNormal( &m_vecUp, &m_vecUp, &mtxRotate );
        D3DXVec3TransformNormal( &m_vecRight, &m_vecRight, &mtxRotate );
    
    } // End if Roll
        
    // Set view matrix as dirty
    m_bViewDirty = true;
}