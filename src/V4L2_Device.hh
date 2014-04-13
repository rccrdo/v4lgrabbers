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


#ifndef V4L2_Device_HH
#define V4L2_Device_HH

extern "C" {
#include <linux/videodev2.h>
}

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <iostream>
#include <list>

#include "Debug.hh"

#include "Defaults.hh"
#include "Grabber.hh"
#include "GrabberInitData.hh"
#include "GrabberControlData.hh"
#include "Grabber_Helpers.hh"
#include "V4L2_Helpers.hh"
#include "CropData.hh"


// flags in V4L2_Device are used instead of many booleans; they are stored in mInternalFlags
// wich is a 32 bit unsigned int: b(00000000'00000000'00000000'00000000')
// every bit is flag as follows:
// ie. VIDEO_CAPTURE_READWRITE 		flag high means b(00000000'00000000'00000000'00000001')
// ie. VIDEO_CAPTURE_STREAMING_MMAP	flag high means b(00000000'00000000'00000000'00100000')
// and so on... (than USE powers of 2!!!)
#define VIDEO_CAPTURE_CROPSCALE      ((unsigned int) 1 )
#define VIDEO_CAPTURE_READWRITE      ((unsigned int) 1 << 1 )
#define VIDEO_CAPTURE_STREAMING      ((unsigned int) 1 << 2)
#define VIDEO_CAPTURE_STREAMING_PTRS ((unsigned int) 1 << 3)
#define VIDEO_CAPTURE_STREAMING_MMAP ((unsigned int) 1 << 4)
#define VIDEO_CAPTURE_HAS_FRAME_SKIPPING_SUPPORT ((unsigned int) 1 << 5)
#define VIDEO_CAPTURE_HAS_HIGHQ_STILLIMAGE_SUPPORT ((unsigned int) 1 << 6)

#define VIDEO_CAPTURE_USING_STREAMING_MMAP ((unsigned int) 1 << 29)
#define VIDEO_CAPTURE_USING_STREAMING_PTRS ((unsigned int) 1 << 30)
#define VIDEO_CAPTURE_USING_READWRITE ((unsigned int) 1 << 31)

// !!! inside the class, to set and get a particlar flag value use these macros !!!
// sets to 1 the flag passed as x
#define SET_V4L2DEV_FLAG(x) mInternalFlags = (mInternalFlags | x )
// ie. to set flag VIDEO_CAPTURE_READWRITE use SET_V4L2DEV_FLAG (VIDEO_CAPTURE_READWRITE)

// gets the flag value of position passed as x (returns a bool! or int [0,1] if you prefer...]
#define GET_V4L2DEV_FLAG(x) (mInternalFlags & x)
// ie. to get flag VIDEO_CAPTURE_STREAMING value use GET_V4L2DEV_FLAG (VIDEO_CAPTURE_STREAMING)

// clears value of flag x
#define CLEAR_V4L2DEV_FLAG(x) mInternalFlags = ( mInternalFlags & (0xFFFFFFFF - x))

// !!! this must be used only in constructor and internal_reset() !!!
// clear all flags to 0s
#define CLEAR_V4L2DEV_FLAGS mInternalFlags = 0x00000000


// min number of buffers used for streaming
// if the driver cannot allocate at least V4L2_MINNUMBUFFERS buffers than initing the driver will fail
#define V4L2_MIN_NUM_BUFFERS 2

// max number of times we iterate in our custom xioctl
// [sometimes ioctl can return -1 when some interrupt occurs, as this isn't an error
//  we try V4L2_MAX_IOCTL_TIMES to see if we can get something usefull from ioctl]
#define V4L2_MAX_IOCTL_TIMES 5


// V4L2 device logging helpers
#define V4L2DEV_DEBUG_PREFIX	(" * DEBUG - v4l2_dev - ")
#define V4L2DEV_NOTICE_PREFIX	(" * NOTICE - v4l2_dev - ")
#define V4L2DEV_WARNING_PREFIX	(" * WARNING - v4l2_dev - ")
#define V4L2DEV_CRITICAL_PREFIX	(" * !!! CRITICAL !!! - v4l2_dev - ")

// Used to be implemented using ACE helpers - need to switch to something else
#define V4L2DEV_DEBUG(x) {}
#define V4L2DEV_NOTICE(x) {}
#define V4L2DEV_WARNING(x) {}
#define V4L2DEV_CRITICAL(x) {}


static int xioctl (int fd, int request, void *arg);

class V4L2_Device : public Grabber
{
public:
	V4L2_Device(GrabberInitData* devData);
	~V4L2_Device();

	bool set_ctrl_value(GrabberControlID id, short newValue);
	bool init(void);
	void grab(void);

	bool set_crop(CropData &cas);
	bool get_crop(CropData &cas);
	PixelBufferFormat get_format(void);

private:
	// set image format
	bool internal_set_format(PixelBufferFormat fmt);

	// set IO method: prefer streaming-mmap on streaming-ptrs and as last try simple read()/write()
	// returns false when no IO method could be used (wich should never happen)
	bool internal_setup_io_MMAP (void);  
	bool internal_setup_io_PTRS (void);  
	bool internal_setup_io_READ (void);  

	void internal_free_pixbufs_mem (void);


	// reset completely device and this class
	void internal_reset(void);

	// get device capabilities and sets mInternalFlags
	// if capabilities request is not supported returns false
	bool internal_get_device_capabilities (void);  

	// get Crop and Scale capabilities and sets mMaxWidth and mMaxHeight vars
	// if Crop and Scale is not supported returns false
	bool internal_get_cropscale_capabilities (void);  

	// check if a video control is supported by the device
	// if it is adds a GrabberControlData struct to the vector mGrabberControls
	void internal_addCtrlIfAny (unsigned int V4L2_ctrlID);

	bool internal_get_streaming_params (void);  

	bool internal_get_image_format (void);  

	// activates/deactivates streaming in respect to bool activate
	// return false when op doesn't succed
	bool internal_activate_streaming (bool activate);  


	unsigned char mMaxNumBuffers;			// number of buffers used in streaming mode
	v4l2_buffer mV4L2Buf;				// v4l2 buffer used for streaming
	v4l2_control mV4L2Ctrl;			// used to change controls values without need of malloc everytime

	int mDevID;					// V4L2 device id
	unsigned int mInternalFlags;			// capabilities flags, see beginning of this file

	v4l2_cropcap mCropScaleCap;			// used to retrieve crop and scale capabilities
	v4l2_format mImageFormat;			// used to retrieve and switch drivers image format

  
	float mStreamFreq;			// used (maybe) in some streaming tecniches

};

#endif /*V4L2_Device_HH*/
