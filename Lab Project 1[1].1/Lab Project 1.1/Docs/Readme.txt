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
//               Chapter 1 - Demo 1 - Software Render v1.0                   \\
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

This demo represents an extremely simplified transformation pipeline, designed
to show the conversion of three dimensional mesh points, into two dimensional
screen coordinates, ready for rasterization. The mesh we are using is generated
in code, and is rendered using simple GDI techniques for clarity.

Please note that the purpose of this demonstration is to show the processes
discussed within the material in a clear and concise manner, therefore no speed
optimisations are applied.

2. General Usage
----------------

The application is a non interactive demo which simply demonstrates multiple
rotating meshes. This rotational animation can be enabled or disabled via the 
application menu.

3. Controls
-----------

None.

4. Trouble Shooting
-------------------

Systems utilising multiple monitors which are set with differing color depths,
may experience a significant loss in frame rate due to the GDI color format
conversions performed when presenting the frame buffer via ::BitBlt(). To 
prevent this problem from occuring, it is recommended that you disable any 
additional monitors.