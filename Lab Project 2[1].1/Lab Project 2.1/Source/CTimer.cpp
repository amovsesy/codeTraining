//-----------------------------------------------------------------------------
// File: CTimer.cpp
//
// Desc: This class handles all timing functionality. This includes counting
//       the number of frames per second, to scaling vectors and values
//       relative to the time that has passed since the previous frame.
//
// Copyright (c) 1997-2002 Daedalus Developments. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CTimer Specific Includes
//-----------------------------------------------------------------------------
#include "..\\Includes\\CTimer.h"

//-----------------------------------------------------------------------------
// Name : CTimer () (Constructor)
// Desc : CTimer Class Constructor
//-----------------------------------------------------------------------------
CTimer::CTimer()
{
	// Query performance hardware and setup time scaling values
	if (QueryPerformanceFrequency((LARGE_INTEGER *)&m_PerfFreq)) 
    { 
		m_PerfHardware		= TRUE;
		QueryPerformanceCounter((LARGE_INTEGER *) &m_LastTime); 
		m_TimeScale			= 1.0f / m_PerfFreq;
	} 
    else 
    { 
		// no performance counter, read in using timeGetTime 
		m_PerfHardware		= FALSE;
		m_LastTime			= timeGetTime(); 
		m_TimeScale			= 0.001f;
	
    } // End If No Hardware

	// Clear any needed values
	m_FrameRate			= 0;
	m_FPSFrameCount		= 0;
	m_FPSTimeElapsed	= 0.0f;
}

//-----------------------------------------------------------------------------
// Name : CTimer () (Destructor)
// Desc : CTimer Class Destructor
//-----------------------------------------------------------------------------
CTimer::~CTimer()
{
}

//-----------------------------------------------------------------------------
// Name : Tick () 
// Desc : Function which signals that frame has advanced
// Note : You can specify a number of frames per second to lock the frame rate
//        to. This will simply soak up the remaining time to hit that target.
//-----------------------------------------------------------------------------
void CTimer::Tick( float fLockFPS )
{
    // Is performance hardware available?
	if ( m_PerfHardware ) 
    {
        // Query high-resolution performance hardware
		QueryPerformanceCounter((LARGE_INTEGER *)&m_CurrentTime);
	} 
    else 
    {
        // Fall back to less accurate timer
		m_CurrentTime = timeGetTime();

	} // End If no hardware available

	// Calculate elapsed time in seconds
	m_TimeElapsed = (m_CurrentTime - m_LastTime) * m_TimeScale;

    // Should we lock the frame rate ?
    if ( fLockFPS > 0.0f )
    {
        while ( m_TimeElapsed < (1.0f / fLockFPS))
        {
            // Is performance hardware available?
	        if ( m_PerfHardware ) 
            {
                // Query high-resolution performance hardware
		        QueryPerformanceCounter((LARGE_INTEGER*)&m_CurrentTime);
	        } 
            else 
            {
                // Fall back to less accurate timer
		        m_CurrentTime = timeGetTime();

	        } // End If no hardware available

	        // Calculate elapsed time in seconds
	        m_TimeElapsed = (m_CurrentTime - m_LastTime) * m_TimeScale;

        } // End While
    } // End If

	// Save current frame time
	m_LastTime = m_CurrentTime;

	// Calculate Frame Rate
	m_FPSFrameCount++;
	m_FPSTimeElapsed += m_TimeElapsed;
	if ( m_FPSTimeElapsed > 1.0f) 
    {
		m_FrameRate			= m_FPSFrameCount;
		m_FPSFrameCount		= 0;
		m_FPSTimeElapsed	= 0.0f;
	} // End If Second Elapsed

}

//-----------------------------------------------------------------------------
// Name : GetFrameRate () 
// Desc : Returns the frame rate, sampled over the last second or so.
//-----------------------------------------------------------------------------
unsigned long CTimer::GetFrameRate( LPTSTR lpszString ) const
{
    // Fill string buffer ?
    if ( lpszString )
    {
        // Copy frame rate value into string
        _itot( m_FrameRate, lpszString, 10 );

        // Append with FPS
        strcat( lpszString, _T(" FPS") );

    } // End if build FPS string

    return m_FrameRate;
}

//-----------------------------------------------------------------------------
// Name : GetTimeElapsed () 
// Desc : Returns the amount of time elapsed since the last frame (Seconds)
//-----------------------------------------------------------------------------
float CTimer::GetTimeElapsed() const
{
    return m_TimeElapsed;
}
