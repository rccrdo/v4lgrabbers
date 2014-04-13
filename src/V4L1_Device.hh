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


#ifndef V4L1_Device_HH
#define V4L1_Device_HH

extern "C" {
#include <linux/videodev.h>
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
#include "V4L1_Helpers.hh"
#include "CropData.hh"


// flags in V4L1_Device are used instead of many booleans; they are stored in mInternalFlags
// wich is a 32 bit unsigned int: b(00000000'00000000'00000000'00000000')
// every bit is flag as follows:
// ie. VIDEO_CAPTURE_CROPSCALE 		flag high means b(00000000'00000000'00000000'00000001')
// ie. VIDEO_CAPTURE_USING_READWRITE	flag high means b(10000000'00000000'00000000'00000000')
// and so on... (than USE powers of 2!!!)
#define VIDEO_CAPTURE_CROPSCALE      ((unsigned int) 1 )
#define VIDEO_CAPTURE_SUPPORT      ((unsigned int) 2 )
#define VIDEO_CAPTURE_USING_MMAP ((unsigned int) 1 << 29)
#define VIDEO_CAPTURE_USING_READWRITE ((unsigned int) 1 << 31)

// !!! inside the class, to set and get a particlar flag value use these macros !!!
// sets to 1 the flag passed as x
#define SET_V4L1DEV_FLAG(x) mInternalFlags = (mInternalFlags | x )
// ie. to set flag VIDEO_CAPTURE_READWRITE use SET_V4L1DEV_FLAG (VIDEO_CAPTURE_READWRITE)

// gets the flag value of position passed as x (returns a bool! or int [0,1] if you prefer...]
#define GET_V4L1DEV_FLAG(x) (mInternalFlags & x)
// ie. to get flag VIDEO_CAPTURE_STREAMING value use GET_V4L1DEV_FLAG (VIDEO_CAPTURE_STREAMING)

// clears value of flag x
#define CLEAR_V4L1DEV_FLAG(x) mInternalFlags = ( mInternalFlags & (0xFFFFFFFF - x))

// !!! this must be used only in constructor and internal_reset() !!!
// clear all flags to 0s
#define CLEAR_V4L1DEV_FLAGS mInternalFlags = 0x00000000


// min number of buffers used for streaming
// if the driver cannot allocate at least V4L1_MINNUMBUFFERS buffers than initing the driver will fail
#define V4L1_MIN_NUM_BUFFERS 2

// max number of times we iterate in our custom xioctl
// [sometimes ioctl can return -1 when some interrupt occurs, as this isn't an error
//  we try V4L1_MAX_IOCTL_TIMES to see if we can get something usefull from ioctl]
#define V4L1_MAX_IOCTL_TIMES 5


// V4L1 device logging helpers
#define V4L1DEV_DEBUG_PREFIX	(" * DEBUG - v4l1_dev - ")
#define V4L1DEV_NOTICE_PREFIX	(" * NOTICE - v4l1_dev - ")
#define V4L1DEV_WARNING_PREFIX	(" * WARNING - v4l1_dev - ")
#define V4L1DEV_CRITICAL_PREFIX	(" * !!! CRITICAL !!! - V4L1_dev - ")

// Used to be implemented using ACE helpers - need to switch to something else
#define V4L1DEV_DEBUG(x) {}
#define V4L1DEV_NOTICE(x) {}
#define V4L1DEV_WARNING(x) {}
#define V4L1DEV_CRITICAL(x) {}


static int xioctl (int fd, int request, void *arg);

class V4L1_Device : public Grabber
{
public:
	V4L1_Device(GrabberInitData* devData);
	~V4L1_Device();

	bool set_ctrl_value(GrabberControlID id, short newValue);
	bool init(void);
	void grab(void);

	bool set_crop(CropData &cas);
	bool get_crop(CropData &cas);
	PixelBufferFormat get_format(void);

private:
	// returns false when mmap IO cannot be used
	bool internal_setup_io_MMAP (void);
	// returns false when read/write IO cannot be used
	bool internal_setup_io_READ (void);

	// reset completely device and this class
	void internal_reset(void);

	// get device capabilities
	bool internal_get_device_capabilities(void);  

	// get picture data and add v4l1 supported controls to the vector mGrabberControls
	void internal_get_picture_data (void);

	video_mmap mVMMAP;				// used to retrieve mmapped memory from the driver
	int mMMAPSize;

	unsigned char mMaxNumBuffers;			// number of buffers used in streaming mode

	int mDevID;					// V4L1 device id
	unsigned int mInternalFlags;			// capabilities flags, see beginning of this file

	video_picture mPicture;			// used to retrieve v4l1 picture data
  
};

#endif /*V4L1_Device_HH*/
