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

#ifndef Grabber_HH
#define Grabber_HH

#include <iostream>
#include <vector>
#include <list>

#include "Debug.hh"
#include "PixelBuffer.hh"
#include "CropData.hh"
#include "GrabberControlData.hh"
#include "GrabberInitData.hh"


#define GRABBER_WARNING_PREFIX	(" * WARNING - Grabber - ")

// Used to be implemented using ACE helpers - need to switch to something else
#define GRABBER_WARNING(x)

class Grabber {
public:
	Grabber(GrabberInitData* initData);
	~Grabber();

	// get ctrl datas for a control with id GrabberControlID
	const GrabberControlData* get_ctrl_data (GrabberControlID id);
	// get the value for the ctrl with id GrabberControlID
	int get_ctrl_value(GrabberControlID id);

	// grab a picture
	// when grab is successful mActualBuffer's value is the index in mPixelBuffers of last grabbed PixelBuffer
	virtual void grab(void) = 0;

	// init device
	virtual bool init(void) = 0;

	PixelBuffer* get_last_grabbed(void);

	// set value for ctrl with id GrabberControlID
	// returns false when request doesn't succed (this may happen when some kernel events rise for example or crls isn't supported)
	virtual bool set_ctrl_value(GrabberControlID id, short newValue) = 0;

	// set crop and scale options
	// hw may not support all crop options and possibilities
	// if request succeds than true is returned
	// else returns false and in <cas> are saved the crop values that the driver set (as near as possible to requested ones...)
	virtual bool set_crop(CropData &cas) = 0;

	// get actual crop and scale options
	// returns false if device couldn't make request
	virtual bool get_crop(CropData &cas) = 0;

	// get actual format
	virtual PixelBufferFormat get_format(void) = 0;

protected:
	int find_ctrl_index(GrabberControlID id);//returns the index of ctrl with GrabberControlID -id- if found; else returns -1


	//
	// members
	//
	std::vector <PixelBuffer*> mPixelBuffers;	// PixelBuffers used for streaming and exchanged with the image processor ecc...
	// mPixelBuffers.size() returns the number of buffers used

	std::list<int> mBuffersOrder;			// Inherited classes fill mPixelBuffers but not always in "linear order": 0-1-2-3 
	// ie. V4L2 mmap method doesn't
	// inherited classes fill this list so that head element is
	// last grabbed pixbuf index and tail element is the index of the oldest one.

	std::list<int>::const_iterator pbIter;	// an iterator for mBuffersOrder

	std::vector <GrabberControlData*> mGrabberControls;
	std::string mPathToDev;		// path to device: ie. /dev/video0
	unsigned int mMaxWidth;		// max image's width for this grabber
	unsigned int mMaxHeight;	// max image's height for this grabber
};

#endif /*Grabber_HH*/
