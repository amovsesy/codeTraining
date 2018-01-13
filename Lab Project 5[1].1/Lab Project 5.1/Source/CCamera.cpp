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

    // Internal features are dirty by default
    m_bViewDirty      = true;
    m_bProjDirty      = true;
    m_bFrustumDirty   = true;

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

    // Internal features are dirty by default
    m_bViewDirty      = true;
    m_bProjDirty      = true;
    m_bFrustumDirty   = true;

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
    m_bFrustumDirty   = true;

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
        D3DXMatrixPerspectiveFovLH( &m_mtxProj, D3DXToRadian( m_fFOV ), fAspect, m_fNearClip, m_fFarClip );
            
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
// Name : CalcFrustumPlanes () (Private)
// Desc : Calculate the 6 frustum planes based on the current values.
//-----------------------------------------------------------------------------
void CCamera::CalcFrustumPlanes()
{
    // Only update planes if something has changed
    if ( !m_bFrustumDirty ) return;

    // Build a combined view & projection matrix
    D3DXMATRIX m = GetViewMatrix() * GetProjMatrix();

    // Left clipping plane
    m_Frustum[0].a = -(m._14 + m._11);
    m_Frustum[0].b = -(m._24 + m._21);
    m_Frustum[0].c = -(m._34 + m._31);
    m_Frustum[0].d = -(m._44 + m._41);

    // Right clipping plane
    m_Frustum[1].a = -(m._14 - m._11);
    m_Frustum[1].b = -(m._24 - m._21);
    m_Frustum[1].c = -(m._34 - m._31);
    m_Frustum[1].d = -(m._44 - m._41);

    // Top clipping plane
    m_Frustum[2].a = -(m._14 - m._12);
    m_Frustum[2].b = -(m._24 - m._22);
    m_Frustum[2].c = -(m._34 - m._32);
    m_Frustum[2].d = -(m._44 - m._42);

    // Bottom clipping plane
    m_Frustum[3].a = -(m._14 + m._12);
    m_Frustum[3].b = -(m._24 + m._22);
    m_Frustum[3].c = -(m._34 + m._32);
    m_Frustum[3].d = -(m._44 + m._42);

    // Near clipping plane
    m_Frustum[4].a = -(m._13);
    m_Frustum[4].b = -(m._23);
    m_Frustum[4].c = -(m._33);
    m_Frustum[4].d = -(m._43);

    // Far clipping plane
    m_Frustum[5].a = -(m._14 - m._13);
    m_Frustum[5].b = -(m._24 - m._23);
    m_Frustum[5].c = -(m._34 - m._33);
    m_Frustum[5].d = -(m._44 - m._43);

    // Normalize the m_Frustum
    for ( ULONG i = 0; i < 6; i++ ) D3DXPlaneNormalize( &m_Frustum[i], &m_Frustum[i] );

    // Frustum is no longer dirty
    m_bFrustumDirty = false;
}

//-----------------------------------------------------------------------------
// Name : BoundsInFrustum ()
// Desc : Determine whether or not the box passed is within the frustum.
//-----------------------------------------------------------------------------
bool CCamera::BoundsInFrustum( const D3DXVECTOR3 & Min, const D3DXVECTOR3 & Max )
{
    // First calculate the frustum planes
    CalcFrustumPlanes();

    ULONG i;
    D3DXVECTOR3 NearPoint, FarPoint, Normal;

    // Loop through all the planes
    for ( i = 0; i < 6; i++ )
    {
        // Store the plane normal
        Normal = D3DXVECTOR3( m_Frustum[i].a, m_Frustum[i].b, m_Frustum[i].c );

        if ( Normal.x > 0.0f )
        {
            if ( Normal.y > 0.0f )
            {
                if ( Normal.z > 0.0f ) 
                {
                    NearPoint.x = Min.x; NearPoint.y = Min.y; NearPoint.z = Min.z;
                
                } // End if Normal.z > 0
                else 
                {
                    NearPoint.x = Min.x; NearPoint.y = Min.y; NearPoint.z = Max.z; 
                
                } // End if Normal.z <= 0
            
            } // End if Normal.y > 0
            else
            {
                if ( Normal.z > 0.0f ) 
                {
                    NearPoint.x = Min.x; NearPoint.y = Max.y; NearPoint.z = Min.z;
                
                } // End if Normal.z > 0
                else 
                {
                    NearPoint.x = Min.x; NearPoint.y = Max.y; NearPoint.z = Max.z; 
                
                } // End if Normal.z <= 0

            } // End if Normal.y <= 0
        
        } // End if Normal.x > 0
        else
        {
            if ( Normal.y > 0.0f )
            {
                if ( Normal.z > 0.0f ) 
                {
                    NearPoint.x = Max.x; NearPoint.y = Min.y; NearPoint.z = Min.z;
                
                } // End if Normal.z > 0
                else 
                {
                    NearPoint.x = Max.x; NearPoint.y = Min.y; NearPoint.z = Max.z; 
                
                } // End if Normal.z <= 0
            
            } // End if Normal.y > 0
            else
            {
                if ( Normal.z > 0.0f ) 
                {
                    NearPoint.x = Max.x; NearPoint.y = Max.y; NearPoint.z = Min.z;
                
                } // End if Normal.z > 0
                else 
                {
                    NearPoint.x = Max.x; NearPoint.y = Max.y; NearPoint.z = Max.z; 
                
                } // End if Normal.z <= 0

            } // End if Normal.y <= 0

        } // End if Normal.x <= 0
                
        // Near extreme point is outside, and thus the
        // AABB is totally outside the frustum ?
        if ( D3DXVec3Dot( &Normal, &NearPoint ) + m_Frustum[i].d > 0.0f ) return false;

    } // Next Plane

    // Is within the frustum
    return true;
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
CCam1stPerson::CCam1stPerson( const CCamera * pCamera )
{
    // Update the camera from the camera passed
    SetCameraDetails( pCamera );
}

//-----------------------------------------------------------------------------
// Name : SetCameraDetails ()
// Desc : Sets this camera up based upon the camera passed in.
//-----------------------------------------------------------------------------
void CCam1stPerson::SetCameraDetails( const CCamera * pCamera )
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

    // If we are switching building from a spacecraft style cam
    if ( pCamera->GetCameraMode() == MODE_SPACECRAFT )
    {
        // Flatten out the vectors
        m_vecUp      = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
        m_vecRight.y = 0.0f;
        m_vecLook.y  = 0.0f;

        // Finally, normalize them
        D3DXVec3Normalize( &m_vecRight, &m_vecRight );
        D3DXVec3Normalize( &m_vecLook, &m_vecLook );

    } // End if MODE_SPACECRAFT

    // Rebuild both matrices and frustum
    m_bViewDirty    = true;
    m_bProjDirty    = true;
    m_bFrustumDirty = true;

}

