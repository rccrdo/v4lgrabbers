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


#include "Grabber_Helpers.hh"

unsigned int pixelbuffer_length(PixelBufferFormat fmt, unsigned int w, unsigned int h) {
	/* 1 byte per pixel formats */
	if ( (fmt == PIXELBUFFER_FMT_RGB1) or
	     (fmt == PIXELBUFFER_FMT_BA81) or
	     (fmt == PIXELBUFFER_FMT_GREY)   ) return (int) ( w*h );

	/* 2 byte per pixel formats */
	if ( (fmt == PIXELBUFFER_FMT_RGBO) or
	     (fmt == PIXELBUFFER_FMT_RGBP) or
	     (fmt == PIXELBUFFER_FMT_R444) or
	     (fmt == PIXELBUFFER_FMT_RGBQ) or
	     (fmt == PIXELBUFFER_FMT_RGBR) or
	     (fmt == PIXELBUFFER_FMT_YUYV) or
	     (fmt == PIXELBUFFER_FMT_UYVY) or
	     (fmt == PIXELBUFFER_FMT_YV12) or
	     (fmt == PIXELBUFFER_FMT_YU12) or
	     (fmt == PIXELBUFFER_FMT_422P)   ) return (int) ( w*h*2 );

	/* 3 byte per 2 pixels formats */
	if ( (fmt == PIXELBUFFER_FMT_Y41P) or
	     (fmt == PIXELBUFFER_FMT_YVU9) or
	     (fmt == PIXELBUFFER_FMT_YUV9) or
	     (fmt == PIXELBUFFER_FMT_411P) or
	     (fmt == PIXELBUFFER_FMT_NV12) or
	     (fmt == PIXELBUFFER_FMT_NV21)   ) return (int) ( ((w*h*3)/2)+1 );

	/* 3 byte per pixel formats */
	if ( (fmt == PIXELBUFFER_FMT_BGR3) or
	     (fmt == PIXELBUFFER_FMT_RGB3)   ) return (int) ( w*h*3 );

	/* 4 byte per pixel formats */
	if ( (fmt == PIXELBUFFER_FMT_BGR4) or
	     (fmt == PIXELBUFFER_FMT_RGB4)   ) return (int) ( w*h*4 );

	/* if we get here fmt is of un-handled type */
	return 0;
}

#ifdef Grabber_Verbose
std::string pixelbuffer_fmt_to_string(PixelBufferFormat fmt) {
	switch (fmt) {
		/* RGB formats */
	case (PIXELBUFFER_FMT_RGB1)  :  return "RGB1";
	case (PIXELBUFFER_FMT_RGBO)  :  return "RGBO";
	case (PIXELBUFFER_FMT_RGBP)  :  return "RGBP";
	case (PIXELBUFFER_FMT_R444)  :  return "R444";
	case (PIXELBUFFER_FMT_RGBQ)  :  return "RGBQ";
	case (PIXELBUFFER_FMT_RGBR)  :  return "RGBR";
	case (PIXELBUFFER_FMT_BGR3)  :  return "BGR3";
	case (PIXELBUFFER_FMT_RGB3)  :  return "RGB3";
	case (PIXELBUFFER_FMT_BGR4)  :  return "BGR4";
	case (PIXELBUFFER_FMT_RGB4)  :  return "RGB4";
		/* RGB bayern */
	case (PIXELBUFFER_FMT_BA81)  :  return "BA81 : Bayer";
		/* YUV */
	case (PIXELBUFFER_FMT_GREY)  :  return "GREY";
	case (PIXELBUFFER_FMT_YUYV)  :  return "YUYV - YUV 4:2:2";
	case (PIXELBUFFER_FMT_UYVY)  :  return "UYVY"; // 4 bytes is 2 pixels
	case (PIXELBUFFER_FMT_Y41P)  :  return "Y41P - YUV 4:1:1";
	case (PIXELBUFFER_FMT_YV12)  :  return "YV12 - YUV 4:2:0";
	case (PIXELBUFFER_FMT_YU12)  :  return "YU12 - YUV 4:2:0";
	case (PIXELBUFFER_FMT_YVU9)  :  return "YVU9 - YUV 4:1:0";
	case (PIXELBUFFER_FMT_YUV9)  :  return "YUV9 - YUV 4:1:0";
	case (PIXELBUFFER_FMT_422P)  :  return "422P - YUV 4:2:2";
	case (PIXELBUFFER_FMT_411P)  :  return "411P - YUV 4:1:1";
	case (PIXELBUFFER_FMT_NV12)  :  return "NV12 - YUV 4:2:0";
	case (PIXELBUFFER_FMT_NV21)  :  return "NV21 - YUV 4:2:0";
	default : return "! WARNING ! unknown format";
	}
}
#endif


#ifdef Grabber_Verbose
std::string grabber_ctrl_data_to_string (GrabberControlData* gData) {
	std::stringstream temp;
	if (!gData) return "! WARNING ! NULL ptr";
	temp 	<< "min: " << gData->min	<< "\t max: " << gData->max
		<< "\t step: " << gData->step	<< "\t def: " << gData->def
		<< "\t value: " << gData->value;
	return temp.str();
}
#endif

#ifdef Grabber_Verbose
std::string grabber_ctrl_id_to_string(GrabberControlID id) {
	switch (id) {
	case (GRABBER_CTRL_BRIGHTNESS)         :  return "brightness";
	case (GRABBER_CTRL_CONTRAST)           :  return "contrast";
	case (GRABBER_CTRL_SATURATION)         :  return "saturation";
	case (GRABBER_CTRL_HUE)                :  return "hue";
	case (GRABBER_CTRL_AUTO_WHITE_BALANCE) :  return "auto white balance";
	case (GRABBER_CTRL_RED_BALANCE)        :  return "red balance";
	case (GRABBER_CTRL_BLUE_BALANCE)       :  return "blue balance";
	case (GRABBER_CTRL_GAMMA)              :  return "gamma";
	case (GRABBER_CTRL_EXPOSURE)           :  return "exsposure";
	case (GRABBER_CTRL_AUTOGAIN)           :  return "auto-gain";
	case (GRABBER_CTRL_GAIN)               :  return "gain";
	case (GRABBER_CTRL_HFLIP)              :  return "H-flip";
	case (GRABBER_CTRL_VFLIP)              :  return "V-flip";
	default : return "! WARNING ! unknown ctrl id";
	}
}
#endif

