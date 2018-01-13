//-----------------------------------------------------------------------------
// File: CScene.cpp
//
// Desc: Contains scene related data such as meshes, lights, materials etc.
//
// Copyright (c) 1997-2002 Adam Hoult & Gary Simmons. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CScene Specific Includes
//-----------------------------------------------------------------------------
#include "..\\Includes\\CScene.h"
#include "..\\Includes\\CObject.h"
#include "..\\Includes\\CTimer.h"

//-----------------------------------------------------------------------------
// IWF File Reading includes
//-----------------------------------------------------------------------------
#include "..\\Libs\\libIWF.h"
#include "..\\Libs\\iwfFile.h"
#include "..\\Libs\\iwfObjects.h"

//-----------------------------------------------------------------------------
// Module Local Constants
//-----------------------------------------------------------------------------
namespace
{
    const LPCSTR TexturePath = "Data\\";    // Location of texture data.
};

//-----------------------------------------------------------------------------
// Name : CScene () (Constructor)
// Desc : CScene Class Constructor
//-----------------------------------------------------------------------------
CScene::CScene()
{
	// Reset / Clear all required values
    m_nLightLimit      = 0;
    m_nReservedLights  = 0;

    m_nMaterialCount   = 0;
    m_nTextureCount    = 0;
    m_nLightCount      = 0;
    m_nLightGroupCount = 0;
    m_nWaterTexture    = -1;
    m_pMaterialList    = NULL;
    m_pTextureList     = NULL;
    m_pLightList       = NULL;
    m_ppLightGroupList = NULL;
    m_pD3DDevice       = NULL;
    m_bHardwareTnL     = false;

    // Set up our dynamic light properties
    ZeroMemory( &m_DynamicLight, sizeof(D3DLIGHT9) );
    m_DynamicLight.Type      = D3DLIGHT_POINT;
    m_DynamicLight.Range     = 150.0f;
    m_DynamicLight.Diffuse.a = 1.0f;
    m_DynamicLight.Diffuse.r = 1.0f;
    m_DynamicLight.Position  = D3DXVECTOR3( 290, 10, 500 );
    m_DynamicLight.Attenuation0 = 1.0f;

    // Set our texture matrix to identity
    D3DXMatrixIdentity( &m_mtxTexture );
}

//-----------------------------------------------------------------------------
// Name : ~CScene () (Destructor)
// Desc : CScene Class Destructor
//-----------------------------------------------------------------------------
CScene::~CScene()
{
    // Release allocated resources
    Release();

}

//-----------------------------------------------------------------------------
// Name : Release ()
// Desc : Release all active resources
//-----------------------------------------------------------------------------
void CScene::Release( )
{
    ULONG i;

    // Release any allocated memory
    if ( m_ppLightGroupList )
    {
        for ( i = 0; i < m_nLightGroupCount; i++ )
        {
            if ( m_ppLightGroupList[i] ) delete m_ppLightGroupList[i];
    
        } // Next Light Group

        delete []m_ppLightGroupList;
    
    } // End if Light Groups

    // Release any allocated textures 
    if ( m_pTextureList )
    {
        for ( i = 0; i < m_nTextureCount; i++ )
        {
            if ( m_pTextureList[i] ) m_pTextureList[i]->Release();
    
        } // Next Light Group

        delete []m_pTextureList;
    
    } // End if Textures

    // Release flat arrays
    if ( m_pMaterialList ) delete []m_pMaterialList;
    if ( m_pLightList ) delete []m_pLightList;

    // Release Direct3D Objects
    if ( m_pD3DDevice ) m_pD3DDevice->Release();

    // Clear Variables
    m_nMaterialCount   = 0;
    m_nLightCount      = 0;
    m_nLightGroupCount = 0;
    m_pTextureList     = NULL;
    m_pMaterialList    = NULL;
    m_pLightList       = NULL;
    m_ppLightGroupList = NULL;
    m_pD3DDevice       = NULL;
    m_bHardwareTnL     = false;

}

//-----------------------------------------------------------------------------
// Name : SetD3DDevice()
// Desc : Sets the D3D Device that will be used for buffer creation and renering
//-----------------------------------------------------------------------------
void CScene::SetD3DDevice( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL )
{
    // Validate Parameters
    if ( !pD3DDevice ) return;

    // Store D3D Device and add a reference
    m_pD3DDevice = pD3DDevice;
    m_pD3DDevice->AddRef();

    // Store vertex processing type for buffer creation
    m_bHardwareTnL = HardwareTnL;
}

//-----------------------------------------------------------------------------
// Name : SetTextureFormat()
// Desc : Informs our scene manager with which format standard textures should
//        be created.
//-----------------------------------------------------------------------------
void CScene::SetTextureFormat( const D3DFORMAT & Format )
{
    // Store texture format
    m_fmtTexture = Format;
}

