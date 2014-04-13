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


#ifndef Grabber_Helpers_HH
#define Grabber_Helpers_HH

#include <string>
#include <sstream>
#include "Debug.hh"
#include "GrabberControlData.hh"
#include "PixelBuffer.hh"

unsigned int pixelbuffer_length (PixelBufferFormat fmt, unsigned int w, unsigned int h);

#ifdef Grabber_Verbose
std::string pixelbuffer_fmt_to_string(PixelBufferFormat fmt);
std::string grabber_ctrl_data_to_string (GrabberControlData* gData);
std::string grabber_ctrl_id_to_string(GrabberControlID id);
#endif

#endif /*Grabber_Helpers_HH*/
