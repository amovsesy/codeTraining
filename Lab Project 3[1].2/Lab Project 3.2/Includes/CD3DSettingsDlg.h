//-----------------------------------------------------------------------------
// File: CD3DSettingsDlg.h
//
// Desc: Direct3D device settings dialog. Allows a user to select the various
//       device initialization options.
//
// Copyright (c) 1997-2002 Adam Hoult & Gary Simmons. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef _CD3DSETTINGSDLG_H_
#define _CD3DSETTINGSDLG_H_

//-----------------------------------------------------------------------------
// CD3DSettingsDlg Specific Includes
//-----------------------------------------------------------------------------
#include "CD3DInitialize.h"		// Automatic Application Include
#include <tchar.h>

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CD3DSettingsDlg (Class)
// Desc : Creates and controls a settings dialog for the d3d devices.
//-----------------------------------------------------------------------------
class CD3DSettingsDlg
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
	         CD3DSettingsDlg();
	virtual ~CD3DSettingsDlg();

	//-------------------------------------------------------------------------
	// Public Functions for This Class
	//-------------------------------------------------------------------------
	int				ShowDialog                ( CD3DInitialize * pInitialize, CD3DSettings *pSettings = NULL, HWND hWndParent = NULL );
    CD3DSettings    GetD3DSettings            ( ) const { return m_D3DSettings; }
    
private:
    //-------------------------------------------------------------------------
	// Private Functions for This Class
	//-------------------------------------------------------------------------
	BOOL			SettingsDlgProc           ( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
    void            AdapterChanged            (  );
    void            DeviceChanged             (  );
    void            WindowedFullscreenChanged (  );
    void            AdapterFormatChanged      (  );
    void            ResolutionChanged         (  );
    void            RefreshRateChanged        (  );
    void            BackBufferFormatChanged   (  );
    void            DepthStencilFormatChanged (  );
    void            MultisampleTypeChanged    (  );
    void            MultisampleQualityChanged (  );
    void            VertexProcessingChanged   (  );
    void            PresentIntervalChanged    (  );

    void            Combo_ItemAdd             ( ULONG ComboID, LPVOID pData, LPCTSTR pStrDesc );
    LPVOID          Combo_GetSelectedItemData ( ULONG ComboID );
    ULONG           Combo_GetItemCount        ( ULONG ComboID );
    void            Combo_SelectItem          ( ULONG ComboID, ULONG Index );
    void            Combo_SelectItemData      ( ULONG ComboID, LPVOID pData );
    void            Combo_Clear               ( ULONG ComboID );
    bool            Combo_FindText            ( ULONG ComboID, LPCTSTR pStrText );

    CD3DSettings::Settings * GetSettings ( )  { return m_D3DSettings.GetSettings(); }

	//-------------------------------------------------------------------------
	// Private Static Functions For This Class
	//-------------------------------------------------------------------------
    static BOOL CALLBACK StaticDlgProc        (HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

    //-------------------------------------------------------------------------
	// Private Variables For This Class
	//-------------------------------------------------------------------------
	CD3DInitialize *m_pInitialize;			// The class which stores enum objects
    HWND            m_hWndDlg;              // The main dialog window handle 
    CD3DSettings    m_D3DSettings;          // The settings we are currently using

};

#endif // _CD3DSETTINGSDLG_H_