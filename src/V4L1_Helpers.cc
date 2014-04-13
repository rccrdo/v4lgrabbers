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


#include "V4L1_Helpers.hh"


#ifdef V4L1_Device_Verbose
std::string v4l1_capabilities_to_string(video_capability &cap) {
	std::stringstream s; s.str("");
	s << " - name: " 	<< cap.name << "\n"
	  << " - max width: "	<< cap.maxwidth << "\n"
	  << " - max height: "<< cap.maxheight << "\n"
	  << " - min width: "	<< cap.minwidth << "\n"
	  << " - min height: "<< cap.minheight << "\n";

	s << " - video capture: ";
	if ( cap.type & VID_TYPE_CAPTURE) s << "YES\n"; else s << "NO\n";

	s << " - image scaling support: ";
	if ( cap.type & VID_TYPE_SCALES) s << "YES\n"; else s << "NO\n";

	s << " - grey tones images only: ";
	if ( cap.type & VID_TYPE_MONOCHROME) s << "YES\n"; else s << "NO\n";

	s << " - crop support: ";
	if ( cap.type & VID_TYPE_SUBCAPTURE) s << "YES\n"; else s << "NO\n";
	return s.str();
}
#endif


PixelBufferFormat v4l1_palette_to_pixelbuffer_fmt(unsigned int palette) {
	switch (palette) {
// RGB
	case (VIDEO_PALETTE_RGB555)   :  return PIXELBUFFER_FMT_RGBO;
	case (VIDEO_PALETTE_RGB565)   :  return PIXELBUFFER_FMT_RGBP;
	case (VIDEO_PALETTE_RGB24)    :  return PIXELBUFFER_FMT_RGB3;
	case (VIDEO_PALETTE_RGB32)    :  return PIXELBUFFER_FMT_RGB4;
// YUV
	case (VIDEO_PALETTE_GREY)     :  return PIXELBUFFER_FMT_GREY;
	case (VIDEO_PALETTE_YUYV)     :  return PIXELBUFFER_FMT_YUYV;
	case (VIDEO_PALETTE_UYVY)     :  return PIXELBUFFER_FMT_UYVY;
	case (VIDEO_PALETTE_YUV420)   :  return PIXELBUFFER_FMT_YU12;
	case (VIDEO_PALETTE_YUV422P)  :  return PIXELBUFFER_FMT_422P;
	case (VIDEO_PALETTE_YUV411P)  :  return PIXELBUFFER_FMT_411P;
	default : {
		// if we get here fmt has bad value and this should never happen
		return PIXELBUFFER_FMT_NONE;
	}
	}
}

