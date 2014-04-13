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


#include "V4L1_Device.hh"

static int xioctl (int fd, int request, void *arg) 
{
	int res;
	for (int i=0; i< V4L1_MAX_IOCTL_TIMES; i++) {
		res = ioctl (fd, request, arg);
		if ((res==-1) && (errno==EINTR)) continue;
		return res;
	}
}


V4L1_Device::V4L1_Device(GrabberInitData* initData) : Grabber(initData) {
	mMMAPSize = -1;
	mDevID = -1;						// reset class state
	CLEAR_V4L1DEV_FLAGS;
	mBuffersOrder.clear();		 		// avoid grabber to give away a bad PixelBuffer
	mMaxNumBuffers = initData->maxNumBuffers;
}


V4L1_Device::~V4L1_Device() {
	internal_reset();
}


bool V4L1_Device::set_ctrl_value(GrabberControlID id, short newValue) {
	if (mDevID<0) {
		V4L1DEV_WARNING("device not inited!\n");
		return false;
	}

	int pos=find_ctrl_index(id);
	if (pos==-1) return false;
	switch (id) {
	case (GRABBER_CTRL_BRIGHTNESS)  :  { mPicture.brightness = (__u32) newValue; break; }
	case (GRABBER_CTRL_HUE)	    :  { mPicture.hue = (__u32) newValue; break; }
	case (GRABBER_CTRL_SATURATION)  :  { mPicture.colour = (__u32) newValue; break; }
	case (GRABBER_CTRL_CONTRAST)    :  { mPicture.contrast = (__u32) newValue; break; }
	default : { return false; }
	}
	if (xioctl( mDevID, VIDIOCSPICT, &mPicture)== -1 ) {
		V4L1DEV_WARNING("VIDIOCSPICT failed\n");
		return false;
	}
	mGrabberControls[pos]->value= (__u32) newValue;
	return true;
}


bool V4L1_Device::init() {
	if(mDevID!=-1) {	// if dev was already opened completely reset grabber state
		internal_reset ();
		V4L1DEV_WARNING("device re-init\n");
	}

	// open video device
#ifdef V4L1_Device_Verbose
	std::cout << "\nOpening v4l1 device: " << mPathToDev << " ...\n";
#endif
	mDevID = open( mPathToDev.c_str(), O_RDONLY);		// open in read only mode
	if(mDevID == -1) {
		V4L1DEV_WARNING("cannot open device\n");
		return false;
	}

	// get device capabilities
	// on fatal error reset grabber state
	if (!internal_get_device_capabilities()) {
		internal_reset();
		return false;
	}

	/* Retrieve picture data */
#ifdef V4L1_Device_Verbose
	std::cout << "\nRetrieving picture data ...\n";
#endif
	internal_get_picture_data();

	// Set up best IO method
	// try first with mmap, if it doesn't succeds than try with read()
	if (! (internal_setup_io_MMAP()) ) {
		mBuffersOrder.clear();
		internal_setup_io_READ ();
	}
	// check if any IO method was set
	if (!(GET_V4L1DEV_FLAG(VIDEO_CAPTURE_USING_MMAP) | GET_V4L1DEV_FLAG(VIDEO_CAPTURE_USING_READWRITE)) ) {
		V4L1DEV_CRITICAL("no IO methods available\n");
		internal_reset();
		return false;   
	}
    
#ifdef V4L1_Device_Verbose
	std::cout << "\nIO Streaming method used: ";
	if (GET_V4L1DEV_FLAG(VIDEO_CAPTURE_USING_MMAP)) std::cout << "MMAP\n";
	else if (GET_V4L1DEV_FLAG(VIDEO_CAPTURE_USING_READWRITE)) std::cout << "READ/WRITE\n";
#endif

	/* device inited successfully! */
	return true;
}


