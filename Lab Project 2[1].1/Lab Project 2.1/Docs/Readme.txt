//***************************************************************************\\
//                                                                           \\
//         3D Graphics Programming with...                                   \\
//              ____  _               _  __  __   ___   ___                  \\
//             |  _ \(_)_ __ ___  ___| |_\ \/ /  / _ \ / _ \                 \\
//             | | | | | '__/ _ \/ __| __|\  /  | (_) | | | |                \\
//             | |_| | | | |  __/ (__| |_ /  \   \__, | |_| |                \\
//             |____/|_|_|  \___|\___|\__/_/\_\    /_(_)___/                 \\
//                                                                           \\
//---------------------------------------------------------------------------\\
//               Chapter 2 - Demo 1 - Initialization - v1.0                  \\
//---------------------------------------------------------------------------\\
//                                                                           \\
// Copyright (c) 1997 - 2002 Adam Hoult & Gary Simmons. All Rights Reserved. \\
//                                                                           \\
//***************************************************************************\\


                            Table Of Contents
                            -----------------

                            1. Introduction
                            2. General Usage
                            3. Controls
                            4. Trouble Shooting

	
1. Introduction
---------------

This demo is designed to demonstrate the simple basics behind creating and
initializing the various objects and components required by Direct Graphics
for the purposes of rendering or otherwise. As in previous examples, at this
point we simply render two code generated cubes to ensure full function of the
rendering device, with the exception that in this case, the polygons are now
rendered using Direct Graphics itself.

Although no device enumeration is performed within this application, the device
is created using parameters which will generally be available on almost all
hardware that supports windowed rendering. If no HAL device can be created in
windowed mode, the code will attempt to create a REF device in it's place.
In the next demonstration, device enumeration will be performed, allowing us 
much greater control over the device formats, display modes and general 
processing parameters needed.

2. General Usage
----------------

The application is a low interactivity demo which simply demonstrates the
creation of the Direct 3D device used for a simple rendering procedure. No 
enumeration is performed, and therfore only the most general feature set can be
assumed.

3. Controls
-----------

Escape       - Exit
Cursor Left  - Strafe Camera Left
Cursor Right - Strafe Camera Right

4. Trouble Shooting
-------------------

None at this time.