//-----------------------------------------------------------------------------
// Name : LoadScene ()
// Desc : Loads in the specified IWF scene file.
//-----------------------------------------------------------------------------
bool CScene::LoadScene( TCHAR * strFileName, ULONG LightLimit /* = 0 */, ULONG LightReservedCount /* = 0 */ )
{
    CFileIWF File;

    // File loading may throw an exception
    try
    {
        // Attempt to load the file
        File.Load( strFileName );

        // Copy over the entities and materials we want from the file
        if (!ProcessEntities( File )) return false;
        if (!ProcessMaterials( File )) return false;
        if (!ProcessTextures( File )) return false;

        // Store values
        m_nLightLimit     = LightLimit;
        m_nReservedLights = LightReservedCount;

        // Check for unlimited light sources
        if ( m_nLightLimit == 0 ) m_nLightLimit = m_nLightCount + LightReservedCount;
        
        // Now process the meshes and extract the required data
        if (!ProcessMeshes( File )) return false;

        // Build vertex / index buffers
        for ( USHORT i = 0; i < m_nLightGroupCount; i++ )
        {
            if ( !m_ppLightGroupList[i]->BuildBuffers( m_pD3DDevice, m_bHardwareTnL, true )) return false;

        } // Next Light Group

        // Allow file loader to release any active objects
        File.ClearObjects();
        
    } // End Try Block

    // Catch any exceptions
    catch (...)
    {
        return false;
    
    } // End Catch Block

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : ProcessEntities () (Private)
// Desc : Processes the entities stored inside the file object passed
// Note : Discards all entities other than lights in this demonstration.
//-----------------------------------------------------------------------------
bool CScene::ProcessEntities( const CFileIWF& File )
{
    D3DLIGHT9 Light;
    ULONG     i;
    ULONG     LightCount = 0;

    // Count the number of light entities
    // Loop through and build our lights
    for ( i = 0; i < File.m_vpEntityList.size(); i++ )
    {
        // Retrieve pointer to file entity
        iwfEntity * pFileEntity = File.m_vpEntityList[i];

        // Only build if this is a light entity
        if ( pFileEntity->EntityTypeID == ENTITY_LIGHT && pFileEntity->DataSize > 0 ) LightCount++;
        
    } // Next Material

    // Detect no-op
    if ( LightCount == 0 ) return true;

    // Allocate enough space for all our lights
    m_pLightList = new D3DLIGHT9[ LightCount ];
    if (!m_pLightList) return false;

    // Loop through and build our lights
    for ( i = 0; i < File.m_vpEntityList.size(); i++ )
    {
        // Retrieve pointer to file entity
        iwfEntity * pFileEntity = File.m_vpEntityList[i];

        // Only build if this is a light entity
        if ( pFileEntity->EntityTypeID == ENTITY_LIGHT && pFileEntity->DataSize > 0 )
        {
            LIGHTENTITY * pFileLight = (LIGHTENTITY*)pFileEntity->DataArea;
            
            // Skip if this is not a valid light type (Not relevant to the API)
            if ( pFileLight->LightType == LIGHTTYPE_AMBIENT ) continue;

            // Extract the light values we need
            Light.Type         = (D3DLIGHTTYPE)(pFileLight->LightType + 1);
            Light.Diffuse      = D3DXCOLOR( pFileLight->DiffuseRed, pFileLight->DiffuseGreen, pFileLight->DiffuseBlue, pFileLight->DiffuseAlpha );
            Light.Ambient      = D3DXCOLOR( pFileLight->AmbientRed, pFileLight->AmbientGreen, pFileLight->AmbientBlue, pFileLight->AmbientAlpha );
            Light.Specular     = D3DXCOLOR( pFileLight->SpecularRed, pFileLight->SpecularGreen, pFileLight->SpecularBlue, pFileLight->SpecularAlpha );
            Light.Position     = D3DXVECTOR3( pFileEntity->ObjectMatrix._41, pFileEntity->ObjectMatrix._42, pFileEntity->ObjectMatrix._43 );
            Light.Direction    = D3DXVECTOR3( pFileEntity->ObjectMatrix._31, pFileEntity->ObjectMatrix._32, pFileEntity->ObjectMatrix._33 );
            Light.Range        = pFileLight->Range;
            Light.Attenuation0 = pFileLight->Attenuation0;
            Light.Attenuation1 = pFileLight->Attenuation1;
            Light.Attenuation2 = pFileLight->Attenuation2;
            Light.Falloff      = pFileLight->FallOff;
            Light.Theta        = pFileLight->Theta;
            Light.Phi          = pFileLight->Phi;

            // Add this to our vector
            m_pLightList[ m_nLightCount++ ] = Light;
            
        } // End if light
        
    } // Next Entity

    // Success!
    return true;

}

//-----------------------------------------------------------------------------
// Name : ProcessMaterials () (Private)
// Desc : Processes the materials stored inside the file object passed
//-----------------------------------------------------------------------------
bool CScene::ProcessMaterials( const CFileIWF& File )
{
    ULONG i;
    
    // Allocate enough room for all of our materials
    m_pMaterialList = new D3DMATERIAL9[ File.m_vpMaterialList.size() ];
    if ( !m_pMaterialList ) return false;
    m_nMaterialCount = File.m_vpMaterialList.size();

    // Loop through and build our materials
    for ( i = 0; i < File.m_vpMaterialList.size(); i++ )
    {
        // Retrieve pointer to file material
        iwfMaterial * pFileMaterial = File.m_vpMaterialList[i];

        // Retrieve pointer to our local material
        D3DMATERIAL9 * pMaterial = &m_pMaterialList[i];

        // Copy over the data we need from the file material
        pMaterial->Diffuse  = (D3DCOLORVALUE&)pFileMaterial->Diffuse;
        pMaterial->Ambient  = (D3DCOLORVALUE&)pFileMaterial->Ambient;
        pMaterial->Emissive = (D3DCOLORVALUE&)pFileMaterial->Emissive;
        pMaterial->Specular = (D3DCOLORVALUE&)pFileMaterial->Specular;
        pMaterial->Power    = pFileMaterial->Power;
        
    } // Next Material

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : ProcessTextures () (Private)
// Desc : Processes the textures stored inside the file object passed
//-----------------------------------------------------------------------------
bool CScene::ProcessTextures( const CFileIWF& File )
{
    ULONG i;
    char  FileName[MAX_PATH];
    
    // Allocate enough room for all of our textures
    m_pTextureList = new LPDIRECT3DTEXTURE9[ File.m_vpTextureList.size() ];
    if ( !m_pTextureList ) return false;
    m_nTextureCount = File.m_vpTextureList.size();

    // Loop through and build our textures
    ZeroMemory( m_pTextureList, m_nTextureCount * sizeof(LPDIRECT3DTEXTURE9));
    for ( i = 0; i < File.m_vpTextureList.size(); i++ )
    {
        // Retrieve pointer to file texture
        TEXTURE_REF * pFileTexture = File.m_vpTextureList[i];

        // Skip if this is an internal texture (not supported by this demo)
        if ( pFileTexture->TextureSource != TEXTURE_EXTERNAL ) continue;

        // Build the final texture path
        strcpy( FileName, TexturePath );
        strcat( FileName, pFileTexture->Name );
        
        // Load the texture from file
        D3DXCreateTextureFromFileEx( m_pD3DDevice, FileName, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
                                     0, m_fmtTexture, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 
                                     0, NULL, NULL, &m_pTextureList[i] );

        // Store the index to the texture we want to animate if this is the one
        if ( stricmp( pFileTexture->Name, "Water Bump Map 001.jpg") == 0 ) m_nWaterTexture = i;
        
    } // Next Texture

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : ProcessMeshes () (Private)
// Desc : Processes the meshes stored inside the file object passed
//-----------------------------------------------------------------------------
bool CScene::ProcessMeshes( CFileIWF & pFile )
{
    long i, j, k, l, m, TextureIndex, MaterialIndex;
    CLightGroup    * pLightGroup = NULL;
    CPropertyGroup * pTexProperty = NULL;
    CPropertyGroup * pMatProperty = NULL;

    // Allocate the light groups, and assign the surfaces to them
    if (!BuildLightGroups( pFile )) return false;
    
    // For each texture (including no texture, -1)
    for ( l = -1; l < (signed)m_nTextureCount; l++ )
    {
        // For each material (including no material, -1)
        for ( m = -1; m < (signed)m_nMaterialCount; m++ )
        {
            // Loop through each surface of each mesh
            for ( i = 0; i < pFile.m_vpMeshList.size(); i++ )
            {
                iwfMesh * pMesh = pFile.m_vpMeshList[i];

                for ( j = 0; j < pMesh->SurfaceCount; j++ )
                {
                    iwfSurface * pSurface = pMesh->Surfaces[j];

                    // Determine the indices we are using.
                    MaterialIndex = -1;
                    TextureIndex  = -1;
                    if ( (pSurface->Components & SCOMPONENT_MATERIALS) && pSurface->ChannelCount > 0 ) MaterialIndex = pSurface->MaterialIndices[0];
                    if ( (pSurface->Components & SCOMPONENT_TEXTURES ) && pSurface->ChannelCount > 0 ) TextureIndex  = pSurface->TextureIndices[0];    

                    // Skip if this is not in order
                    if ( TextureIndex != l || MaterialIndex != m ) continue;

                    // Retrieve the lightgroup pointer for this surface
                    pLightGroup = (CLightGroup*)pSurface->CustomData;

                    // Determine if we already have a property group for this texture
                    for ( k = 0; k < pLightGroup->m_nPropertyGroupCount; k++ )
                    {
                        // Break if texture index matches
                        if ( (long)pLightGroup->m_pPropertyGroup[k]->m_nPropertyData == TextureIndex ) break;

                    } // Next Group

                    // If we didn't have this property group, add it
                    if ( k == pLightGroup->m_nPropertyGroupCount )
                    {
                        if ( pLightGroup->AddPropertyGroup( ) < 0 ) return false;

                        // Set up property group data for primary key
                        pTexProperty = pLightGroup->m_pPropertyGroup[ k ];
                        pTexProperty->m_PropertyType  = CPropertyGroup::PROPERTY_TEXTURE;
                        pTexProperty->m_nPropertyData = (ULONG)TextureIndex;
    
                    } // End if no group

                    // Process for secondary key (material)
                    pTexProperty = pLightGroup->m_pPropertyGroup[ k ];
            
                    // Determine if we already have a property group for this material
                    for ( k = 0; k < pTexProperty->m_nPropertyGroupCount; k++ )
                    {
                        // Break if texture index matches
                        if ( (long)pTexProperty->m_pPropertyGroup[k]->m_nPropertyData == MaterialIndex ) break;

                    } // Next Group

                    // If we didn't have this property group, add it
                    if ( k == pTexProperty->m_nPropertyGroupCount )
                    {
                        if ( pTexProperty->AddPropertyGroup( ) < 0 ) return false;

                        // Set up property group data for primary key
                        pMatProperty = pTexProperty->m_pPropertyGroup[ k ];
                        pMatProperty->m_PropertyType  = CPropertyGroup::PROPERTY_MATERIAL;
                        pMatProperty->m_nPropertyData = (ULONG)MaterialIndex;
                        pMatProperty->m_nVertexStart  = pLightGroup->m_nVertexCount;
                        pMatProperty->m_nVertexCount  = 0;
    
                    } // End if no group

                    // Process the vertices / indices and store in this property group
                    pMatProperty = pTexProperty->m_pPropertyGroup[ k ];
                    if (!ProcessIndices( pLightGroup, pMatProperty, pSurface ) ) return false;
                    if (!ProcessVertices( pLightGroup, pMatProperty, pSurface ) ) return false;

                } // Next Surface

            } // Next Mesh

        } // Next Material
    
    } // Next Texture

    // Clear the custom data pointer so that it isn't released
    for ( i = 0; i < pFile.m_vpMeshList.size(); i++ )
    {
        iwfMesh * pMesh = pFile.m_vpMeshList[i];
        for ( j = 0; j < pMesh->SurfaceCount; j++ ) pMesh->Surfaces[j]->CustomData = NULL;

    } // Next Mesh

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : BuildLightGroups () (Private)
// Desc : Processes the entities stored inside the file object passed
// Note : Pass in the light limit imposed by the device (this information can
//        be found via D3DCAPS9::MaxActiveLights), and also the number of slots
//        in each group (Starting from 0) you would like to remain free for
//        dynamic lights for instance. We only return false on failure.
//-----------------------------------------------------------------------------
bool CScene::BuildLightGroups( CFileIWF & pFile )
{
    ULONG           i, j, k, n, *SelectedLights = NULL, LightCount = 0;
    float          *LightContribution = NULL, BestScore = 0.0f;
    CLightGroup    *pLightGroup = NULL;
    CPropertyGroup *pProperty = NULL;
    long            BestLight = -1;

    // Setup our light contribution tables
    LightContribution = new float[ m_nLightCount ];
    if (!LightContribution) goto BuildFailure;

    SelectedLights = new ULONG[ (m_nLightLimit - m_nReservedLights) ];
    if (!SelectedLights) goto BuildFailure;
    
    // Loop through each Mesh
    for ( n = 0; n < pFile.m_vpMeshList.size(); n++ )
    {
        iwfMesh * pMesh = pFile.m_vpMeshList[n];

        // Loop through each mesh surface
        for ( i = 0; i < pMesh->SurfaceCount; i++ )
        {
            iwfSurface * pSurface = pMesh->Surfaces[i];

            // Now we will determine which lights affect this surface
            ZeroMemory( LightContribution, m_nLightCount * sizeof(float));
            for ( j = 0; j < m_nLightCount; j++ )
            {    
                // Calculate the contribution this light gives this mesh.
                LightContribution[j] = GetLightContribution( pSurface, &m_pLightList[j] );

            } // Next Light

            // Now we have the light contribution table, we can select
            // the best lights for the job (with an acceptable error)
            LightCount = 0;
            for ( j = 0; j < (m_nLightLimit - m_nReservedLights); j++ )
            {
                // Reset our best score
                BestScore = 0.0f;
                BestLight = -1;

                // Find the light with the best score
                for ( k = 0; k < m_nLightCount; k++ )
                {
                    if ( LightContribution[ k ] > BestScore )
                    {
                        BestScore = LightContribution[ k ];
                        BestLight = k;
                
                    } // End if better than previous
            
                } // Next Light

                // Have we run out of lights ?
                if ( BestLight < 0 ) break;

                // Select our best light. We reset it's score here.
                SelectedLights[ LightCount++ ] = BestLight;
                LightContribution[ BestLight ] = 0.0f;
        
            } // Next Light Slot

            // We now have a list of all the best scoring lights
            // up to our light limit. We can now add this surface to
            // a matching light group, or create a new one if none exists.
            pLightGroup = NULL;
            for ( j = 0; j < m_nLightGroupCount; j++ )
            {
                if ( m_ppLightGroupList[j]->GroupMatches( LightCount, SelectedLights ) )
                {
                    // Select this light group and bail
                    pLightGroup = m_ppLightGroupList[j];
                    break;
            
                } // End if group matches
             
            } // Next Light Group

            // If we didn't find a light group, allocate and add one
            if ( !pLightGroup )
            {
                if (!(pLightGroup = new CLightGroup) ) goto BuildFailure;
            
                // Add it to the list
                if ( AddLightGroup( 1 ) < 0 ) goto BuildFailure;
                m_ppLightGroupList[ m_nLightGroupCount - 1 ] = pLightGroup;
                pLightGroup->SetLights( LightCount, SelectedLights );
            
            } // End if no group found

            // We are about to make use of the custom data pointer, so discard
            // any custom data loaded in from file.
            if ( pSurface->CustomData ) delete[] pSurface->CustomData;
            pSurface->CustomDataSize = 0;

            // Store the lightgroup to which this surface belongs (used later)
            pSurface->CustomData = (UCHAR*)pLightGroup;
        
        } // Next Surface

    } // Next Mesh

    // Release memory
    if ( LightContribution ) delete []LightContribution;
    if ( SelectedLights ) delete []SelectedLights;

    // Success!
    return true;

BuildFailure:
    // If we dropped here, something bad happened :)
    if ( LightContribution ) delete []LightContribution;
    if ( SelectedLights ) delete []SelectedLights;

    // Failure!
    return false;
}

//-----------------------------------------------------------------------------
// Name : ProcessIndices () (Private)
// Desc : Processes the indices stored inside the polygon object passed
// Note : This performs the brunt of the conversion between primitive types.
//-----------------------------------------------------------------------------
bool CScene::ProcessIndices( CLightGroup * pLightGroup, CPropertyGroup * pProperty, iwfSurface * pFilePoly )
{
    ULONG i, Counter, VertexStart, IndexCount;
    
    // Store current property vertex start and index start
    VertexStart = pLightGroup->m_nVertexCount - pProperty->m_nVertexStart; // Ensure property indices start from 0
    IndexCount  = pProperty->m_nIndexCount;

    // Generate indices
    if ( pFilePoly->IndexCount > 0 )
    {
        ULONG IndexType = pFilePoly->IndexFlags & INDICES_MASK_TYPE;
    
        // Interpret indices (we want them in tri-list format)
        switch ( IndexType )
        {
            case INDICES_TRILIST:
            
                // We can do a straight copy (converting from 32bit to 16bit)
                if ( pProperty->AddIndex( pFilePoly->IndexCount ) < 0 ) return false;
                for ( i = 0; i < pFilePoly->IndexCount; i++ ) pProperty->m_pIndex[i + IndexCount] = pFilePoly->Indices[i] + VertexStart;
                break;

            case INDICES_TRISTRIP:
            
                // Index in strip order
                if ( pProperty->AddIndex( (pFilePoly->IndexCount - 2) * 3 ) < 0 ) return false;
                for ( Counter = IndexCount, i = 0; i < pFilePoly->IndexCount - 2; i++ )
                {
                    // Starting with triangle 0.
                    // Is this an 'Odd' or 'Even' triangle
                    if ( (i % 2) == 0 )
                    {
                        pProperty->m_pIndex[ Counter++ ] = pFilePoly->Indices[ i ] + VertexStart;
                        pProperty->m_pIndex[ Counter++ ] = pFilePoly->Indices[ i + 1 ] + VertexStart;
                        pProperty->m_pIndex[ Counter++ ] = pFilePoly->Indices[ i + 2 ] + VertexStart;
                    
                    } // End if 'Even' triangle
                    else
                    {
                        pProperty->m_pIndex[ Counter++ ] = pFilePoly->Indices[ i ] + VertexStart;
                        pProperty->m_pIndex[ Counter++ ] = pFilePoly->Indices[ i + 2 ] + VertexStart;
                        pProperty->m_pIndex[ Counter++ ] = pFilePoly->Indices[ i + 1 ] + VertexStart;

                    } // End if 'Odd' triangle

                } // Next vertex

                break;

            case INDICES_TRIFAN:

                // Index in fan order.
                if ( pProperty->AddIndex( (pFilePoly->IndexCount - 2 ) * 3 ) < 0 ) return false;
                for ( Counter = IndexCount, i = 1; i < pFilePoly->VertexCount - 1; i++ )
                {
                    pProperty->m_pIndex[ Counter++ ] = pFilePoly->Indices[ 0 ] + VertexStart;
                    pProperty->m_pIndex[ Counter++ ] = pFilePoly->Indices[ i ] + VertexStart;
                    pProperty->m_pIndex[ Counter++ ] = pFilePoly->Indices[ i + 1 ] + VertexStart;

                } // Next Triangle

                break;

        } // End Switch

    } // End if Indices Stored
    else
    {
        // We are going to try and build the indices ourselves
        ULONG VertexType = pFilePoly->VertexFlags & VERTICES_MASK_TYPE;

        // Interpret vertices (we want our indices in tri-list format)
        switch ( VertexType )
        {
            case VERTICES_TRILIST:

                // Straight fill
                if ( pProperty->AddIndex( pFilePoly->VertexCount ) < 0 ) return false;
                for ( i = 0; i < pFilePoly->VertexCount; i++ ) pProperty->m_pIndex[i + IndexCount] = i + VertexStart;

                break;

            case VERTICES_TRISTRIP:

                // Index in strip order
                if ( pProperty->AddIndex( (pFilePoly->VertexCount - 2) * 3 ) < 0 ) return false;
                for ( Counter = IndexCount, i = 0; i < pFilePoly->VertexCount - 2; i++ )
                {
                    // Starting with triangle 0.
                    // Is this an 'Odd' or 'Even' triangle
                    if ( (i % 2) == 0 )
                    {
                        pProperty->m_pIndex[ Counter++ ] = i + VertexStart;
                        pProperty->m_pIndex[ Counter++ ] = i + 1 + VertexStart;
                        pProperty->m_pIndex[ Counter++ ] = i + 2 + VertexStart;
                    
                    } // End if 'Even' triangle
                    else
                    {
                        pProperty->m_pIndex[ Counter++ ] = i + VertexStart;
                        pProperty->m_pIndex[ Counter++ ] = i + 2 + VertexStart;
                        pProperty->m_pIndex[ Counter++ ] = i + 1 + VertexStart;

                    } // End if 'Odd' triangle

                } // Next vertex

                break;

            case VERTICES_TRIFAN:
            
                // Index in fan order.
                if ( pProperty->AddIndex( (pFilePoly->VertexCount - 2 ) * 3 ) < 0 ) return false;
                for ( Counter = IndexCount, i = 1; i < pFilePoly->VertexCount - 1; i++ )
                {
                    pProperty->m_pIndex[ Counter++ ] = VertexStart;
                    pProperty->m_pIndex[ Counter++ ] = i + VertexStart;
                    pProperty->m_pIndex[ Counter++ ] = i + 1 + VertexStart;

                } // Next Triangle

                break;

        } // End Switch

    } // End if no Indices stored
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : ProcessVertices () (Private)
// Desc : Processes the vertices stored inside the polygon object passed
//-----------------------------------------------------------------------------
bool CScene::ProcessVertices( CLightGroup * pLightGroup, CPropertyGroup *pProperty, iwfSurface * pFilePoly )
{
    ULONG i, VertexStart = pLightGroup->m_nVertexCount;

    // Allocate enough vertices
    if ( pLightGroup->AddVertex( pFilePoly->VertexCount ) < 0 ) return false;

    // Loop through each vertex and copy required data.
    for ( i = 0; i < pFilePoly->VertexCount; i++ )
    {
        // Copy over vertex data
        pLightGroup->m_pVertex[i + VertexStart].x      = pFilePoly->Vertices[i].x;
        pLightGroup->m_pVertex[i + VertexStart].y      = pFilePoly->Vertices[i].y;
        pLightGroup->m_pVertex[i + VertexStart].z      = pFilePoly->Vertices[i].z;
        pLightGroup->m_pVertex[i + VertexStart].Normal = (D3DXVECTOR3&)pFilePoly->Vertices[i].Normal;
        
        // If we have any texture coordinates, set them
        if ( pFilePoly->TexChannelCount > 0 && pFilePoly->TexCoordSize[0] == 2 )
        {
            pLightGroup->m_pVertex[i + VertexStart].tu = pFilePoly->Vertices[i].TexCoords[0][0];
            pLightGroup->m_pVertex[i + VertexStart].tv = pFilePoly->Vertices[i].TexCoords[0][1];

        } // End if has tex coordinates

        pProperty->m_nVertexCount++;

    } // Next Vertex

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : AddLightGroup() (Private)
// Desc : Adds a light group, or multiple light groups, to this scene.
// Note : Returns the index for the first group added, or -1 on failure.
//-----------------------------------------------------------------------------
long CScene::AddLightGroup( ULONG Count )
{
    CLightGroup ** pGroupBuffer = NULL;
    
    // Allocate new resized array
    if (!( pGroupBuffer = new CLightGroup*[ m_nLightGroupCount + Count ] )) return -1;

    // Clear out slack pointers
    ZeroMemory( &pGroupBuffer[ m_nLightGroupCount ], Count * sizeof( CLightGroup* ) );

    // Existing Data?
    if ( m_ppLightGroupList )
    {
        // Copy old data into new buffer
        memcpy( pGroupBuffer, m_ppLightGroupList, m_nLightGroupCount * sizeof( CLightGroup* ) );

        // Release old buffer
        delete []m_ppLightGroupList;

    } // End if
    
    // Store pointer for new buffer
    m_ppLightGroupList = pGroupBuffer;
    m_nLightGroupCount += Count;

    // Return first LightGroup
    return m_nLightGroupCount - Count;
}


//-----------------------------------------------------------------------------
// Name : GetLightContribution () (Private)
// Desc : Determines how much light the specified light parameters contribute
//        to the mesh passed. This allows us to determine which lights are more
//        important to the speciied mesh than others
// Note : Does not take into account specular because we have no camera pos :)
//-----------------------------------------------------------------------------
float CScene::GetLightContribution( iwfSurface * pPoly, D3DLIGHT9 * pLight )
{
    D3DXVECTOR3 Direction, LightDir = pLight->Direction;
    float       Contribution = 0.0f, MaxContribution = 0.0f;
    D3DXCOLOR   Diffuse, Ambient, Color;
    float       Atten, Spot, Rho, Dot;
    float       Distance;
    ULONG       i;

    // We can only get light contribution of we have a material
    if ( pPoly->ChannelCount == 0 || !(pPoly->Components & SCOMPONENT_MATERIALS )) return 0.0f;
    if ( pPoly->MaterialIndices[0] < 0 ) return 0.0f;
    
    // Retrieve the material for colour calculations
    D3DMATERIAL9 * pMaterial = &m_pMaterialList[ pPoly->MaterialIndices[0] ];

    // Loop through each vertex
    for ( i = 0; i < pPoly->VertexCount; i++ )
    {
        // Retrieve lighting forumla params
        Direction = (D3DXVECTOR3&)pPoly->Vertices[i] - pLight->Position;
        Distance  = D3DXVec3Length( &Direction );

        // Skip if the light is out of range of the vertex (does not apply to directional)
        if ( pLight->Type != D3DLIGHT_DIRECTIONAL && Distance > pLight->Range ) continue;
        
        // Normalize our direction from the vertex to the light
        D3DXVec3Normalize( &Direction, &Direction );

        // Calculate light's attenuation factor.
        Atten = 1.0f;
        if ( pLight->Type != D3DLIGHT_DIRECTIONAL )
        {
            Atten = ( pLight->Attenuation0 + pLight->Attenuation1 * Distance + pLight->Attenuation2 * (Distance * Distance));
            if ( Atten > 0 ) Atten = 1 / Atten; // Avoid divide by zero case
        
        } // End if other types

        // Calculate light's spot factor
        Spot = 1.0f;
        if ( pLight->Type == D3DLIGHT_SPOT )
        {
            // Calculate RHO
            Rho = fabsf(D3DXVec3Dot( &(-LightDir), &Direction ));
            
            if ( Rho > cosf( pLight->Theta / 2.0f ) ) 
                Spot = 1.0f;
            else if ( Rho <= cosf( pLight->Phi  / 2.0f ) ) 
                Spot = 0.0f;
            else
            {
                Spot = ((Rho - cosf( pLight->Phi / 2.0f)) / (cosf( pLight->Theta / 2.0f) - cosf( pLight->Phi / 2.0f))) * pLight->Falloff;

            } // End if calculate spot factor
            
        } // End if Spotlight
         
        // Calculate diffuse contribution for this vertex (Cd*Ld*(N.Ldir)*Atten*Spot)
        Dot = D3DXVec3Dot( (D3DXVECTOR3*)&pPoly->Vertices[i].Normal, &Direction );
        Diffuse.a = 0;
        Diffuse.r = pMaterial->Diffuse.r * pLight->Diffuse.r * Dot * Atten * Spot;
        Diffuse.g = pMaterial->Diffuse.g * pLight->Diffuse.g * Dot * Atten * Spot;
        Diffuse.b = pMaterial->Diffuse.b * pLight->Diffuse.b * Dot * Atten * Spot;

        // Calculate ambient contribution for this vertex (Ca*[Ga + sum(Lai)*Atti*Spoti])
        Ambient.a = 0;
        Ambient.r = pMaterial->Ambient.r * ( /*Global_Ambient.r +*/ pLight->Ambient.r * Atten * Spot );
        Ambient.g = pMaterial->Ambient.g * ( /*Global_Ambient.g +*/ pLight->Ambient.g * Atten * Spot );
        Ambient.b = pMaterial->Ambient.b * ( /*Global_Ambient.b +*/ pLight->Ambient.b * Atten * Spot );
        
        // Calculate light contribution (fabsf() because even dark-lights contribute)
        Color = Ambient + Diffuse;
        Contribution = fabsf(Color.r);
        if ( fabsf(Color.g) > Contribution ) Contribution = fabsf(Color.g);
        if ( fabsf(Color.b) > Contribution ) Contribution = fabsf(Color.b);
        
        // Store the maximum contribution to this surface.
        if ( Contribution > MaxContribution )  MaxContribution = Contribution;
        
    } // Next Vertex

    // Return the total contribution this light gives to this mesh.
    return MaxContribution;
    
}

//-----------------------------------------------------------------------------
// Name : AnimateObjects () (Private)
// Desc : Animates the objects we currently have loaded.
//-----------------------------------------------------------------------------
void CScene::AnimateObjects( CTimer & Timer )
{
    // Shift the texture coordinates along the U axis
    m_mtxTexture._31 += 0.5f * Timer.GetTimeElapsed();
    if ( m_mtxTexture._31 > 1.0f ) m_mtxTexture._31 -= 1.0f;
}

//-----------------------------------------------------------------------------
// Name : Render ()
// Desc : Render the scene
//-----------------------------------------------------------------------------
void CScene::Render( )
{
    ULONG         i, j, k;
    CLightGroup * pLightGroup = NULL;
    ULONG       * pLightList  = NULL;

    // Set up our dynamic lights here if we need to
    //m_pD3DDevice->SetLight( 0, &m_DynamicLight );
    //m_pD3DDevice->LightEnable( 0, TRUE );

    // Loop through each light group
    for ( i = 0; i < m_nLightGroupCount; i++ )
    {
        // Set active lights
        pLightGroup = m_ppLightGroupList[i];
        pLightList  = pLightGroup->m_pLightList;
        for ( j = m_nReservedLights; j < m_nLightLimit; j++ )
        {
            if ( (j - m_nReservedLights) >= (pLightGroup->m_nLightCount ) )
            {
                // Disable any light sources which should not be active
                m_pD3DDevice->LightEnable( j, FALSE );
            
            } // End if no more lights
            else
            {
                // Set this light as active
                m_pD3DDevice->SetLight( j, &m_pLightList[ pLightList[ j - m_nReservedLights ] ] );
                m_pD3DDevice->LightEnable( j, TRUE );

            } // End if set lights

        } // Next Light

        // Set vertex stream
        m_pD3DDevice->SetStreamSource( 0, pLightGroup->m_pVertexBuffer, 0, sizeof(CVertex));

        // Now loop through and render the associated property groups
        for ( j = 0; j < pLightGroup->m_nPropertyGroupCount; ++j )
        {
            CPropertyGroup * pTexProperty = pLightGroup->m_pPropertyGroup[j];
            long TextureIndex = (long)pTexProperty->m_nPropertyData;
            
            // Set Properties
            if ( TextureIndex >= 0 )
            {
                m_pD3DDevice->SetTexture( 0, m_pTextureList[ TextureIndex ] );
            
            } // End if has texture
            else
            {
                m_pD3DDevice->SetTexture( 0, NULL );
            
            } // End if has no texture

            // Set the texture matrix for our animating water example
            if ( TextureIndex == m_nWaterTexture )
            {
                m_pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
                m_pD3DDevice->SetTransform( D3DTS_TEXTURE0, &m_mtxTexture );
            
            } // End if Water Texture
            
            // Render child property group
            for ( k = 0; k < pTexProperty->m_nPropertyGroupCount; ++k )
            {
                CPropertyGroup * pMatProperty = pTexProperty->m_pPropertyGroup[k];
            
                m_pD3DDevice->SetMaterial( &m_pMaterialList[ (long)pMatProperty->m_nPropertyData ] );
                m_pD3DDevice->SetIndices( pMatProperty->m_pIndexBuffer );
                m_pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, pMatProperty->m_nVertexStart , 0, pMatProperty->m_nVertexCount, 0, pMatProperty->m_nIndexCount / 3 );
            
            } // Next Property Group

            // Disable the texture matrix
            if ( TextureIndex == m_nWaterTexture )
            {
                m_pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
            
            } // End if Water Texture
            
        } // Next Property Group
        
    } // Next Light Group

}

//-----------------------------------------------------------------------------
// Name : CLightGroup () (Constructor)
// Desc : CLightGroup Class Constructor
//-----------------------------------------------------------------------------
CLightGroup::CLightGroup()
{
	// Reset / Clear all required values
    m_nPropertyGroupCount = 0;
    m_nVertexCount        = 0;
    m_nLightCount         = 0;
    m_nVertexCapacity     = 0;
    m_pPropertyGroup      = NULL;
    m_pVertex             = NULL;
    m_pLightList          = NULL;
    m_pVertexBuffer       = NULL;
}

//-----------------------------------------------------------------------------
// Name : ~CLightGroup () (Destructor)
// Desc : CLightGroup Class Destructor
//-----------------------------------------------------------------------------
CLightGroup::~CLightGroup()
{
    // Release our group components
    if ( m_pPropertyGroup ) 
    {
        // Delete all individual groups in the array.
        for ( ULONG i = 0; i < m_nPropertyGroupCount; i++ )
        {
            if ( m_pPropertyGroup[i] ) delete m_pPropertyGroup[i];
        
        } // Next Polygon

        // Free up the array itself
        delete []m_pPropertyGroup;
    
    } // End if

    // Release flat arrays
    if ( m_pVertex ) delete []m_pVertex;
    if ( m_pLightList ) delete m_pLightList;

    // Release D3D Objects
    if ( m_pVertexBuffer ) m_pVertexBuffer->Release();

    // Reset / Clear all required values
    m_nPropertyGroupCount = 0;
    m_nVertexCount        = 0;
    m_nLightCount         = 0;
    m_pPropertyGroup      = NULL;
    m_pVertex             = NULL;
    m_pLightList          = NULL;
    m_pVertexBuffer       = NULL;
}


//-----------------------------------------------------------------------------
// Name : SetLights ()
// Desc : Setup and store which lights are to be active for this group.
//-----------------------------------------------------------------------------
bool CLightGroup::SetLights( ULONG LightCount, ULONG LightList[] )
{
    // Release previous set if any
    if ( m_pLightList ) delete m_pLightList;

    // Allocate enough room for these lights
    m_pLightList = new ULONG[LightCount];
    if (!m_pLightList) return false;

    // Store values
    m_nLightCount = LightCount;
    memcpy( m_pLightList, LightList, LightCount * sizeof(ULONG) );

    // Success
    return true;
}

//-----------------------------------------------------------------------------
// Name : GroupMatches ()
// Desc : Determine if the group of lights specified matches this light group
//-----------------------------------------------------------------------------
bool CLightGroup::GroupMatches( ULONG LightCount, ULONG LightList[] ) const
{
    // If length does not match, neither can the list
    if ( m_nLightCount != LightCount ) return false;

    // Compare the light lists (Match even if no lights stored)
    if ( m_nLightCount > 0 )
    {
        //if ( memcmp( m_pLightList, LightList, LightCount * sizeof(ULONG)) != 0 ) return false;
        for ( ULONG i = 0; i < LightCount; i++ )
        {
            for ( ULONG j = 0; j < m_nLightCount; j++ ) if ( LightList[i] == m_pLightList[j] ) break;
            if ( j == m_nLightCount ) return false;
        
        } // Next Light
    
    } // End if any lights

    return true;
} 

//-----------------------------------------------------------------------------
// Name : BuildBuffers()
// Desc : Instructs the light group to build a vertex buffer from the data 
//        currently stored within the group object.
// Note : By passing in true to the 'ReleaseOriginals' parameter, the original
//        buffers will be destroyed.
//-----------------------------------------------------------------------------
bool CLightGroup::BuildBuffers( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL, bool ReleaseOriginals )
{
    HRESULT     hRet    = S_OK;
    CVertex    *pVertex = NULL;
    ULONG       ulUsage = D3DUSAGE_WRITEONLY;

    // Should we use software vertex processing ?
    if ( !HardwareTnL ) ulUsage |= D3DUSAGE_SOFTWAREPROCESSING;

    // Release any previously allocated vertex / index buffers
    if ( m_pVertexBuffer ) m_pVertexBuffer->Release();
    m_pVertexBuffer = NULL;

    // Create our vertex buffer
    hRet = pD3DDevice->CreateVertexBuffer( sizeof(CVertex) * m_nVertexCount, ulUsage, VERTEX_FVF,
                                             D3DPOOL_MANAGED, &m_pVertexBuffer, NULL );
    if ( FAILED( hRet ) ) return false;

    // Lock the vertex buffer ready to fill data
    hRet = m_pVertexBuffer->Lock( 0, sizeof(CVertex) * m_nVertexCount, (void**)&pVertex, 0 );
    if ( FAILED( hRet ) ) return false;

    // Copy over the vertex data
    memcpy( pVertex, m_pVertex, sizeof(CVertex) * m_nVertexCount );

    // We are finished with the vertex buffer
    m_pVertexBuffer->Unlock();

    // Release old data if requested
    if ( ReleaseOriginals )
    {
        // Release our mesh components
        if ( m_pVertex ) delete []m_pVertex;

        // Clear variables
        m_pVertex       = NULL;

    } // End if ReleaseOriginals

    // Build buffers for each child property group
    for ( USHORT i = 0; i < m_nPropertyGroupCount; i++ )
    {
        CPropertyGroup * pProperty = m_pPropertyGroup[i];

        for ( USHORT j = 0; j < pProperty->m_nPropertyGroupCount; j++ )
        {
            if (!pProperty->m_pPropertyGroup[j]->BuildBuffers( pD3DDevice, HardwareTnL, ReleaseOriginals )) return false;

        } // Next Property Group
    
    } // Next Property Group

    // Success
    return true;
}

//-----------------------------------------------------------------------------
// Name : AddVertex()
// Desc : Adds a vertex, or multiple vertices, to this light group.
// Note : Returns the index for the first vertex added, or -1 on failure.
//-----------------------------------------------------------------------------
long CLightGroup::AddVertex( USHORT Count )
{
    CVertex * pVertexBuffer = NULL;
    
    if ( m_nVertexCount + Count > m_nVertexCapacity )
    {
        // Adjust our vertex capacity (resize 100 at a time)
        for ( ; m_nVertexCapacity < (m_nVertexCount + Count) ; ) m_nVertexCapacity += 100;

        // Allocate new resized array
        if (!( pVertexBuffer = new CVertex[ m_nVertexCapacity ] )) return -1;

        // Existing Data?
        if ( m_pVertex )
        {
            // Copy old data into new buffer
            memcpy( pVertexBuffer, m_pVertex, m_nVertexCount * sizeof(CVertex) );

            // Release old buffer
            delete []m_pVertex;

        } // End if

        // Store pointer for new buffer
        m_pVertex = pVertexBuffer;
    
    } // End if a resize is required

    // Increase vertex count
    m_nVertexCount += Count;
    
    // Return first vertex
    return m_nVertexCount - Count;
}

//-----------------------------------------------------------------------------
// Name : AddPropertyGroup()
// Desc : Adds a property group, or multiple groups, to this light group.
// Note : Returns the index for the first group added, or -1 on failure.
//-----------------------------------------------------------------------------
long CLightGroup::AddPropertyGroup( USHORT Count /* = 1  */ )
{

    CPropertyGroup ** pGroupBuffer = NULL;
    
    // Allocate new resized array
    if (!( pGroupBuffer = new CPropertyGroup*[ m_nPropertyGroupCount + Count ] )) return -1;

    // Clear out slack pointers
    ZeroMemory( &pGroupBuffer[ m_nPropertyGroupCount ], Count * sizeof( CPropertyGroup* ) );

    // Existing Data?
    if ( m_pPropertyGroup )
    {
        // Copy old data into new buffer
        memcpy( pGroupBuffer, m_pPropertyGroup, m_nPropertyGroupCount * sizeof( CPropertyGroup* ) );

        // Release old buffer
        delete []m_pPropertyGroup;

    } // End if
    
    // Store pointer for new buffer
    m_pPropertyGroup = pGroupBuffer;

    // Allocate new polygon pointers
    for ( UINT i = 0; i < Count; i++ )
    {
        // Allocate new group
        if (!( m_pPropertyGroup[ m_nPropertyGroupCount ] = new CPropertyGroup() )) return -1;

        // Increase overall group count
        m_nPropertyGroupCount++;

    } // Next group
    
    // Return first group
    return m_nPropertyGroupCount - Count;
}

//-----------------------------------------------------------------------------
// Name : CPropertyGroup () (Constructor)
// Desc : CPropertyGroup Class Constructor
//-----------------------------------------------------------------------------
CPropertyGroup::CPropertyGroup()
{
	// Reset / Clear all required values
    m_PropertyType        = PROPERTY_NONE;
    m_nPropertyData       = 0;
    m_nPropertyGroupCount = 0;
    m_nIndexCount         = 0;
    m_nVertexStart        = 0;
    m_nVertexCount        = 0;
    m_nIndexCapacity      = 0;
    m_pPropertyGroup      = NULL;
    m_pIndex              = NULL;
    m_pIndexBuffer        = NULL;
}

//-----------------------------------------------------------------------------
// Name : ~CPropertyGroup () (Destructor)
// Desc : CPropertyGroup Class Destructor
//-----------------------------------------------------------------------------
CPropertyGroup::~CPropertyGroup()
{
	// Release our mesh components
    if ( m_pPropertyGroup ) 
    {
        // Delete all individual groups in the array.
        for ( ULONG i = 0; i < m_nPropertyGroupCount; i++ )
        {
            if ( m_pPropertyGroup[i] ) delete m_pPropertyGroup[i];
        
        } // Next Polygon

        // Free up the array itself
        delete []m_pPropertyGroup;
    
    } // End if

    // Release flat arrays
    if (m_pIndex) delete []m_pIndex;

    // Release D3D Objects
    if ( m_pIndexBuffer ) m_pIndexBuffer->Release();

    // Clear variables
    m_nPropertyGroupCount = 0;
    m_nIndexCount         = 0;
    m_nVertexStart        = 0;
    m_nVertexCount        = 0;
    m_pPropertyGroup      = NULL;
    m_pIndex              = NULL;
    m_pIndexBuffer        = NULL;
}

//-----------------------------------------------------------------------------
// Name : AddPropertyGroup()
// Desc : Adds a property group, or multiple groups, to this group.
// Note : Returns the index for the first group added, or -1 on failure.
//-----------------------------------------------------------------------------
long CPropertyGroup::AddPropertyGroup( USHORT Count /* = 1  */ )
{

    CPropertyGroup ** pGroupBuffer = NULL;
    
    // Allocate new resized array
    if (!( pGroupBuffer = new CPropertyGroup*[ m_nPropertyGroupCount + Count ] )) return -1;

    // Clear out slack pointers
    ZeroMemory( &pGroupBuffer[ m_nPropertyGroupCount ], Count * sizeof( CPropertyGroup* ) );

    // Existing Data?
    if ( m_pPropertyGroup )
    {
        // Copy old data into new buffer
        memcpy( pGroupBuffer, m_pPropertyGroup, m_nPropertyGroupCount * sizeof( CPropertyGroup* ) );

        // Release old buffer
        delete []m_pPropertyGroup;

    } // End if
    
    // Store pointer for new buffer
    m_pPropertyGroup = pGroupBuffer;

    // Allocate new polygon pointers
    for ( UINT i = 0; i < Count; i++ )
    {
        // Allocate new group
        if (!( m_pPropertyGroup[ m_nPropertyGroupCount ] = new CPropertyGroup() )) return -1;

        // Increase overall group count
        m_nPropertyGroupCount++;

    } // Next group
    
    // Return first group
    return m_nPropertyGroupCount - Count;
}

//-----------------------------------------------------------------------------
// Name : AddIndex()
// Desc : Adds an index, or multiple indices, to this group.
// Note : Returns the index for the first index added, or -1 on failure.
//-----------------------------------------------------------------------------
long CPropertyGroup::AddIndex( USHORT Count )
{

    USHORT * pIndexBuffer = NULL;
    
    if ( m_nIndexCount + Count > m_nIndexCapacity )
    {
        // Adjust our Index capacity (resize 100 at a time)
        for ( ; m_nIndexCapacity < (m_nIndexCount + Count) ; ) m_nIndexCapacity += 100;

        // Allocate new resized array
        if (!( pIndexBuffer = new USHORT[ m_nIndexCapacity ] )) return -1;

        // Existing Data?
        if ( m_pIndex )
        {
            // Copy old data into new buffer
            memcpy( pIndexBuffer, m_pIndex, m_nIndexCount * sizeof(USHORT) );

            // Release old buffer
            delete []m_pIndex;

        } // End if

        // Store pointer for new buffer
        m_pIndex = pIndexBuffer;
    
    } // End if a resize is required

    // Increase Index count
    m_nIndexCount += Count;
    
    // Return first Index
    return m_nIndexCount - Count;
}

//-----------------------------------------------------------------------------
// Name : BuildBuffers()
// Desc : Instructs the property group to build an index buffer from the data 
//        currently stored within the group object.
// Note : By passing in true to the 'ReleaseOriginals' parameter, the original
//        buffers will be destroyed.
//-----------------------------------------------------------------------------
bool CPropertyGroup::BuildBuffers( LPDIRECT3DDEVICE9 pD3DDevice, bool HardwareTnL, bool ReleaseOriginals )
{
    HRESULT     hRet    = S_OK;
    USHORT     *pIndex  = NULL;
    ULONG       ulUsage = D3DUSAGE_WRITEONLY;

    // Should we use software vertex processing ?
    if ( !HardwareTnL ) ulUsage |= D3DUSAGE_SOFTWAREPROCESSING;

    // Release any previously allocated vertex / index buffers
    if ( m_pIndexBuffer ) m_pIndexBuffer->Release();
    m_pIndexBuffer = NULL;

    // Create our index buffer
    hRet = pD3DDevice->CreateIndexBuffer( sizeof(USHORT) * m_nIndexCount, ulUsage, D3DFMT_INDEX16,
                                             D3DPOOL_MANAGED, &m_pIndexBuffer, NULL );
    if ( FAILED( hRet ) ) return false;

    // Lock the index buffer ready to fill data
    hRet = m_pIndexBuffer->Lock( 0, sizeof(USHORT) * m_nIndexCount, (void**)&pIndex, 0 );
    if ( FAILED( hRet ) ) return false;

    // Copy over the index data
    memcpy( pIndex, m_pIndex, sizeof(USHORT) * m_nIndexCount );

    // We are finished with the index buffer
    m_pIndexBuffer->Unlock();

    // Release old data if requested
    if ( ReleaseOriginals )
    {
        // Release our components
        if ( m_pIndex ) delete []m_pIndex;

        // Clear variables
        m_pIndex = NULL;

    } // End if ReleaseOriginals

    // Build buffers for each child property group
    for ( USHORT i = 0; i < m_nPropertyGroupCount; i++ )
    {
        if (!m_pPropertyGroup[i]->BuildBuffers( pD3DDevice, HardwareTnL, ReleaseOriginals )) return false;
    
    } // Next Property Group

    // Success
    return true;
}