//-----------------------------------------------------------------------------
// Name : Rotate ()
// Desc : Rotate the camera about the players local axis.
//-----------------------------------------------------------------------------
void CCam1stPerson::Rotate( float x, float y, float z )
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

    // Frustum is dirty by definition
    m_bFrustumDirty = true;
}



//-----------------------------------------------------------------------------
// CCam3rdPerson Member Functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CCam3rdPerson () (Alternate Constructor)
// Desc : CCam3rdPerson Class Constructor, extracts values from the passed camera.
//-----------------------------------------------------------------------------
CCam3rdPerson::CCam3rdPerson( const CCamera * pCamera )
{
    // Update the camera from the camera passed
    SetCameraDetails( pCamera );
}

//-----------------------------------------------------------------------------
// Name : SetCameraDetails ()
// Desc : Sets this camera up based upon the camera passed in.
//-----------------------------------------------------------------------------
void CCam3rdPerson::SetCameraDetails( const CCamera * pCamera )
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

    // If we are switching building from a spacecraft style cam
    if ( pCamera->GetCameraMode() == MODE_SPACECRAFT )
    {
        // Flatten out the vectors
        m_vecUp      = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
        m_vecRight.y = 0.0f;
        m_vecLook.y  = 0.0f;

        // Finally, normalize them
        D3DXVec3Normalize( &m_vecRight, &m_vecRight );
        D3DXVec3Normalize( &m_vecLook, &m_vecLook );

    } // End if MODE_SPACECRAFT

    // Rebuild both matrices and frustum
    m_bViewDirty    = true;
    m_bProjDirty    = true;
    m_bFrustumDirty = true;

}

//-----------------------------------------------------------------------------
// Name : Update ()
// Desc : Allow our camera to update
//-----------------------------------------------------------------------------
void CCam3rdPerson::Update( float TimeScale, float Lag )
{
    D3DXMATRIX  mtxRotate;
    D3DXVECTOR3 vecOffset, vecPosition, vecDir;

    // Validate requirements
    if (!m_pPlayer) return;

    float fTimeScale = 1.0f, Length = 0.0f;
    if ( Lag != 0.0f ) fTimeScale = TimeScale * (1.0f / Lag);

    // Build a rotation matrix so that we can rotate our offset vector to it's position behind the player
    D3DXMatrixIdentity( &mtxRotate );
    D3DXVECTOR3 vecRight = m_pPlayer->GetRight(), vecUp = m_pPlayer->GetUp(), vecLook = m_pPlayer->GetLook();
    mtxRotate._11 = vecRight.x; mtxRotate._21 = vecUp.x; mtxRotate._31 = vecLook.x;
	mtxRotate._12 = vecRight.y; mtxRotate._22 = vecUp.y; mtxRotate._32 = vecLook.y;
	mtxRotate._13 = vecRight.z; mtxRotate._23 = vecUp.z; mtxRotate._33 = vecLook.z;

    // Calculate our rotated offset vector
    D3DXVec3TransformCoord( &vecOffset, &m_pPlayer->GetCamOffset(), &mtxRotate );
    
    // vecOffset now contains information to calculate where our camera position SHOULD be.
    vecPosition = m_pPlayer->GetPosition() + vecOffset;
    vecDir = vecPosition - m_vecPos;
    Length = D3DXVec3Length( &vecDir );
    D3DXVec3Normalize( &vecDir, &vecDir );

    // Move based on camera lag
    float Distance = Length * fTimeScale;
    if ( Distance > Length ) Distance = Length;
    
    // If we only have a short way to travel, move all the way
    if ( Length < 0.01f ) Distance = Length;
    
    // Update our camera
    if ( Distance > 0 )
    {
        m_vecPos += vecDir * Distance;
    
        // Ensure our camera is looking at the axis origin
        SetLookAt( m_pPlayer->GetPosition() );

        // Our view matrix parameters have been update
        m_bViewDirty    = true;
        m_bFrustumDirty = true;

    } // End if

}