void V4L1_Device::grab() {
	if (mDevID<0) {	// check if this grabber was inited
		V4L1DEV_WARNING("device not inited!\n");
		return;
	}
	// search a buffer without locks
	unsigned int pos = -1;
	for (unsigned int index = 0; index < mPixelBuffers.size(); index++) {
		if ( ! PIXELBUFFERISLOCKED(mPixelBuffers[index]) ) {			// if PixelBuffer has some locks than don't touch it
			pos = index;
			SETPIXELBUFFERFLAG(mPixelBuffers[pos],PIXEL_BUFFER_INUSE_GRABBER_Q);
			break;
		}
	}
	if (pos == -1)  { // if we get here or mPixelBuffers.size()==0 or no buffer with no lock was available
		V4L1DEV_CRITICAL("no buffers available\n");
		return;  
	}

	/*** MMAP STREAMING ***/
	if (GET_V4L1DEV_FLAG(VIDEO_CAPTURE_USING_MMAP)) {
		mVMMAP.frame  = pos;
		mVMMAP.format = mPicture.palette;
		mVMMAP.width  = mMaxWidth;
		mVMMAP.height = mMaxHeight;

		if (xioctl( mDevID, VIDIOCMCAPTURE, &mVMMAP) <0) {
			V4L1DEV_WARNING("VIDIOCMCAPTURE failed\n");
			CLEARPIXELBUFFERFLAG(mPixelBuffers[pos],PIXEL_BUFFER_INUSE_GRABBER_Q);
			return;
		} 

		if (xioctl( mDevID, VIDIOCSYNC, &pos) < 0) {
			V4L1DEV_WARNING("VIDIOCSYNC failed\n");
			CLEARPIXELBUFFERFLAG(mPixelBuffers[pos],PIXEL_BUFFER_INUSE_GRABBER_Q);
			return;
		} 
		mBuffersOrder.push_front(pos);						// say to the grabber what is the actual PixelBuffer
		CLEARPIXELBUFFERFLAG(mPixelBuffers[pos],PIXEL_BUFFER_INUSE_GRABBER_Q);
		mBuffersOrder.pop_back();						// so that mBuffersOrder.size() is never > mPixelBuffers.size()
		mPixelBuffers[pos]->sec = 0;						// set timestamp
		mPixelBuffers[pos]->usec = 0;
  
		return;
	}

	/*** READ/WRITE STREAMING ***/
	else if (GET_V4L1DEV_FLAG(VIDEO_CAPTURE_USING_READWRITE)) {
		if (read(mDevID, mPixelBuffers[pos]->buf, mPixelBuffers[pos]->length) <0) {
			CLEARPIXELBUFFERFLAG(mPixelBuffers[pos],PIXEL_BUFFER_INUSE_GRABBER_Q);
			V4L1DEV_WARNING("read() error\n");
			return;
		}
		mBuffersOrder.push_front(pos);						// say to the grabber what is the actual PixelBuffer
		CLEARPIXELBUFFERFLAG(mPixelBuffers[pos],PIXEL_BUFFER_INUSE_GRABBER_Q);
		mBuffersOrder.pop_back();						// so that mBuffersOrder.size() is never > mPixelBuffers.size()
		return;
	}
	// if we get here this is very bad ...
	assert(0);
}


