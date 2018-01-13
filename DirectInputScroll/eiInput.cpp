#include "StdAfx.h"
#include "assert.h"

#include "eiInput.h"



const DIDATAFORMAT& pointerDataFormatAlias = c_dfDIMouse;
const DIDATAFORMAT& pointer2DataFormatAlias = c_dfDIMouse2;
const DIDATAFORMAT& gameCtrlDataFormatAlias = c_dfDIJoystick;
const DIDATAFORMAT& gameCtrl2DataFormatAlias = c_dfDIJoystick2;



HatState LookupHatState(DWORD pov)
{
	switch (pov)
	{
	case 0:
		return HS_NORTH;
	case 4500:
		return HS_NORTHEAST;
	case 9000:
		return HS_EAST;
	case 13500:
		return HS_SOUTHEAST;
	case 18000:
		return HS_SOUTH;
	case 22500:
		return HS_SOUTHWEST;
	case 27000:
		return HS_WEST;
	case 31500:
		return HS_NORTHWEST;
	}
	return HS_CENTERED;
}


eiInputManager* eiInputDevice::inputMgr;
LPDIRECTINPUT8 GetDIinterface()
{
	assert(eiInputDevice::inputMgr);
	return eiInputDevice::inputMgr->directInput;
}

//////////////////////////
/// INPUTDEVICE //////
//////////////////////////

eiInputDevice::eiInputDevice(void* data, DWORD size)
	:	device(0),
		stateData(data),
		oldStateData(new BYTE[size]),
		stateDataSize(size),
		actionMapped(false)
{
}

eiInputDevice::~eiInputDevice()
{
	assert(inputMgr);

	if (device)
	{
		device->Unacquire();

		UINT rCount = device->Release();
		
		char str[100];
		sprintf( str, "device->Release() = %d\n", rCount);
		OutputDebugString(str);

		inputMgr->DelDevice( this );
	}
}

HRESULT eiInputDevice::SetAxisMinMax(long min, long max)
{
	DIPROPRANGE dipdw =
	{
		{
			sizeof(DIPROPRANGE), 
			sizeof(DIPROPHEADER),
			0,
			DIPH_DEVICE,
		},
		min,
		max
	};

	DIVERIFY(device->SetProperty( DIPROP_RANGE, &dipdw.diph ));

	return S_OK;
}

void eiInputDevice::Unacquire()
{
	if (device)
		device->Unacquire();
}

void eiInputDevice::ReleaseDevice()
{
	if (device)
		device->Release(), device = 0;
}

void eiInputDevice::GetDeviceCaps()
{
	assert(device);	// device isn't initialized -- see the Attach member function

	ZeroMemory( &caps, sizeof(caps) );
	caps.dwSize = sizeof(caps);

	DIVERIFY(device->GetCapabilities( &caps ));

	ZeroMemory( &inst, sizeof(inst) );
	inst.dwSize = sizeof(inst);

	DIVERIFY(device->GetDeviceInfo( &inst ));
}

bool eiInputDevice::RefreshState()
{
	assert(device);	// device isn't initialized -- see the Attach member function
	assert(stateData);
	assert(oldStateData);
	assert(stateDataSize > 0);

	memcpy( oldStateData, stateData, stateDataSize );

	if (IsPolledDevice())
		device->Poll();

	HRESULT hr = DIERR_INPUTLOST;
	while( DIERR_INPUTLOST == hr || DIERR_NOTACQUIRED == hr ) 
	{
		hr = device->GetDeviceState( stateDataSize, stateData );

		if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
			device->Acquire();
	}
	
	// returns 'true' if any of the state data has changed
	return memcmp( oldStateData, stateData, stateDataSize ) != 0;
}

void eiInputDevice::SetBufferSize(DWORD bufSize)
{
	assert(device);	// device isn't initialized -- see the Attach member function

	DIPROPDWORD dipdw =
	{
		{
			sizeof(DIPROPDWORD), 
			sizeof(DIPROPHEADER),
			0,
			DIPH_DEVICE,
		},
		bufSize,
	};

	DIVERIFY(device->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ));
}

DWORD eiInputDevice::GetBufferSize()
{
	assert(device);	// device isn't initialized -- see the Attach member function

	DIPROPDWORD dipdw =
	{
		{
			sizeof(DIPROPDWORD), 
			sizeof(DIPROPHEADER),
			0,
			DIPH_DEVICE,
		},
		0,
	};

	DIVERIFY(device->GetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ));

	return dipdw.dwData;
}


DWORD eiInputDevice::GetEventCount()
{
	assert(device);	// device isn't initialized -- see the Attach member function

	if (GetBufferSize() == 0)
		return 0;

	DWORD count = GetBufferSize();

	HRESULT hr = DIERR_INPUTLOST;
	while ( DIERR_INPUTLOST == hr || DIERR_NOTACQUIRED == hr)
	{
		hr = device->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), 0, &count, DIGDD_PEEK );
		if (hr == DIERR_INPUTLOST || DIERR_NOTACQUIRED == hr)
			device->Acquire();
	}

	return count;
}

bool eiInputDevice::PeekEvent(DIDEVICEOBJECTDATA& data)
{
	assert(device);	// device isn't initialized -- see the Attach member function
	assert(GetBufferSize() > 0);  // no event buffer size was specified during creation

	DWORD count = 1;

	HRESULT hr = DIERR_INPUTLOST;
	while ( hr == DIERR_INPUTLOST || DIERR_NOTACQUIRED == hr )
	{
		hr = device->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), &data, &count, DIGDD_PEEK );
		if( hr == DIERR_INPUTLOST || DIERR_NOTACQUIRED == hr)
			device->Acquire();
	}

	return count > 0;
}

bool eiInputDevice::GetEvent(DIDEVICEOBJECTDATA& data)
{
	assert(device);	// device isn't initialized -- see the Attach member function

	if (GetBufferSize() == 0) 
		return false; // there won't be any events waiting, as there is no place to put them

	DWORD count = 1;

	HRESULT hr = DIERR_INPUTLOST;
	while ( hr == DIERR_INPUTLOST || DIERR_NOTACQUIRED == hr )
	{
		hr = device->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), &data, &count, 0 );
		if ( hr == DIERR_INPUTLOST || DIERR_NOTACQUIRED == hr )
			device->Acquire();
	}

	return count > 0;
}

