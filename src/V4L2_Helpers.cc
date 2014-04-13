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


#include "V4L2_Helpers.hh"

#ifdef V4L2_Device_Verbose
std::string errnoToString (int err) {
	switch (err) {
	case (EACCES)  :  return "EACCES";
	case (EINVAL)  :  return "EINVAL";
	case (ERANGE)  :  return "ERANGE";
	case (EBUSY)   :  return "EBUSY";
	case (ENOMEM)  :  return "ENOMEM";
	case (ENXIO)   :  return "ENXIO";
	case (EMFILE)  :  return "EMFILE";
	case (ENFILE)  :  return "ENFILE";
	default 	   :  return "! unknown errno value";
	}
}
#endif


#ifdef V4L2_Device_Verbose
std::string v4l2_pix_format_struct_to_string(v4l2_pix_format* pixFmt) {
	std::stringstream temp;
	temp << " - width       : " << pixFmt->width << "\n"
	     << " - height      : " << pixFmt->height << "\n"
	     << " - bytesPerLine: " << pixFmt->bytesperline << "\n"
	     << " - img size    : " << pixFmt->sizeimage << "\n"
	     << " - pixelformat : " << pixelbuffer_fmt_to_string( v4l2_pix_fmt_to_pixelbuffer_fmt(pixFmt->pixelformat) ) << "\n"
	     << " - field       : " << v4l2_field_struct_to_string(pixFmt->field) << "\n"
	     << " - colorspace  : " << v4l2_color_space_to_string(pixFmt->colorspace);
	return temp.str();
}
#endif


#ifdef V4L2_Device_Verbose
std::string v4l2_capture_param_to_string(v4l2_captureparm* capP) {
	std::stringstream temp;
	temp << " - frame skipping/repeating support: ";
	if (capP->capability & V4L2_CAP_TIMEPERFRAME)  temp << "YES\n"; else temp << "NO\n";

	temp << " - HQ mode support: ";
	if (capP->capturemode & V4L2_MODE_HIGHQUALITY) temp << "YES\n"; else temp << "NO\n";

	temp << " - freq: " << ( (float)capP->timeperframe.denominator / (float) capP->timeperframe.numerator) << "\n"
	     << " - internal buffers: " << capP->readbuffers;
	return temp.str();
}
#endif


#ifdef V4L2_Device_Verbose
std::string v4l2_field_struct_to_string(v4l2_field field) {
	switch (field) {
	case (V4L2_FIELD_ANY)         :  return "ANY";
	case (V4L2_FIELD_NONE)        :  return "NONE";
	case (V4L2_FIELD_TOP)         :  return "TOP";
	case (V4L2_FIELD_BOTTOM)      :  return "BOTTOM";
	case (V4L2_FIELD_INTERLACED)  :  return "INTERLACED";
	case (V4L2_FIELD_SEQ_TB) 	  :  return "SEQ_TB";
	case (V4L2_FIELD_SEQ_BT)      :  return "SEQ_BT";
	case (V4L2_FIELD_ALTERNATE)   :  return "ALTERNATE";
	default : return "! unknown field";
	}
}
#endif


#ifdef V4L2_Device_Verbose
std::string v4l2_color_space_to_string(v4l2_colorspace cspace) {
	switch (cspace) {
	case (V4L2_COLORSPACE_SMPTE170M)      :  return "SMPTE170M";
	case (V4L2_COLORSPACE_SMPTE240M)      :  return "SMPTE240M";
	case (V4L2_COLORSPACE_REC709)         :  return "REC709";
	case (V4L2_COLORSPACE_BT878)          :  return "BT878";
	case (V4L2_COLORSPACE_470_SYSTEM_M)   :  return "SYSTEM_M";
	case (V4L2_COLORSPACE_470_SYSTEM_BG)  :  return "SYSTEM_BG";
	case (V4L2_COLORSPACE_JPEG)           :  return "JPEG";
	case (V4L2_COLORSPACE_SRGB)           :  return "SRGB";
	default: return "! unknown colorspace";
	}
}
#endif


#ifdef V4L2_Device_Verbose
std::string v4l2_capabilities_to_string(unsigned int cap) {
	std::string s("");
	s += " - video capture: ";
	if ( cap & V4L2_CAP_VIDEO_CAPTURE) s += "YES\n"; else s += "NO\n";

	s += " - read() I/O: ";
	if ( cap & V4L2_CAP_READWRITE) s += "YES\n"; else s += "NO\n";

	return s;
}
#endif


#ifdef V4L2_Device_Verbose
std::string v4l2_crop_scale_opt_to_string(v4l2_buf_type t) {
	switch (t) {
	case (V4L2_BUF_TYPE_VIDEO_CAPTURE)  :  return "VIDEO_CAPTURE";
	case (V4L2_BUF_TYPE_VIDEO_OUTPUT)   :  return "VIDEO_OUTPUT";
	case (V4L2_BUF_TYPE_VIDEO_OVERLAY)  :  return "VIDEO_OVERLAY";
	case (V4L2_BUF_TYPE_PRIVATE)        :  return "PRIVATE";
	default : return "! unknown buffer type";
	}
}
#endif