bool V4L1_Device::internal_setup_io_MMAP () {
	if (mDevID<0) {	// if device is not yet open
		V4L1DEV_WARNING("device not inited!\n");
		return false;
	}

	if (!GET_V4L1DEV_FLAG(VIDEO_CAPTURE_SUPPORT) ){	// device doesn't support cpature
		V4L1DEV_WARNING("no capture support!\n");
		return false;
	}

	struct video_mbuf mBuf;
	memset (&mBuf, 0 , sizeof(video_mbuf));	

	if (xioctl( mDevID, VIDIOCGMBUF, &mBuf)<0) {
		V4L1DEV_WARNING("VIDIOCGMBUF failed\n");
		return false;
	}

	if (mBuf.frames < V4L1_MIN_NUM_BUFFERS) {
		V4L1DEV_WARNING("couldn't allocate minimum necessary buffers\n");
		return false;
	}

	SET_V4L1DEV_FLAG(VIDEO_CAPTURE_USING_MMAP);			// set flag: if mmap fails, internal_reset will know that unmap is needed

	while (mPixelBuffers.size()!= mBuf.frames) {			// push reqBufs.count new PixelBuffers in vector mPixelBuffers
		PixelBuffer* newBuf = new PixelBuffer();
		if (newBuf==NULL) {
			V4L1DEV_CRITICAL("out of memory\n");
			return false;
		}
		PIXELBUFFERCLEARSTRUCT(newBuf);				// reset PixelBuffer struct
		newBuf->width = mMaxWidth;
		newBuf->height = mMaxHeight;
		mPixelBuffers.push_back(newBuf);			// push new buffer in the vector
		mBuffersOrder.push_front(-1);				// make mBuffersOrder.size() = mPixelBuffers.size()
		// -1 means that PixelBuffers are not yet valid
	}

	// mmap and assing start of mmapped buffer to the first PixelBuffer
	mPixelBuffers[0]->buf = (unsigned char*)mmap(NULL, mBuf.size, PROT_READ|PROT_WRITE, MAP_SHARED, mDevID, 0);
	if (mPixelBuffers[0]->buf == MAP_FAILED) {
		CLEAR_V4L1DEV_FLAG(VIDEO_CAPTURE_USING_MMAP);
		while (mPixelBuffers.size()!=0)  {
			mPixelBuffers.erase(mPixelBuffers.begin());		// erase all entries in mPixelBuffers
		}
		V4L1DEV_WARNING("mmap failed\n");
		return false;
	}
	mMMAPSize = mBuf.size;
	// assign the different frames at offset mBuf.offset[frame_num] to the mPixelBuffers
	for (unsigned int bufIndex = 1; bufIndex < mBuf.frames; bufIndex++) {
		mPixelBuffers[bufIndex]->length = pixelbuffer_length ( v4l1_palette_to_pixelbuffer_fmt(mPicture.palette),
					       mMaxWidth, mMaxHeight );	// driver allocates memory in respect to the format
		mPixelBuffers[bufIndex]->buf = (void*) ((int)mPixelBuffers[0]->buf + mBuf.offsets[bufIndex]);
	}      
	return true;
}


bool V4L1_Device::internal_setup_io_READ () {
	if (mDevID<0) {	// if device is not yet open
		V4L1DEV_WARNING("device not inited!\n");
		return false;
	}

	SET_V4L1DEV_FLAG(VIDEO_CAPTURE_USING_READWRITE);			// set that we use this method
	// create the desidered number of PixelBuffers (in read write few buffers are necessary)
	while (mPixelBuffers.size()!= mMaxNumBuffers) {			
		PixelBuffer* newBuf = new PixelBuffer();
		if (newBuf==NULL) {
			V4L1DEV_CRITICAL("out of memory\n");
			return false;
		}
		PIXELBUFFERCLEARSTRUCT(newBuf);					// reset PixelBuffer struct
		newBuf->width = mMaxWidth;
		newBuf->height = mMaxHeight;
		newBuf->length =  pixelbuffer_length ( v4l1_palette_to_pixelbuffer_fmt(mPicture.palette), mMaxWidth, mMaxHeight);
		newBuf->buf = (void*) malloc( newBuf->length );			// malloc memory in respect to format, width and height
		if (newBuf->buf==NULL) {
			V4L1DEV_CRITICAL("out of memory\n");
			return false;
		}
		mPixelBuffers.push_back(newBuf);					// push new buffer in the vector
		mBuffersOrder.push_front(-1);					// make mBuffersOrder.size() = mPixelBuffers.size()
		// -1 means that PixelBuffers are not yet valid
	}
	return true;
}



