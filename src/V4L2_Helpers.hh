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


#ifndef V4L2_Helpers_HH
#define V4L2_Helpers_HH

#include <string>
#include <sstream>

#include <iostream>
#include <errno.h>

#include "Debug.hh"
#include "GrabberControlData.hh"
#include "PixelBuffer.hh"
#include "Grabber_Helpers.hh"

extern "C" {
#include <linux/videodev2.h>
}

#ifdef V4L2_Device_Verbose
std::string errnoToString (int err);
std::string v4l2_capabilities_to_string(unsigned int cap);
std::string v4l2_crop_scale_opt_to_string(v4l2_buf_type t);
std::string v4l2_capture_param_to_string(v4l2_captureparm* capP);
std::string v4l2_pix_format_struct_to_string(v4l2_pix_format* pixFfmt);
std::string v4l2_color_space_to_string(v4l2_colorspace cspace);
std::string v4l2_field_struct_to_string(v4l2_field field);
std::string v4l2_rect_struct_to_string(v4l2_rect rect);
#endif

unsigned int grabber_ctrl_id_to_v4l2_ctrl_id (GrabberControlID id);
GrabberControlID v4l2_ctrl_id_to_grabber_ctrl_id (unsigned int id);

unsigned int pixelbuffer_fmt_to_v4l2_pix_fmt(PixelBufferFormat fmt);
PixelBufferFormat v4l2_pix_fmt_to_pixelbuffer_fmt(unsigned int fmt);

#endif /*V4L2_Helpers_HH*/
