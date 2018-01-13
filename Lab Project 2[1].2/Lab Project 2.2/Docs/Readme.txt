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
//                  Chapter 2 - Demo 2 - Enumeration v1.0                    \\
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

This demo demonstrates how to enumerate all the various features, modes and
settings exposed by the end-user graphics hardware, and how to initialize 
Direct Graphics in such a way that it uses those selected features. Although
there are many different types of hardware features available, such as hardware
vertex processing ( Hardware Transform and Lighting ), anti-aliasing, many
different display modes etc, in a real world situation it is not usually
necessary to go much deeper than enumerating all the display modes alone. Any
additional device features would simply be tested once, not collected as we
do here.

2. General Usage
----------------

The application is a low interactivity demo which simply demonstrates both the
enumeration for supported features, and the creation of the Direct 3D devices
which make use of any selected features. Whilst the application is running (and
once at startup) you may select new device settings via the Direct3D Settings
dialog, which can be accessed via the "File / Change Device" menu item.

3. Controls
-----------

Escape         - Exit
Cursor Left    - Strafe Camera Left
Cursor Right   - Strafe Camera Right
Shift + Return - Toggle between windowed / fullscreen modes

4. Trouble Shooting
-------------------

None at this time.