void V4L1_Device::internal_reset () {
	mBuffersOrder.clear();		 // avoid grabber to give away a bad PixelBuffer

	if(mDevID>-1) {			// if video device was opened...
		// wait to acquire exclusive lock over PixelBuffers
		for (int i=0; i< mPixelBuffers.size(); i++) {
			assert(mPixelBuffers[i]);
			while (PIXELBUFFERISLOCKED(mPixelBuffers[i])) {usleep(100);}			// if something is still working on pixbuf we must wait
			SETPIXELBUFFERFLAG(mPixelBuffers[i],PIXEL_BUFFER_INUSE_GRABBER_DESTROY);
		}

		// free memory of mPixelBuffers
		if (GET_V4L1DEV_FLAG(VIDEO_CAPTURE_USING_MMAP)) {					// mmap streaming case
			if (mPixelBuffers[0]->buf != MAP_FAILED)
				if (mMMAPSize>-1)
					munmap (mPixelBuffers[0]->buf, mMMAPSize);	// unmap memory
		}
		else if (GET_V4L1DEV_FLAG(VIDEO_CAPTURE_USING_READWRITE)) {			// read()/write() streaming case
			for (int i=0; i< mPixelBuffers.size(); i++) {
				if (mPixelBuffers[i]->buf != NULL)  free (mPixelBuffers[i]->buf);	// free memory
				delete mPixelBuffers[i];
			}
		}
		while (mPixelBuffers.size()!=0) mPixelBuffers.erase(mPixelBuffers.begin());	// erase all entries in mPixelBuffers

		close(mDevID);								// close device
		mDevID = -1;								// reset device fd value
	}

	// free memory for vector mGrabberControls
	for (int i=0; i< mGrabberControls.size(); i++) delete mGrabberControls[i];
	while (mGrabberControls.size()!=0) mGrabberControls.erase(mGrabberControls.begin());

	memset (&mPicture, 0 , sizeof(video_picture));	
	CLEAR_V4L1DEV_FLAGS;						// reset flags
	mMMAPSize = -1;
}



bool V4L1_Device::internal_get_device_capabilities () {
	if (mDevID<0) {	// if device is not yet open
		V4L1DEV_WARNING("device not inited!\n");
		return false;
	}

	video_capability mCapability;
	memset (&mCapability, 0 , sizeof(video_capability));
	if (xioctl( mDevID, VIDIOCGCAP, &mCapability)== -1 ) {
		V4L1DEV_WARNING("VIDIOCGCAP failed\n");
		return false;
	}

#ifdef V4L1_Device_Verbose
	std::cout << "\nRetrieving device capabilities ...\n" 
		  << v4l1_capabilities_to_string(mCapability);
#endif

	// check for capture capablities
	if ((mCapability.type & VID_TYPE_CAPTURE)) SET_V4L1DEV_FLAG(VIDEO_CAPTURE_SUPPORT);
	else V4L1DEV_WARNING("no capture support\n");
	return true;
}