#ifdef V4L2_Device_Verbose
std::string v4l2_rect_struct_to_string(v4l2_rect rect) {
	std::stringstream temp;
	temp << "left " << rect.left 
	     << "\t top " << rect.top 
	     << "\t width " << rect.width 
	     << "\t height " << rect.height;
	return temp.str();
}
#endif


unsigned int grabber_ctrl_id_to_v4l2_ctrl_id (GrabberControlID id) {
	switch (id) {
	case (GRABBER_CTRL_BRIGHTNESS)         :  return V4L2_CID_BRIGHTNESS;
	case (GRABBER_CTRL_CONTRAST)           :  return V4L2_CID_CONTRAST;
	case (GRABBER_CTRL_SATURATION)         :  return V4L2_CID_SATURATION;
	case (GRABBER_CTRL_HUE)                :  return V4L2_CID_HUE;
	case (GRABBER_CTRL_AUTO_WHITE_BALANCE) :  return V4L2_CID_AUTO_WHITE_BALANCE;
	case (GRABBER_CTRL_RED_BALANCE)        :  return V4L2_CID_RED_BALANCE;
	case (GRABBER_CTRL_BLUE_BALANCE)       :  return V4L2_CID_BLUE_BALANCE;
	case (GRABBER_CTRL_GAMMA)              :  return V4L2_CID_GAMMA;
	case (GRABBER_CTRL_EXPOSURE)           :  return V4L2_CID_EXPOSURE;
	case (GRABBER_CTRL_AUTOGAIN)           :  return V4L2_CID_AUTOGAIN;
	case (GRABBER_CTRL_GAIN)               :  return V4L2_CID_GAIN;
	case (GRABBER_CTRL_HFLIP)              :  return V4L2_CID_HFLIP;
	case (GRABBER_CTRL_VFLIP)              :  return V4L2_CID_VFLIP;
	default: {
		// if we get here id has a bad value and this should never happen
		assert(0);
	}
	} 
}


GrabberControlID v4l2_ctrl_id_to_grabber_ctrl_id (unsigned int id) {
	switch (id) {
	case (V4L2_CID_BRIGHTNESS)         :  return GRABBER_CTRL_BRIGHTNESS;
	case (V4L2_CID_CONTRAST)           :  return GRABBER_CTRL_CONTRAST;
	case (V4L2_CID_SATURATION)         :  return GRABBER_CTRL_SATURATION;
	case (V4L2_CID_HUE)                :  return GRABBER_CTRL_HUE;
	case (V4L2_CID_AUTO_WHITE_BALANCE) :  return GRABBER_CTRL_AUTO_WHITE_BALANCE;
	case (V4L2_CID_RED_BALANCE)        :  return GRABBER_CTRL_RED_BALANCE;
	case (V4L2_CID_BLUE_BALANCE)       :  return GRABBER_CTRL_BLUE_BALANCE;
	case (V4L2_CID_GAMMA)              :  return GRABBER_CTRL_GAMMA;
	case (V4L2_CID_EXPOSURE)           :  return GRABBER_CTRL_EXPOSURE;
	case (V4L2_CID_AUTOGAIN)           :  return GRABBER_CTRL_AUTOGAIN;
	case (V4L2_CID_GAIN)               :  return GRABBER_CTRL_GAIN;
	case (V4L2_CID_HFLIP)              :  return GRABBER_CTRL_HFLIP;
	case (V4L2_CID_VFLIP)              :  return GRABBER_CTRL_VFLIP;
	default: return GRABBER_CTRL_NONE; // a kind of control we don't care about
	} 
}