void eiInputDevice::FlushEvents()
{
	assert(device);	// device isn't initialized -- see the Attach member function

	if (GetBufferSize() == 0) 
		return; // no events to flush, as there is no place to put them anyway

	DWORD count = GetEventCount();

	HRESULT hr = DIERR_INPUTLOST;
	while ( hr == DIERR_INPUTLOST || DIERR_NOTACQUIRED == hr )
	{
		hr = device->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), 0, &count, 0 );
		if ( hr == DIERR_INPUTLOST || DIERR_NOTACQUIRED == hr )
			device->Acquire();
	}
}

bool eiInputDevice::DeliverEvents()
{
	assert(device);	// device isn't attached -- see the device Attach() member functions

	DIDEVICEOBJECTDATA d;
	while (GetEvent( d ))
	{
		if (d.uAppData == -1)
			DispatchEvent( d );
		else
			MappedEvent( d );
	}

	return true;
}

bool eiInputDevice::EnumObjects(DWORD enumFlags)
{
	assert(device);

	objectList.clear();

	HRESULT r = device->EnumObjects( EnumObjectsCallback, this, enumFlags );

	if (SUCCEEDED(r))
		return true;

	return false;
}

BOOL CALLBACK eiInputDevice::EnumObjectsCallback(LPCDIDEVICEOBJECTINSTANCE object, void* thisPtr)
{
	eiInputDevice* thisObj = static_cast<eiInputDevice*>(thisPtr);

	if (object)
		thisObj->objectList.push_back( *object );

	return TRUE;
}


bool eiInputDevice::UpdateActionMap(DIACTIONFORMAT* format)
{
	device->Unacquire();

	if (device->BuildActionMap( format, 0, DIDBAM_DEFAULT ) != DI_OK)
		return false;

	InspectMappings( format );

	device->SetActionMap( format, 0, DIDSAM_DEFAULT );

	return true;
}

bool eiInputDevice::IsAcquired()
{
	assert(device);

	HRESULT hr = device->Acquire();

	if (hr == DI_OK || hr == S_FALSE)
		return true;

	return false;
}


eiInputObjectIterator eiInputDevice::GetObjectIterator()
{
	return eiInputObjectIterator( objectList );
}


////////////////////
/// KEYBOARD ///////
////////////////////


eiKeyboard::eiKeyboard()
	:	eiInputDevice(keyArray, sizeof(keyArray))
{
}