void V4L1_Device::internal_get_picture_data () {
	if (mDevID<0) {	// if device is not yet open
		V4L1DEV_WARNING("device not inited!\n");
		return;
	}

	memset (&mPicture, 0 , sizeof(video_picture));
	if (xioctl( mDevID, VIDIOCGPICT, &mPicture)== -1 ) {
		V4L1DEV_WARNING("VIDIOCGPICT failed\n");
		return;
	}

	GrabberControlData* ctrlData = NULL;
	// ! by default in v4l1 All values except for the palette type are scaled between 0-65535.
	// add brightness control
	ctrlData = new GrabberControlData();	
	ctrlData->ID = GRABBER_CTRL_BRIGHTNESS;
	ctrlData->min  = 0;
	ctrlData->max  = 65535;
	ctrlData->step = 1;
	ctrlData->def  = (int) mPicture.brightness;
	ctrlData->value  = (int) mPicture.brightness;
	mGrabberControls.push_back(ctrlData);
#ifdef V4L1_Device_Verbose
	std::cout << " - ctrls: " << grabber_ctrl_id_to_string(GRABBER_CTRL_BRIGHTNESS) << "*";
#endif

	// add hue control
	ctrlData = new GrabberControlData();
	ctrlData->ID = GRABBER_CTRL_HUE;
	ctrlData->min  = 0;
	ctrlData->max  = 65535;
	ctrlData->step = 1;
	ctrlData->def  = (int) mPicture.hue;
	ctrlData->value  = (int) mPicture.hue;
	mGrabberControls.push_back(ctrlData);
#ifdef V4L1_Device_Verbose
	std::cout << grabber_ctrl_id_to_string(GRABBER_CTRL_HUE) << "*";
#endif

	// add saturation control
	ctrlData = new GrabberControlData();
	ctrlData->ID = GRABBER_CTRL_SATURATION;
	ctrlData->min  = 0;
	ctrlData->max  = 65535;
	ctrlData->step = 1;
	ctrlData->def  = (int) mPicture.colour;
	ctrlData->value  = (int) mPicture.colour;
	mGrabberControls.push_back(ctrlData);
#ifdef V4L1_Device_Verbose
	std::cout << grabber_ctrl_id_to_string(GRABBER_CTRL_SATURATION) << "*";
#endif

	// add contrast control
	ctrlData = new GrabberControlData();
	ctrlData->ID = GRABBER_CTRL_CONTRAST;
	ctrlData->min  = 0;
	ctrlData->max  = 65535;
	ctrlData->step = 1;
	ctrlData->def  = (int) mPicture.contrast;
	ctrlData->value  = (int) mPicture.contrast;
	mGrabberControls.push_back(ctrlData);
#ifdef V4L1_Device_Verbose
	std::cout << grabber_ctrl_id_to_string(GRABBER_CTRL_CONTRAST) << "*\n";
#endif

#ifdef V4L1_Device_Verbose
	std::cout << " - palette: "
		  << pixelbuffer_fmt_to_string(v4l1_palette_to_pixelbuffer_fmt(mPicture.palette)) << "\n";
#endif
	return;
}


PixelBufferFormat V4L1_Device::get_format() {
	if (mDevID==-1) return PIXELBUFFER_FMT_NONE;
	return v4l1_palette_to_pixelbuffer_fmt(mPicture.palette);
}


bool V4L1_Device::get_crop(CropData &cas) {
	if (mDevID==-1) return false;

	/* get actual settings */
	video_window crop;
	memset (&crop, 0 , sizeof(video_window));
	if ( xioctl(mDevID, VIDIOCGWIN, &crop) == -1) {
		V4L1DEV_WARNING("VIDIOCGWIN failed\n");
		return false;
	}
	/* copy values */
	crop.x = cas.left;
	crop.y = cas.top;
	crop.width = cas.width;
	crop.height = cas.height;
	return true;
}


bool V4L1_Device::set_crop(CropData &cas) {
	if (mDevID==-1) return false;

	/* get actual settings */
	video_window crop;
	memset (&crop, 0 , sizeof(video_window));
	if ( xioctl(mDevID, VIDIOCGWIN, &crop) == -1) {
		V4L1DEV_WARNING("VIDIOCGWIN failed\n");
		return false;
	}

	/* set new values */
	crop.x = cas.left;
	crop.y = cas.top;
	crop.width = cas.width;
	crop.height = cas.height;
	if ( xioctl(mDevID, VIDIOCSWIN, &crop) == -1) {
		V4L1DEV_WARNING("VIDIOCSWIN failed\n");
		return false;
	}

	/* driver may change some values against hardware capabilities */
	if ( xioctl(mDevID, VIDIOCGWIN, &crop) == -1) {
		V4L1DEV_WARNING("VIDIOCGWIN failed\n");
		return false;
	}
	cas.left = crop.x;
	cas.top = crop.y;
	cas.width = crop.width;
	cas.height = crop.height;
	return true;
}

