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


#ifndef PixelBuffer_HH
#define PixelBuffer_HH

#include <sys/time.h>


// locks in PixelBuffer struct are used to lock the struct while any part of the program is working on it.
// For example when the image processor receives a PixelBuffer from a grabber rises flag PIXEL_BUFFER_INUSE_IMG_PROC
// to avoid the grabber writing new data on the buffer

// flags are stored in locks member of struct PixelBuffer: a 32bit unsigned int b(00000000'00000000'00000000'00000000')
// every bit represents a flag like:
// ie. PIXEL_BUFFER_INUSE_GRABBER	flag high means b(00000000'00000000'00000000'00000001')
// ie. PIXEL_BUFFER_INUSE_IMG_PROC	flag high means b(00000000'00000000'00000000'00000010')
// and so on... (than USE powers of 2!!!)

#define PIXEL_BUFFER_INUSE_GRABBER_Q	((unsigned int) 1 )
#define PIXEL_BUFFER_INUSE_IMG_PROC	((unsigned int) 1 << 1 )

#define PIXEL_BUFFER_INUSE_GRABBER_DESTROY ((unsigned int) 1 << 31 )

// sets the value of flag y for a PixelBuffer* passed as x
#define SETPIXELBUFFERFLAG(x,y) x->locks = x->locks | y

// clears the value of flag y for a PixelBuffer* passed as x
#define CLEARPIXELBUFFERFLAG(x,y) x->locks = ((x->locks) & (0xFFFFFFFF - y))

// returns true if any of 32 flags is set for a PixelBuffer* x
//  " " "  false if all 32 flags are 0
#define PIXELBUFFERISLOCKED(x) ( (x->locks) & 0xFFFFFFFF )


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// +++ this *must* be used only for debug purposes +++
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// gets the value of flag y for a PixelBuffer* x
// returns a bool
#define GETPIXELBUFFERFLAG(x,y) (x->locks & y)
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// completely reset a PixelBuffer struct passed as ptr x
#define PIXELBUFFERCLEARSTRUCT(x) x->buf    = NULL;	\
	x->length = 0;					\
	x->width  = mMaxWidth;				\
	x->height = mMaxHeight;				\
	x->locks  = (unsigned int) 0x00000000;		\
	x->fmt    = PIXELBUFFER_FMT_NONE;		\
	x->sec    = 0;					\
	x->usec   = 0; 

enum PixelBufferFormat {
	PIXELBUFFER_FMT_NONE,
	// RGB formats
	PIXELBUFFER_FMT_RGB1,
	PIXELBUFFER_FMT_RGBO,
	PIXELBUFFER_FMT_RGBP,
	PIXELBUFFER_FMT_R444,
	PIXELBUFFER_FMT_RGBQ,
	PIXELBUFFER_FMT_RGBR,
	PIXELBUFFER_FMT_BGR3,
	PIXELBUFFER_FMT_RGB3,
	PIXELBUFFER_FMT_BGR4,
	PIXELBUFFER_FMT_RGB4,
	// RGB bayern
	PIXELBUFFER_FMT_BA81,
	// YUV formats
	PIXELBUFFER_FMT_GREY,
	PIXELBUFFER_FMT_YUYV,
	PIXELBUFFER_FMT_UYVY,
	PIXELBUFFER_FMT_Y41P,
	PIXELBUFFER_FMT_YV12,
	PIXELBUFFER_FMT_YU12,
	PIXELBUFFER_FMT_YVU9,
	PIXELBUFFER_FMT_YUV9,
	PIXELBUFFER_FMT_422P,
	PIXELBUFFER_FMT_411P,
	PIXELBUFFER_FMT_NV12,
	PIXELBUFFER_FMT_NV21,
};

struct PixelBuffer {
	void* buf;			// a buffer of data (pixels data) long mLenght
	size_t length;		// pixel buffer lenght in bytes

	unsigned int width;		// pixel buffer width  (= mLenght/height)
	unsigned int height;		// pixel buffer height (= mLenght/width)

	unsigned int locks;		// flags used as locks

	PixelBufferFormat fmt;	// PixelBuffer format

	// image timestamp
	// ! valid only when sec > 0;
	time_t  sec;  		// seconds
	suseconds_t usec; 		// microseconds
};

#endif /*PixelBuffer_HH*/