HRESULT eiKeyboard::Attach(const GUID* pGuid, DWORD bufSize, DIACTIONFORMAT* format)
{
	assert(inputMgr);  // you must create eiInputManager before attaching devices
	LPDIRECTINPUT8 directInput = inputMgr->GetDirectInput();
	assert(directInput);  

	const GUID* id = pGuid;

	if (id == 0)
		id = &GUID_SysKeyboard;

	inputMgr->AddDevice( this );

	HRESULT r = S_OK;

	DIVERIFY(directInput->CreateDevice( *id, &device, 0 ));

	GetDeviceCaps();

	device->SetDataFormat( &c_dfDIKeyboard );

	if (format)
	{
		inputMgr->actionFormat = format;

		r = device->BuildActionMap( format, 0/*m_strUserName*/, 0L );
		SetActionMapping( true );

		InspectMappings( format );

	    DIVERIFY(device->SetActionMap( format, 0/*m_strUserName*/, DIDSAM_DEFAULT ));
	}
	else if (bufSize > 0)
		SetBufferSize( bufSize );

    DIVERIFY(device->SetCooperativeLevel( inputMgr->GetHwnd(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND ));

	return r;
}

BYTE eiKeyboard::GetKeyState(int key)
{
	assert(device);	// device isn't attached -- see eiPointer::Attach()

	assert(key>=0);
	assert(key<256);

	return keyArray[key];
}

bool eiKeyboard::KeyIsDown(int key)
{
	assert(device);	// device isn't attached -- see eiKeyboard::Attach()

	assert(key>=0);
	assert(key<256);

	return (keyArray[key] & 0x80) != 0;
}

void eiKeyboard::DispatchEvent(const DIDEVICEOBJECTDATA& rawEvent)
{
	assert(device);	// device isn't attached -- see eiKeyboard::Attach()
	
	if (rawEvent.dwData & 0x80)
		KeyDown( rawEvent.dwOfs );
	else
		KeyUp( rawEvent.dwOfs );
}


////////////////////
/// MOUSE    ///////
////////////////////

eiPointer::eiPointer()
	:	eiInputDevice(&pointerState, sizeof(pointerState))
{
}

HRESULT eiPointer::Attach(const GUID* pGuid, DWORD bufSize, DIACTIONFORMAT* format)
{
	assert(inputMgr);  // you must create eiInputManager before attaching devices
	LPDIRECTINPUT8 directInput = inputMgr->GetDirectInput();
	assert(directInput);  

	const GUID* id = pGuid;

	if (id == 0)
		id = &GUID_SysMouse;

	inputMgr->AddDevice( this );

	DIVERIFY(directInput->CreateDevice( *id, &device, 0 ));

	GetDeviceCaps();

	device->SetDataFormat( &pointer2DataFormatAlias );

	HRESULT r = S_OK;

	if (format)
	{
		inputMgr->actionFormat = format;

		r = device->BuildActionMap( format, 0/*m_strUserName*/, 0L );
		SetActionMapping( true );

		InspectMappings( format );

		DIVERIFY(device->SetActionMap( format, 0/*m_strUserName*/, DIDSAM_DEFAULT  ));
	}
	else if (bufSize > 0)
		SetBufferSize( bufSize );
	
	DIVERIFY(device->SetCooperativeLevel( inputMgr->GetHwnd(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND ));

	return r;
}

BYTE eiPointer::GetButtonState(int button)
{
	assert(device);	// device isn't attached -- see eiPointer::Attach()

	assert(button>=0);
	assert(button < sizeof(pointerState.rgbButtons));

	return pointerState.rgbButtons[button];
}

bool eiPointer::ButtonIsDown(int button)
{
	assert(device);  // device isn't attached -- see eiPointer::Attach()

	assert(button>=0);
	assert(button < sizeof(pointerState.rgbButtons));

	return (pointerState.rgbButtons[button] & 0x80) != 0;
}

void eiPointer::DispatchEvent(const DIDEVICEOBJECTDATA& event)
{
	switch (event.dwOfs)
	{
	case DIMOFS_X:
		AxisX( event.dwData );
		break;
	case DIMOFS_Y:
		AxisY( event.dwData );
		break;
	case DIMOFS_Z:
		AxisZ( event.dwData );
		break;
	default: // not an axis--must be a button
		if (event.dwData & 0x80)
			ButtonDown( event.dwOfs - DIMOFS_BUTTON0 );
		else
			ButtonUp( event.dwOfs - DIMOFS_BUTTON0 );
	}
}


///////////////////////
/// GAME CONTROLLER ///
///////////////////////

eiGameController::eiGameController()
	:	eiInputDevice(&controllerState, sizeof(controllerState))
{
}

HRESULT eiGameController::Attach(const GUID* pGuid, DWORD bufSize, DIACTIONFORMAT* format)
{
	assert(inputMgr);  // you must create eiInputManager before attaching devices
	LPDIRECTINPUT8 directInput = inputMgr->GetDirectInput();
	assert(directInput);  


	HRESULT r = S_OK;

	DIVERIFY(directInput->CreateDevice( *pGuid, &device, 0 ));

	GetDeviceCaps();

	if (IsPolledDevice() && bufSize > 0)
			return DIERR_NOTBUFFERED;

	device->SetDataFormat( &gameCtrlDataFormatAlias );

	if (format)
	{
		inputMgr->actionFormat = format;

		r = device->BuildActionMap( format, 0/*m_strUserName*/, 0L );
		SetActionMapping( true );

		InspectMappings( format );

		DIVERIFY(device->SetActionMap( format, 0/*m_strUserName*/, DIDSAM_DEFAULT ));
	}
	else if (bufSize > 0)
		SetBufferSize( bufSize );

	DIVERIFY(device->SetCooperativeLevel( inputMgr->GetHwnd(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND ));

	inputMgr->AddDevice( this );

	return r;
}

BYTE eiGameController::GetButtonState(int button)
{
	assert(device);	// device isn't attached -- see eiGameController::Attach()

	assert(button>=0);
	assert(button < sizeof(controllerState.rgbButtons));

	return controllerState.rgbButtons[button];
}

bool eiGameController::ButtonIsDown(int button)
{
	assert(device);	// device isn't attached -- see eiGameController::Attach()

	assert(button >= 0);
	assert(button < sizeof(controllerState.rgbButtons));

	return (controllerState.rgbButtons[button] & 0x80) != 0;
}

DWORD eiGameController::GetHatSwitchState(int hat)
{
	assert(device);	// device isn't attached -- see eiGameController::Attach()

	assert(hat>=0);
	assert(hat < sizeof(controllerState.rgdwPOV));

	return controllerState.rgdwPOV[hat];
}


HatState eiGameController::GetHatState(int hat)
{
	assert(device);	// device isn't attached -- see eiGameController::Attach()

	assert(hat >= 0);
	assert(hat < sizeof(controllerState.rgdwPOV)/sizeof(controllerState.rgdwPOV[0]));

	return LookupHatState( controllerState.rgdwPOV[hat] );
}

bool eiGameController::DeliverEvents()
{
	assert(device);

	// Ideal case:
	// -----------
	// if this is NOT a polled device, then we can use the
	// base class DeliverEvents() to retrieve the events
	// and pass them to eiGameController::DispatchEvents()

	if ( ! (GetCapsFlags() & DIDC_POLLEDDEVICE))
		return eiInputDevice::DeliverEvents();

	// If the app is trying to do action mapping, forget it. This is a polled
	// device, and we're resorting to emulation, but only for non-mapped input.
	// (Emulating in this case would require using DI's device mapping data)

	if (IsActionMapped())
	{
#ifdef _DEBUG
		char str[200];
		sprintf( str, "'%s' is a polled device -- cannot generate actions\n", GetProductName() );
		OutputDebugString( str );
#endif
		return false;
	}

	// if it IS a polled device we'll emulate events by conparing the 
	// current and previous device states and dispatching the any changes

	device->Poll();
	RefreshState();

	const GAMECTRLSTATE* old = static_cast<const GAMECTRLSTATE*>(GetPrevStateBuffer());
	
	if (old->lX != controllerState.lX)
		AxisX( controllerState.lX );
	if (old->lY != controllerState.lY)
		AxisY( controllerState.lY );
	if (old->lZ != controllerState.lZ)
		AxisZ( controllerState.lZ );

	if (old->lRx != controllerState.lRx)
		AxisRX( controllerState.lRx );
	if (old->lRy != controllerState.lRy)
		AxisRY( controllerState.lRy );
	if (old->lRz != controllerState.lRz)
		AxisRZ( controllerState.lRz );

	if (old->rglSlider[0] != controllerState.rglSlider[0])
		Slider( 0, controllerState.rglSlider[0] );
	if (old->rglSlider[1] != controllerState.rglSlider[1])
		Slider( 1, controllerState.rglSlider[1] );

	if (old->rgdwPOV[0] != controllerState.rgdwPOV[0])
		Hat( 0, LookupHatState( controllerState.rgdwPOV[0] ) );
	if (old->rgdwPOV[1] != controllerState.rgdwPOV[1])
		Hat( 1, LookupHatState( controllerState.rgdwPOV[1] ) );
	if (old->rgdwPOV[2] != controllerState.rgdwPOV[2])
		Hat( 2, LookupHatState( controllerState.rgdwPOV[2] ) );
	if (old->rgdwPOV[3] != controllerState.rgdwPOV[3])
		Hat( 3, LookupHatState( controllerState.rgdwPOV[3] ) );

	int numButtons = GetNumButtons();

	for (int b=0; b<numButtons; b++)
	{
		if (old->rgbButtons[b] != controllerState.rgbButtons[b])
		{
			if (controllerState.rgbButtons[b] & 0x80)
				ButtonDown( b );
			else 
				ButtonUp( b );
		}
	}

	return true;
}

void eiGameController::DispatchEvent(const DIDEVICEOBJECTDATA& event)
{
	switch (event.dwOfs)
	{
	case DIJOFS_X:
		AxisX( event.dwData );
		break;
	case DIJOFS_Y:
		AxisY( event.dwData );
		break;
	case DIJOFS_Z:
		AxisZ( event.dwData );
		break;
	case DIJOFS_RX:
		AxisRX( event.dwData );
		break;
	case DIJOFS_RY:
		AxisRY( event.dwData );
		break;
	case DIJOFS_RZ:
		AxisRZ( event.dwData );
		break;
	case DIJOFS_SLIDER(0):
		Slider( 0, event.dwData );
		break;
	case DIJOFS_SLIDER(1):
		Slider( 1, event.dwData );
		break;
	case DIJOFS_POV(0):
		Hat( 0, LookupHatState( event.dwData ) );
		break;
	case DIJOFS_POV(1):
		Hat( 1, LookupHatState( event.dwData ) );
		break;
	case DIJOFS_POV(2):
		Hat( 2, LookupHatState( event.dwData ) );
		break;
	case DIJOFS_POV(3):
		Hat( 3, LookupHatState( event.dwData ) );
		break;
	default:
		if (event.dwData & 0x80)
			ButtonDown( event.dwOfs - DIJOFS_BUTTON0 );
		else
			ButtonUp( event.dwOfs - DIJOFS_BUTTON0 );
	}
}


////////////////////////
/// INPUTMANAGER ///////
////////////////////////

eiInputManager::eiInputManager(HWND h)
	:	directInput(0),
		hwnd(h),
		actionFormat(0),
		cleanUp(false)
{
	eiInputDevice::inputMgr = this;

	DirectInput8Create( (HINSTANCE)GetWindowLong( hwnd, GWL_HINSTANCE ), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, 0 );
}

eiInputManager::~eiInputManager()
{
	Terminate();
}

void eiInputManager::Terminate()
{
	char str[200];

	int activeDeviceCount = activeDeviceList.size();

	if (activeDeviceCount > 0)
	{
		cleanUp = true;

		sprintf( str, "~eiInputManger: %d devices still active - deleting\n", activeDeviceCount);
		OutputDebugString( str );

		for (ActiveDeviceList::iterator it = activeDeviceList.begin(); it != activeDeviceList.end(); ++it)
		{	
			delete *it;
		}

		activeDeviceList.clear();
	}


	if (directInput)
	{
		ULONG rc = directInput->Release(), directInput = 0;
		sprintf( str, "di->Release() = %d\n", rc );		
		OutputDebugString( str );
	}

	OutputDebugString("** DI released **\n");
}

BOOL CALLBACK eiInputManager::EnumDeviceCallback(const DIDEVICEINSTANCE* deviceDesc, void* p)
{
	eiInputManager* mgr = static_cast<eiInputManager*>( p );

	LPDIRECTINPUTDEVICE8 device;

	HRESULT r = mgr->directInput->CreateDevice( deviceDesc->guidInstance, &device, 0 );
	if (FAILED(r))
		return DIENUM_CONTINUE;

	assert(device->Release() == 0);

	mgr->deviceDescList.push_back( *deviceDesc );
	
	return DIENUM_CONTINUE;
}

void eiInputManager::EnumDevices(DWORD devType, DWORD enumFlags)
{
	deviceDescList.clear();

	DIVERIFY(directInput->EnumDevices( devType, EnumDeviceCallback, this, enumFlags ));
}

BOOL CALLBACK eiInputManager::EnumDeviceMappingCallback(const DIDEVICEINSTANCE* deviceDesc, LPDIRECTINPUTDEVICE8 dev, DWORD flags, DWORD remaining, void* p)
{
	eiInputManager* mgr = static_cast<eiInputManager*>( p );
	
	mgr->deviceDescList.push_back( *deviceDesc );

	return DIENUM_CONTINUE;
}

void eiInputManager::SetActionMap(DIACTIONFORMAT* format)
{
	UnacquireDevices();

	ActiveDeviceList::iterator it;

	for (it = activeDeviceList.begin(); it != activeDeviceList.end(); ++it)
	{
		eiInputDevice* dev = (*it);

		assert (dev->device);

		dev->device->BuildActionMap( format, 0/*m_strUserName*/, 0L );

		(*it)->device->SetActionMap( format, 0/*m_strUserName*/, DIDSAM_DEFAULT );

	}

}

BOOL CALLBACK eiInputManager::EnumDeviceReAttachCallback(const DIDEVICEINSTANCE* deviceDesc, LPDIRECTINPUTDEVICE8 dev, DWORD flags, DWORD remaining, void* p)
{
	eiInputManager* mgr = static_cast<eiInputManager*>( p );

	// re-create the device, without the user knowing about it
	ActiveDeviceList::iterator it;
	for (it = mgr->activeDeviceList.begin(); it != mgr->activeDeviceList.end(); ++it)
	{
		if ((*it)->device == 0)
		{
			if (memcmp( const_cast<GUID*>(&deviceDesc->guidInstance), (*it)->GetInstanceGuid(), sizeof(GUID) ) == 0)
			{
				OutputDebugString("reattaching device\n");
				(*it)->device = dev;
				dev->AddRef();
			}
		}
	}
	
	return DIENUM_CONTINUE;
}



void eiInputManager::EnumDevicesWithMapping(const char* userName, LPDIACTIONFORMAT format, DWORD enumFlags)
{
	deviceDescList.clear();

	DIVERIFY(directInput->EnumDevicesBySemantics( 0/*userName*/, format, EnumDeviceMappingCallback, this, enumFlags ));
}


eiInputDeviceIterator eiInputManager::GetDeviceIterator()
{
	return eiInputDeviceIterator( deviceDescList );
}

void eiInputManager::DeliverEvents()
{
    for (ActiveDeviceList::iterator it = activeDeviceList.begin(); it != activeDeviceList.end(); ++it)
		(*it)->DeliverEvents();
}

void eiInputManager::AddDevice(eiInputDevice* device)
{
	assert(device);

#ifdef _DEBUG
	for (ActiveDeviceList::iterator it = activeDeviceList.begin(); it != activeDeviceList.end(); ++it)
		assert(device != *it);
#endif

	activeDeviceList.push_back( device );
}

void eiInputManager::DelDevice(eiInputDevice* device)
{
	if (cleanUp)  // short-curcuit the usual active list stuff
		return;

	for (ActiveDeviceList::iterator it = activeDeviceList.begin(); it != activeDeviceList.end(); ++it)
	{
		if (device == *it)
		{
			activeDeviceList.erase( it );
			return;
		}
	}

	assert(false); // couldn't find device in active list
}

HRESULT eiInputManager::ShowConfig(DIACTION* array, int count, DIACTIONFORMAT* actionFormat, DWORD flags)
{
	DICONFIGUREDEVICESPARAMS dicdp;
	ZeroMemory(&dicdp, sizeof(dicdp));

	DICOLORSET& dics = dicdp.dics;
	dics.dwSize = sizeof(DICOLORSET);
	dics.cTextFore = D3DCOLOR_ARGB( 0, 20, 20, 255 );
	dics.cTextHighlight = D3DCOLOR_ARGB( 0, 255, 255, 255 );
	dics.cCalloutLine = D3DCOLOR_ARGB( 0, 0, 0, 128 );
	dics.cCalloutHighlight = D3DCOLOR_ARGB( 0, 0, 0, 255 );
	dics.cBorder = D3DCOLOR_ARGB( 0, 255, 0, 06 );
	dics.cControlFill = D3DCOLOR_ARGB( 0, 50, 50, 50 );
	dics.cHighlightFill = D3DCOLOR_ARGB( 0, 100, 100, 100 );
	dics.cAreaFill = D3DCOLOR_ARGB( 0, 0, 0, 0 );

	// Fill in all the params
	dicdp.dwSize = sizeof(dicdp);
	dicdp.dwcUsers       = 1;
	dicdp.lptszUserNames = 0;

	dicdp.dwcFormats     = 1;
	dicdp.lprgFormats    = actionFormat;
	dicdp.hwnd           = hwnd;
	dicdp.lpUnkDDSTarget = 0;


	UnacquireDevices();

	HRESULT hr = directInput->ConfigureDevices( 0, &dicdp, flags, 0 );
	
	return hr;
}

void eiInputManager::UnacquireDevices()
{
	for (ActiveDeviceList::iterator it = activeDeviceList.begin(); it != activeDeviceList.end(); ++it)
	{
		(*it)->Unacquire();
	}
}

const char* GetDeviceTypeString(DWORD type)
{
	switch( type )
	{
	case DI8DEVTYPE_DEVICE:
		return "DI8DEVTYPE_DEVICE";
	case DI8DEVTYPE_MOUSE:
		return "DI8DEVTYPE_MOUSE";
	case DI8DEVTYPE_KEYBOARD:
		return "DI8DEVTYPE_KEYBOARD";
	case DI8DEVTYPE_JOYSTICK:
		return "DI8DEVTYPE_JOYSTICK";
	case DI8DEVTYPE_GAMEPAD:
		return "DI8DEVTYPE_GAMEPAD";
	case DI8DEVTYPE_DRIVING:
		return "DI8DEVTYPE_DRIVING";
	case DI8DEVTYPE_FLIGHT:
		return "DI8DEVTYPE_FLIGHT";
	case DI8DEVTYPE_1STPERSON:
		return "DI8DEVTYPE_1STPERSON";
	case DI8DEVTYPE_DEVICECTRL:
		return "DI8DEVTYPE_DEVICECTRL";
	case DI8DEVTYPE_SCREENPOINTER:
		return "DI8DEVTYPE_SCREENPOINTER";
	case DI8DEVTYPE_REMOTE:
		return "DI8DEVTYPE_REMOTE";
	case DI8DEVTYPE_SUPPLEMENTAL:
		return "DI8DEVTYPE_SUPPLEMENTAL";
	default:
		return "(unknown)";
	};
	return 0;
}

int GetDeviceFlagStrings(DWORD flags, char** str)
{
	int cur = 0;

	if (DIDC_ATTACHED & flags)
		str[cur++] = "DIDC_ATTACHED";
	if (DIDC_POLLEDDEVICE & flags)
		str[cur++] = "DIDC_POLLEDDEVICE";
	if (DIDC_EMULATED & flags)
		str[cur++] = "DIDC_EMULATED";
	if (DIDC_POLLEDDATAFORMAT & flags)
		str[cur++] = "DIDC_POLLEDDATAFORMAT";
#if(DIRECTINPUT_VERSION >= 0x0500)
	if (DIDC_FORCEFEEDBACK & flags)
		str[cur++] = "DIDC_FORCEFEEDBACK";
	if (DIDC_FFATTACK & flags)
		str[cur++] = "DIDC_FFATTACK";
	if (DIDC_FFFADE & flags)
		str[cur++] = "DIDC_FFFADE";
	if (DIDC_SATURATION & flags)
		str[cur++] = "DIDC_SATURATION";
	if (DIDC_POSNEGCOEFFICIENTS & flags)
		str[cur++] = "DIDC_POSNEGCOEFFICIENTS";
	if (DIDC_POSNEGSATURATION & flags)
		str[cur++] = "DIDC_POSNEGSATURATION";
	if (DIDC_DEADBAND & flags)
		str[cur++] = "DIDC_DEADBAND";
#endif
	if (DIDC_STARTDELAY & flags)
		str[cur++] = "DIDC_STARTDELAY";
#if(DIRECTINPUT_VERSION >= 0x050a)
	if (DIDC_ALIAS & flags)
		str[cur++] = "DIDC_ALIAS";
	if (DIDC_PHANTOM & flags)
		str[cur++] = "DIDC_PHANTOM";
#endif
#if(DIRECTINPUT_VERSION >= 0x0701)
	if (DIDC_HIDDEN & flags)
		str[cur++] = "DIDC_HIDDEN";
#endif

	str[cur] = 0;

	return cur;
}

struct DikAttributes
{
	bool valid;
	bool alphabetic;
	struct
	{
		bool numeric;
		bool punctuation;
	} shift[2];
	bool prog;
	char* text;
};


static DikAttributes dikAttrib[] = 
{  
	// valid, alph,  num,  puct, string

	{ false, false, { false, false, false, false}, false, "DIK_INVALID" },     // ???						0x00
	{ true,  false, { false, false, false, false}, false, "DIK_ESCAPE" },		//define DIK_ESCAPE          0x01
	{ true,  false, { true,  false, false, true }, true, "DIK_1" },		//define DIK_1               0x02
	{ true,  false, { true,  false, false, true }, true, "DIK_2" },		//define DIK_2               0x03
	{ true,  false, { true,  false, false, true }, true, "DIK_3" },		//define DIK_3               0x04
	{ true,  false, { true,  false, false, true }, true, "DIK_4" },		//define DIK_4               0x05
	{ true,  false, { true,  false, false, true }, true, "DIK_5" },		//define DIK_5               0x06
	{ true,  false, { true,  false, false, true }, true, "DIK_6" },		//define DIK_6               0x07
	{ true,  false, { true,  false, false, true }, true, "DIK_7" },		//define DIK_7               0x08
	{ true,  false, { true,  false, false, true }, true, "DIK_8" },		//define DIK_8               0x09
	{ true,  false, { true,  false, false, true }, true, "DIK_9" },		//define DIK_9               0x0A
	{ true,  false, { true,  false, false, true }, true, "DIK_0" },		//define DIK_0               0x0B
	{ true,  false, { false, true,  false, true }, true, "DIK_MINUS" },		//define DIK_MINUS           0x0C    /* - on main keyboard */
	{ true,  false, { false, true,  false, true }, true, "DIK_EQUALS" },		//define DIK_EQUALS          0x0D
	{ true,  false, { false, false, false, false}, true, "DIK_BACK" },		//define DIK_BACK            0x0E    /* backspace */
	{ true,  false, { false, false, false, false}, true, "DIK_TAB" },		//define DIK_TAB             0x0F
	{ true,  true,  { false, false, false, false}, true, "DIK_Q" },		//define DIK_Q               0x10
	{ true,  true,  { false, false, false, false}, true, "DIK_W" },		//define DIK_W               0x11
	{ true,  true,  { false, false, false, false}, true, "DIK_E" },		//define DIK_E               0x12
	{ true,  true,  { false, false, false, false}, true, "DIK_R" },		//define DIK_R               0x13
	{ true,  true,  { false, false, false, false}, true, "DIK_T" },		//define DIK_T               0x14
	{ true,  true,  { false, false, false, false}, true, "DIK_Y" },		//define DIK_Y               0x15
	{ true,  true,  { false, false, false, false}, true, "DIK_U" },		//define DIK_U               0x16
	{ true,  true,  { false, false, false, false}, true, "DIK_I" },		//define DIK_I               0x17
	{ true,  true,  { false, false, false, false}, true, "DIK_O" },		//define DIK_O               0x18
	{ true,  true,  { false, false, false, false}, true, "DIK_P" },		//define DIK_P               0x19
	{ true,  false, { false, true,  false, false}, true, "DIK_LBRACKET" },		//define DIK_LBRACKET        0x1A
	{ true,  false, { false, true,  false, false}, true, "DIK_RBRACKET" },		//define DIK_RBRACKET        0x1B
	{ true,  false, { false, false, false, false}, true, "DIK_RETURN" },		//define DIK_RETURN          0x1C    /* Enter on main keyboard */
	{ true,  false, { false, false, false, false}, true, "DIK_LCONTROL" },		//define DIK_LCONTROL        0x1D
	{ true,  true,  { false, false, false, false}, true, "DIK_A" },		//define DIK_A               0x1E
	{ true,  true,  { false, false, false, false}, true, "DIK_S" },		//define DIK_S               0x1F
	{ true,  true,  { false, false, false, false}, true, "DIK_D" },		//define DIK_D               0x20
	{ true,  true,  { false, false, false, false}, true, "DIK_F" },		//define DIK_F               0x21
	{ true,  true,  { false, false, false, false}, true, "DIK_G" },		//define DIK_G               0x22
	{ true,  true,  { false, false, false, false}, true, "DIK_H" },		//define DIK_H               0x23
	{ true,  true,  { false, false, false, false}, true, "DIK_J" },		//define DIK_J               0x24
	{ true,  true,  { false, false, false, false}, true, "DIK_K" },		//define DIK_K               0x25
	{ true,  true,  { false, false, false, false}, true, "DIK_L" },		//define DIK_L               0x26
	{ true,  false, { false, false, false, false} , true, "DIK_SEMICOLON" },		//define DIK_SEMICOLON       0x27
	{ true,  false, { false, false, false, false} , true, "DIK_APOSTROPHE" },		//define DIK_APOSTROPHE      0x28
	{ true,  false, { false, false, false, false}, true, "DIK_GRAVE" },		//define DIK_GRAVE           0x29    /* accent grave */
	{ true,  false, { false, false, false, false}, true, "DIK_LSHIFT" },		//define DIK_LSHIFT          0x2A
	{ true,  false, { false, false, false, false} , true, "DIK_BACKSLASH" },		//define DIK_BACKSLASH       0x2B
	{ true,  true,  { false, false, false, false}, true, "DIK_Z" },		//define DIK_Z               0x2C
	{ true,  true,  { false, false, false, false}, true, "DIK_X" },		//define DIK_X               0x2D
	{ true,  true,  { false, false, false, false}, true, "DIK_C" },		//define DIK_C               0x2E
	{ true,  true,  { false, false, false, false}, true, "DIK_V" },		//define DIK_V               0x2F
	{ true,  true,  { false, false, false, false}, true, "DIK_B" },		//define DIK_B               0x30
	{ true,  true,  { false, false, false, false}, true, "DIK_N" },		//define DIK_N               0x31
	{ true,  true,  { false, false, false, false}, true, "DIK_M" },		//define DIK_M               0x32
	{ true,  false, { false, true,  false, true }, true, "DIK_COMMA" },		//define DIK_COMMA           0x33
	{ true,  false, { false, true,  false, true }, true, "DIK_PERIOD" },		//define DIK_PERIOD          0x34    /* . on main keyboard */
	{ true,  false, { false, true,  false, true }, true, "DIK_SLASH" },		//define DIK_SLASH           0x35    /* / on main keyboard */
	{ true,  false, { false, false, false, false}, true, "DIK_RSHIFT" },		//define DIK_RSHIFT          0x36
	{ true,  false, { false, false, false, false}, true, "DIK_MULTIPLY" },		//define DIK_MULTIPLY        0x37    /* * on numeric keypad */
	{ true,  false, { false, false, false, false}, true, "DIK_LMENU" },		//define DIK_LMENU           0x38    /* left Alt */
	{ true,  false, { false, false, false, false}, true, "DIK_SPACE" },		//define DIK_SPACE           0x39
	{ true,  false, { false, false, false, false}, false, "DIK_CAPITAL" },		//define DIK_CAPITAL         0x3A
	{ true,  false, { false, false, false, false}, false, "DIK_F1" },		//define DIK_F1              0x3B
	{ true,  false, { false, false, false, false}, false, "DIK_F2" },		//define DIK_F2              0x3C
	{ true,  false, { false, false, false, false}, false, "DIK_F3" },		//define DIK_F3              0x3D
	{ true,  false, { false, false, false, false}, false, "DIK_F4" },		//define DIK_F4              0x3E
	{ true,  false, { false, false, false, false}, false, "DIK_F5" },		//define DIK_F5              0x3F
	{ true,  false, { false, false, false, false}, false, "DIK_F6" },		//define DIK_F6              0x40
	{ true,  false, { false, false, false, false}, false, "DIK_F7" },		//define DIK_F7              0x41
	{ true,  false, { false, false, false, false}, false, "DIK_F8" },		//define DIK_F8              0x42
	{ true,  false, { false, false, false, false}, false, "DIK_F9" },		//define DIK_F9              0x43
	{ true,  false, { false, false, false, false}, false, "DIK_F10" },		//define DIK_F10             0x44
	{ true,  false, { false, false, false, false}, false, "DIK_NUMLOCK" },		//define DIK_NUMLOCK         0x45
	{ true,  false, { false, false, false, false}, false, "DIK_SCROLL" },		//define DIK_SCROLL          0x46    /* Scroll Lock */
	{ true,  false, { false, false, false, false}, true, "DIK_NUMPAD7" },		//define DIK_NUMPAD7         0x47
	{ true,  false, { false, false, false, false}, true, "DIK_NUMPAD8" },		//define DIK_NUMPAD8         0x48
	{ true,  false, { false, false, false, false}, true, "DIK_NUMPAD9" },		//define DIK_NUMPAD9         0x49
	{ true,  false, { false, false, false, false}, true, "DIK_SUBTRACT" },		//define DIK_SUBTRACT        0x4A    /* - on numeric keypad */
	{ true,  false, { false, false, false, false}, true, "DIK_NUMPAD4" },		//define DIK_NUMPAD4         0x4B
	{ true,  false, { false, false, false, false}, true, "DIK_NUMPAD5" },		//define DIK_NUMPAD5         0x4C
	{ true,  false, { false, false, false, false}, true, "DIK_NUMPAD6" },		//define DIK_NUMPAD6         0x4D
	{ true,  false, { false, false, false, false}, true, "DIK_ADD" },		//define DIK_ADD             0x4E    /* + on numeric keypad */
	{ true,  false, { false, false, false, false}, true, "DIK_NUMPAD1" },		//define DIK_NUMPAD1         0x4F
	{ true,  false, { false, false, false, false}, true, "DIK_NUMPAD2" },		//define DIK_NUMPAD2         0x50
	{ true,  false, { false, false, false, false}, true, "DIK_NUMPAD3" },		//define DIK_NUMPAD3         0x51
	{ true,  false, { false, false, false, false}, true, "DIK_NUMPAD0" },		//define DIK_NUMPAD0         0x52
	{ true,  false, { false, false, false, false}, true, "DIK_DECIMAL" },		//define DIK_DECIMAL         0x53    /* . on numeric keypad */
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x54
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x55
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x56
	{ true,  false, { false, false, false, false}, false, "DIK_F11" },		//define DIK_F11             0x57
	{ true,  false, { false, false, false, false}, false, "DIK_F12" },		//define DIK_F12             0x58
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x59
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x5A
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x5B
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x5C
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x5D
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x5E
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x5F
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x60
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x61
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x62
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x63
	{ true,  false, { false, false, false, false}, false, "DIK_F13" },		//define DIK_F13             0x64    /*                     (NEC PC98) */
	{ true,  false, { false, false, false, false}, false, "DIK_F14" },		//define DIK_F14             0x65    /*                     (NEC PC98) */
	{ true,  false, { false, false, false, false}, false, "DIK_F15" },		//define DIK_F15             0x66    /*                     (NEC PC98) */
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x67
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x68
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x69
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x6A
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x6B
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x6C
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x6D
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x6E
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x6F
	{ true,  false, { false, false, false, false}, true, "DIK_KANA" },		//define DIK_KANA            0x70    /* (Japanese keyboard)            */
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x71
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x72
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x73
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x74
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x75
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x76
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x77
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x78
	{ true,  false, { false, false, false, false}, true, "DIK_CONVERT" },		//define DIK_CONVERT         0x79    /* (Japanese keyboard)            */
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x7A
	{ true,  false, { false, false, false, false}, true, "DIK_NOCONVERT" },		//define DIK_NOCONVERT       0x7B    /* (Japanese keyboard)            */
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x7C
	{ true,  false, { false, false, false, false}, true, "DIK_YEN" },		//define DIK_YEN             0x7D    /* (Japanese keyboard)            */
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x7E
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x7F
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x80
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x81
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x82
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x83
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x84
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x85
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x86
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x87
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x88
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x89
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x8A
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x8B
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x8C
	{ true,  false, { false, false, false, false}, true, "DIK_NUMPADEQUALS" },		//define DIK_NUMPADEQUALS    0x8D    /* = on numeric keypad (NEC PC98) */
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x8E
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x8F
	{ true,  false, { false, false, false, false}, true, "DIK_CIRCUMFLEX" },		//define DIK_CIRCUMFLEX      0x90    /* (Japanese keyboard)            */
	{ true,  false, { false, false, false, false}, true, "DIK_AT" },		//define DIK_AT              0x91    /*                     (NEC PC98) */
	{ true,  false, { false, false, false, false}, true, "DIK_COLON" },		//define DIK_COLON           0x92    /*                     (NEC PC98) */
	{ true,  false, { false, false, false, false}, true, "DIK_UNDERLINE" },		//define DIK_UNDERLINE       0x93    /*                     (NEC PC98) */
	{ true,  false, { false, false, false, false}, true, "DIK_KANJI" },		//define DIK_KANJI           0x94    /* (Japanese keyboard)            */
	{ true,  false, { false, false, false, false}, true, "DIK_STOP" },		//define DIK_STOP            0x95    /*                     (NEC PC98) */
	{ true,  false, { false, false, false, false}, true, "DIK_AX" },		//define DIK_AX              0x96    /*                     (Japan AX) */
	{ true,  false, { false, false, false, false}, true, "DIK_UNLABELED" },		//define DIK_UNLABELED       0x97    /*                        (J3100) */
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x98
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x99
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x9A
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x9B
	{ true,  false, { false, false, false, false}, true, "DIK_NUMPADENTER" },		//define DIK_NUMPADENTER     0x9C    /* Enter on numeric keypad */
	{ true,  false, { false, false, false, false}, true, "DIK_RCONTROL" },		//define DIK_RCONTROL        0x9D
//-------
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x9E
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0x9F
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xA0
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xA1
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xA2
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xA3
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xA4
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xA5
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xA6
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xA7
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xA8
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xA9
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xAA
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xAB
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xAC
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xAD
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xAE
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xAF
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xB0
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xB1
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xB2
	{ true,  false, { false, false, false, false}, true, "DIK_NUMPADCOMMA" },		//define DIK_NUMPADCOMMA     0xB3    /* , on numeric keypad (NEC PC98) */
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xB4
	{ true,  false, { false, false, false, false}, true, "DIK_DIVIDE" },		//define DIK_DIVIDE          0xB5    /* / on numeric keypad */
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xB6
	{ true,  false, { false, false, false, false}, false, "DIK_SYSRQ" },		//define DIK_SYSRQ           0xB7
	{ true,  false, { false, false, false, false}, true, "DIK_RMENU" },		//define DIK_RMENU           0xB8    /* right Alt */
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xB9
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xBA
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xBB
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xBC
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xBD
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xBE
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xBF
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xC0
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xC1
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xC2
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xC3
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xC4
	{ true,  false, { false, false, false, false}, false, "DIK_PAUSE" }, // DIK_PAUSE (not part of DX defines)      0xC5
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xC6
	{ true,  false, { false, false, false, false}, true, "DIK_HOME" },		//define DIK_HOME            0xC7    /* Home on arrow keypad */
	{ true,  false, { false, false, false, false}, true, "DIK_UP" },		//define DIK_UP              0xC8    /* UpArrow on arrow keypad */
	{ true,  false, { false, false, false, false}, true, "DIK_PRIOR" },		//define DIK_PRIOR           0xC9    /* PgUp on arrow keypad */
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xCA
	{ true,  false, { false, false, false, false}, true, "DIK_LEFT" },		//define DIK_LEFT            0xCB    /* LeftArrow on arrow keypad */
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xCC
	{ true,  false, { false, false, false, false}, true, "DIK_RIGHT" },		//define DIK_RIGHT           0xCD    /* RightArrow on arrow keypad */
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xCE
	{ true,  false, { false, false, false, false}, true, "DIK_END" },		//define DIK_END             0xCF    /* End on arrow keypad */
	{ true,  false, { false, false, false, false}, true, "DIK_DOWN" },		//define DIK_DOWN            0xD0    /* DownArrow on arrow keypad */
	{ true,  false, { false, false, false, false}, true, "DIK_NEXT" },		//define DIK_NEXT            0xD1    /* PgDn on arrow keypad */
	{ true,  false, { false, false, false, false}, true, "DIK_INSERT" },		//define DIK_INSERT          0xD2    /* Insert on arrow keypad */
	{ true,  false, { false, false, false, false}, true, "DIK_DELETE" },		//define DIK_DELETE          0xD3    /* Delete on arrow keypad */
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xD4
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xD5
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xD6
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xD7
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xD8
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xD9
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xDA
	{ true,  false, { false, false, false, false}, true, "DIK_LWIN" },		//define DIK_LWIN            0xDB    /* Left Windows key */
	{ true,  false, { false, false, false, false}, true, "DIK_RWIN" },		//define DIK_RWIN            0xDC    /* Right Windows key */
	{ true,  false, { false, false, false, false}, true, "DIK_APPS" },		//define DIK_APPS            0xDD    /* AppMenu key */
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xDE
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xDF
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xE0
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xE1
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xE2
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xE3
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xE4
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xE5
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xE6
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xE7
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xE8
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xE9
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xEA
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xEB
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xEC
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xED
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xEE
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xEF
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xF0
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xF1
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xF2
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xF3
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xF4
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xF5
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xF6
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xF7
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xF8
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xF9
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xFA
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xFB
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xFC
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xFD
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xFE
	{ true,  false, { false, false, false, false}, false, 0 },		// ???             0xFF
};


const char* DikGetName(DWORD dik, bool complete)
{
	assert(dik < 256);

	const char* name = dikAttrib[dik].text;

	if (name == 0)
		return 0;

	if ( ! complete)
		if (name[0] == 'D' && name[1] == 'I' && name[2] == 'K' && name[3] == '_')
			return name + 4;

	return name;
}



