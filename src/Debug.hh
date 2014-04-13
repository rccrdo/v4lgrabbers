/*
 * Copyright (c) 2007 Riccardo Lucchese, riccardo.lucchese at gmail.com
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

#ifndef Debug_HH
#define Debug_HH

// ******************************
//  ASSERT
// to make assert do nothing uncomment this
//#define NDEBUG
#include <assert.h>



// ******************************
//  VERBOSE
//  [enable verbose output to std::cout; comment only #defines to disable verbose output]


// don't touch these...
#undef Vista_Enable_All_Verbose
#undef Vista_Disable_All_Verbose
#undef Grabber_Verbose
#undef V4L2_Device_Verbose
#undef V4L1_Device_Verbose


#define Vista_Enable_All_Verbose
//#define Vista_Disable_All_Verbose

// per class defines
//#define V4L2_Device_Verbose
//#define V4L1_Device_Verbose


// if any implementor of Grabber is "verbose" rise this define also for grabber
// [some implementors may use some helper functions that are not built when Grabber_Verbose is undefined]
#ifdef V4L1_Device_Verbose
#define Grabber_Verbose
#endif

#ifdef V4L2_Device_Verbose
#define Grabber_Verbose
#endif

// if verbose mode was enabled for all Vista's stuff than set'em
#ifdef Vista_Enable_All_Verbose
#define V4L2_Device_Verbose
#define V4L1_Device_Verbose
#define Grabber_Verbose
#endif

// if verbose mode was disabled for all Vista's stuff than unset'em
#ifdef Vista_Disable_All_Verbose
#undef V4L2_Device_Verbose
#undef V4L1_Device_Verbose
#undef Grabber_Verbose
#endif


#endif /*Debug_HH*/
