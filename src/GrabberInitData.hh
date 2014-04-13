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


#ifndef GrabberInitData_HH
#define GrabberInitData_HH

#include <string>
#include "PixelBuffer.hh"

struct GrabberInitData {
	// *** standard grabber init data ***
	std::string pathToDev;	// path to device: ie. /dev/video0

	unsigned int maxWidth;	// max image's width we want to grab ( ! we make it clear so that we can allocate less memory !)
	unsigned int maxHeight;	// max image's height we want to grab

	// *** v4lx devices init data ***
	unsigned int maxNumBuffers;	// limit to the max number of PixelBuffer(s) that can be allocated by the grabber
	// ie. webcams work better with many buffers, but many buffers means much memory


	/* TODO: */
	// CropAndScaleData
	PixelBufferFormat fmt;
};


#endif /*GrabberInitData_HH*/