unsigned int pixelbuffer_fmt_to_v4l2_pix_fmt(PixelBufferFormat fmt) {
	switch (fmt) {
	case (PIXELBUFFER_FMT_RGB1)  :  return V4L2_PIX_FMT_RGB332;
	case (PIXELBUFFER_FMT_RGBO)  :  return V4L2_PIX_FMT_RGB555;
	case (PIXELBUFFER_FMT_RGBP)  :  return V4L2_PIX_FMT_RGB565;
	case (PIXELBUFFER_FMT_R444)  :  return V4L2_PIX_FMT_RGB444;
	case (PIXELBUFFER_FMT_RGBQ)  :  return V4L2_PIX_FMT_RGB555X;
	case (PIXELBUFFER_FMT_RGBR)  :  return V4L2_PIX_FMT_RGB565X;
	case (PIXELBUFFER_FMT_BGR3)  :  return V4L2_PIX_FMT_BGR24;
	case (PIXELBUFFER_FMT_RGB3)  :  return V4L2_PIX_FMT_RGB24;
	case (PIXELBUFFER_FMT_BGR4)  :  return V4L2_PIX_FMT_BGR32;
	case (PIXELBUFFER_FMT_RGB4)  :  return V4L2_PIX_FMT_RGB32;
// RGB bayern
	case (PIXELBUFFER_FMT_BA81)  :  return V4L2_PIX_FMT_SBGGR8;
// YUV
	case (PIXELBUFFER_FMT_GREY)  :  return V4L2_PIX_FMT_GREY;
	case (PIXELBUFFER_FMT_YUYV)  :  return V4L2_PIX_FMT_YUYV;
	case (PIXELBUFFER_FMT_UYVY)  :  return V4L2_PIX_FMT_UYVY;
	case (PIXELBUFFER_FMT_Y41P)  :  return V4L2_PIX_FMT_Y41P;
	case (PIXELBUFFER_FMT_YV12)  :  return V4L2_PIX_FMT_YVU420;
	case (PIXELBUFFER_FMT_YU12)  :  return V4L2_PIX_FMT_YUV420;
	case (PIXELBUFFER_FMT_YVU9)  :  return V4L2_PIX_FMT_YVU410;
	case (PIXELBUFFER_FMT_YUV9)  :  return V4L2_PIX_FMT_YUV410;
	case (PIXELBUFFER_FMT_422P)  :  return V4L2_PIX_FMT_YUV422P;
	case (PIXELBUFFER_FMT_411P)  :  return V4L2_PIX_FMT_YUV411P;
	case (PIXELBUFFER_FMT_NV12)  :  return V4L2_PIX_FMT_NV12;
	case (PIXELBUFFER_FMT_NV21)  :  return V4L2_PIX_FMT_NV21;
	default : {
		// if we get here fmt has bad value and this should never happen
		// we return one of the formats wich wants more memory and cross fingers :)
		return V4L2_PIX_FMT_RGB24;
	}
	}
}


PixelBufferFormat v4l2_pix_fmt_to_pixelbuffer_fmt(unsigned int fmt) {
	switch (fmt) {
	case (V4L2_PIX_FMT_RGB332)   :  return PIXELBUFFER_FMT_RGB1;
	case (V4L2_PIX_FMT_RGB555)   :  return PIXELBUFFER_FMT_RGBO;
	case (V4L2_PIX_FMT_RGB565)   :  return PIXELBUFFER_FMT_RGBP;
	case (V4L2_PIX_FMT_RGB444)   :  return PIXELBUFFER_FMT_R444;
	case (V4L2_PIX_FMT_RGB555X)  :  return PIXELBUFFER_FMT_RGBQ;
	case (V4L2_PIX_FMT_RGB565X)  :  return PIXELBUFFER_FMT_RGBR;
	case (V4L2_PIX_FMT_BGR24)    :  return PIXELBUFFER_FMT_BGR3;
	case (V4L2_PIX_FMT_RGB24)    :  return PIXELBUFFER_FMT_RGB3;
	case (V4L2_PIX_FMT_BGR32)    :  return PIXELBUFFER_FMT_BGR4;
	case (V4L2_PIX_FMT_RGB32)    :  return PIXELBUFFER_FMT_RGB4;
// RGB bayern
	case (V4L2_PIX_FMT_SBGGR8)   :  return PIXELBUFFER_FMT_BA81;
// YUV
	case (V4L2_PIX_FMT_GREY)     :  return PIXELBUFFER_FMT_GREY;
	case (V4L2_PIX_FMT_YUYV)     :  return PIXELBUFFER_FMT_YUYV;
	case (V4L2_PIX_FMT_UYVY)     :  return PIXELBUFFER_FMT_UYVY;
	case (V4L2_PIX_FMT_Y41P)     :  return PIXELBUFFER_FMT_Y41P;
	case (V4L2_PIX_FMT_YVU420)   :  return PIXELBUFFER_FMT_YV12;
	case (V4L2_PIX_FMT_YUV420)   :  return PIXELBUFFER_FMT_YU12;
	case (V4L2_PIX_FMT_YVU410)   :  return PIXELBUFFER_FMT_YVU9;
	case (V4L2_PIX_FMT_YUV410)   :  return PIXELBUFFER_FMT_YUV9;
	case (V4L2_PIX_FMT_YUV422P)  :  return PIXELBUFFER_FMT_422P;
	case (V4L2_PIX_FMT_YUV411P)  :  return PIXELBUFFER_FMT_411P;
	case (V4L2_PIX_FMT_NV12)     :  return PIXELBUFFER_FMT_NV12;
	case (V4L2_PIX_FMT_NV21)     :  return PIXELBUFFER_FMT_NV21;
	default : {
		// if we get here fmt has bad value and this should never happen
		return PIXELBUFFER_FMT_NONE;
	}
	}
}