//-----------------------------------------------------------------------------
// Name : SetLookAt () (Private)
// Desc : Set the cameras look at vector, this function will also update the
//        various direction vectors so that they are correct.
//-----------------------------------------------------------------------------
void CCam3rdPerson::SetLookAt( const D3DXVECTOR3& vecLookAt )
{
    D3DXMATRIX Matrix;

    // Generate a look at matrix
    D3DXMatrixLookAtLH( &Matrix, &m_vecPos, &vecLookAt, &m_pPlayer->GetUp() );
    
    // Extract the vectors
    m_vecRight = D3DXVECTOR3( Matrix._11, Matrix._21, Matrix._31 );
    m_vecUp    = D3DXVECTOR3( Matrix._12, Matrix._22, Matrix._32 );
    m_vecLook  = D3DXVECTOR3( Matrix._13, Matrix._23, Matrix._33 );

    // Set view matrix as dirty
    m_bViewDirty = true;

    // Frustum is now dirty by default
    m_bFrustumDirty   = true;
}



//-----------------------------------------------------------------------------
// CCamSpaceCraft Member Functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CCamSpaceCraft () (Alternate Constructor)
// Desc : CCamSpaceCraft Class Constructor, extracts values from the passed camera.
//-----------------------------------------------------------------------------
CCamSpaceCraft::CCamSpaceCraft( const CCamera * pCamera )
{
    // Update the camera from the camera passed
    SetCameraDetails( pCamera );
}

//-----------------------------------------------------------------------------
// Name : SetCameraDetails ()
// Desc : Sets this camera up based upon the camera passed in.
//-----------------------------------------------------------------------------
void CCamSpaceCraft::SetCameraDetails( const CCamera * pCamera )
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

    // Rebuild both matrices and frustum
    m_bViewDirty    = true;
    m_bProjDirty    = true;
    m_bFrustumDirty = true;

}

//-----------------------------------------------------------------------------
// Name : Rotate ()
// Desc : Rotate the camera about the cameras local axis.
// Note : Just performs a standard three vector rotation. This is essentially 
//        how we perform 'anti-gimble-lock' space craft rotations.
//-----------------------------------------------------------------------------
void CCamSpaceCraft::Rotate( float x, float y, float z )
{
    D3DXMATRIX mtxRotate;

    // Validate Requirements
    if (!m_pPlayer) return;

    if ( x != 0 ) 
    {
        // Build rotation matrix
        D3DXMatrixRotationAxis( &mtxRotate, &m_pPlayer->GetRight(), D3DXToRadian( x ) );
        D3DXVec3TransformNormal( &m_vecLook, &m_vecLook, &mtxRotate );
        D3DXVec3TransformNormal( &m_vecUp, &m_vecUp, &mtxRotate );

        // Adjust position
        m_vecPos -= m_pPlayer->GetPosition();
        D3DXVec3TransformCoord( &m_vecPos, &m_vecPos, &mtxRotate );
        m_vecPos += m_pPlayer->GetPosition();


    } // End if Pitch

    if ( y != 0 ) 
    {
        // Build rotation matrix
        D3DXMatrixRotationAxis( &mtxRotate, &m_pPlayer->GetUp(), D3DXToRadian( y ) );
        D3DXVec3TransformNormal( &m_vecLook, &m_vecLook, &mtxRotate );
        D3DXVec3TransformNormal( &m_vecRight, &m_vecRight, &mtxRotate );

        // Adjust position
        m_vecPos -= m_pPlayer->GetPosition();
        D3DXVec3TransformCoord( &m_vecPos, &m_vecPos, &mtxRotate );
        m_vecPos += m_pPlayer->GetPosition();
    
    } // End if Yaw

    if ( z != 0 ) 
    {
        // Build rotation matrix
        D3DXMatrixRotationAxis( &mtxRotate, &m_pPlayer->GetLook(), D3DXToRadian( z ) );
        D3DXVec3TransformNormal( &m_vecUp, &m_vecUp, &mtxRotate );
        D3DXVec3TransformNormal( &m_vecRight, &m_vecRight, &mtxRotate );

        // Adjust position
        m_vecPos -= m_pPlayer->GetPosition();
        D3DXVec3TransformCoord( &m_vecPos, &m_vecPos, &mtxRotate );
        m_vecPos += m_pPlayer->GetPosition();
    
    } // End if Roll
        
    // Set view matrix as dirty
    m_bViewDirty = true;

    // Frustum is now dirty by default
    m_bFrustumDirty = true;
}
