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


#include "V4L2_Device.hh"

static int xioctl (int fd, int request, void *arg) 
{
	int res;
	for (int i=0; i< V4L2_MAX_IOCTL_TIMES; i++) {
		res = ioctl (fd, request, arg);
		if ((res==-1) && (errno==EINTR)) continue;
		return res;
	}
}


V4L2_Device::V4L2_Device(GrabberInitData* initData) : Grabber(initData) {
	mDevID = -1;							// reset class state
	mStreamFreq = -1.0f;
	CLEAR_V4L2DEV_FLAGS;
	memset (&mImageFormat, 0 , sizeof(v4l2_format));
	memset (&mV4L2Buf, 0, sizeof(v4l2_buffer));
	mBuffersOrder.clear();			 		// avoid grabber to give away a bad PixelBuffer
	mMaxNumBuffers = initData->maxNumBuffers;
}


V4L2_Device::~V4L2_Device() {
	internal_reset();						// reset device state and free all memory on the heap
}	


bool V4L2_Device::set_ctrl_value(GrabberControlID id, short newValue) {
	if (mDevID<0) {	// if device is not yet open
		V4L2DEV_WARNING ("device not inited!\n");
		return false;
	}
	int pos=find_ctrl_index(id);
	if (pos==-1) return false;

	memset (&mV4L2Ctrl, 0, sizeof(v4l2_control));				// reset struct to 0s
	mV4L2Ctrl.id = grabber_ctrl_id_to_v4l2_ctrl_id (id);
	mV4L2Ctrl.value = newValue;

	if ( xioctl( mDevID, VIDIOC_S_CTRL, &mV4L2Ctrl) == -1) {
		V4L2DEV_WARNING ("VIDIOC_S_CTRL failed\n");
		return false;
	}

	mGrabberControls[pos]->value=newValue;
	return true;
}


bool V4L2_Device::init() {

	if(mDevID!=-1) {	// if dev was already opened completely reset grabber state
		internal_reset ();
		V4L2DEV_WARNING("device re-init\n");
	}

	// open video device
#ifdef V4L2_Device_Verbose
	std::cout << "\nOpening v4l2 device: " << mPathToDev << " ...\n";
#endif
	mDevID = open( mPathToDev.c_str(), O_RDONLY);		// open in read only mode
	if(mDevID == -1) {
		V4L2DEV_WARNING("cannot open device\n");
		return false;
	}

	// get device capabilities
	// on fatal error reset grabber state
	if (! (internal_get_device_capabilities()) ) {
		internal_reset();
		return false;
	}

	// get crop and scale capabilities
	// on fatal error reset grabber state
	if (! (internal_get_cropscale_capabilities()) ) {
		internal_reset();
		return false;
	}
    
	// Retrieving camera video controls data
#ifdef V4L2_Device_Verbose
	std::cout << "\nRetrieving ctrls data ... ";
#endif
	internal_addCtrlIfAny(V4L2_CID_BRIGHTNESS);
	internal_addCtrlIfAny(V4L2_CID_CONTRAST);
	internal_addCtrlIfAny(V4L2_CID_SATURATION);
	internal_addCtrlIfAny(V4L2_CID_HUE);
	internal_addCtrlIfAny(V4L2_CID_AUTO_WHITE_BALANCE);
	internal_addCtrlIfAny(V4L2_CID_RED_BALANCE);
	internal_addCtrlIfAny(V4L2_CID_BLUE_BALANCE);
	internal_addCtrlIfAny(V4L2_CID_GAMMA);
	internal_addCtrlIfAny(V4L2_CID_EXPOSURE);
	internal_addCtrlIfAny(V4L2_CID_AUTOGAIN);
	internal_addCtrlIfAny(V4L2_CID_GAIN);
	internal_addCtrlIfAny(V4L2_CID_HFLIP);
	internal_addCtrlIfAny(V4L2_CID_VFLIP);
#ifdef V4L2_Device_Verbose
	std::cout << "\n";
#endif

	// Retrieving camera image format
	if (! (internal_get_image_format()) ) {
		internal_reset();
		return false;
	}

	// Set up best IO method
	if (!internal_setup_io_MMAP ()) {
		internal_free_pixbufs_mem();
		if (!internal_setup_io_PTRS ()) {
			internal_free_pixbufs_mem();
			internal_setup_io_READ ();
		}
	}  
	// check if one io method was selected
	if (!(GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_MMAP)
	      | GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_PTRS)
	      | GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_READWRITE)) ) {
		V4L2DEV_CRITICAL("no IO methods available\n");
		internal_reset();
		return false;   
	}

#ifdef V4L2_Device_Verbose
	std::cout << "\nIO Streaming method used: ";
	if (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_MMAP)) std::cout << "MMAP\n";
	else if (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_PTRS)) std::cout << "PTRS\n";
	else if (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_READWRITE)) std::cout << "READ/WRITE\n";
#endif

	// Retrieving streaming parameters: on fatal error reset grabber state (to non-inited) and return from init()
	if (! (internal_get_streaming_params()) ) {
		internal_reset();
		return false;
	}

	// activate streaming
	// on fatal error reset grabber state
	if (! (internal_activate_streaming(true)) ) {
		internal_reset();
		return false;
	}

	/* device inited successfully! */
	return true;
}



void V4L2_Device::grab() {

	if (mDevID<0) {	// check if this grabber was inited
		V4L2DEV_WARNING("device not inited!\n");
		return;
	}

/*** MMAP STREAMING ***/
	if (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_MMAP)) {
		// queue buffers
		for (unsigned int i = 0; i < mPixelBuffers.size(); i++) {
			if ( PIXELBUFFERISLOCKED(mPixelBuffers[i]) ) continue;			// if PixelBuffer has some locks than don't touch it
			memset (&mV4L2Buf, 0, sizeof(v4l2_buffer));				// reset struct to 0s
			mV4L2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			mV4L2Buf.memory = V4L2_MEMORY_MMAP;
			mV4L2Buf.index = i;

			SETPIXELBUFFERFLAG(mPixelBuffers[i],PIXEL_BUFFER_INUSE_GRABBER_Q);
			if ( xioctl( mDevID, VIDIOC_QBUF, &mV4L2Buf) == -1) {
				V4L2DEV_WARNING("VIDIOC_QBUF failed\n");
				CLEARPIXELBUFFERFLAG(mPixelBuffers[i],PIXEL_BUFFER_INUSE_GRABBER_Q);	// clear in use flag as the pixbuf was not queued
			}
		}
		// dequeue a filled PixelBuffer from driver
		memset (&mV4L2Buf, 0, sizeof(v4l2_buffer));					// reset struct to 0s
		mV4L2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		mV4L2Buf.memory = V4L2_MEMORY_MMAP;
		if ( xioctl(mDevID, VIDIOC_DQBUF, &mV4L2Buf) == -1) {
			V4L2DEV_WARNING("VIDIOC_DQBUF failed\n");
			return;
		}
		else {
			mBuffersOrder.push_front(mV4L2Buf.index);					// say to the grabber what is the actual PixelBuffer
			CLEARPIXELBUFFERFLAG(mPixelBuffers[mV4L2Buf.index],PIXEL_BUFFER_INUSE_GRABBER_Q);
			mBuffersOrder.pop_back();							// so that mBuffersOrder.size() is never > mPixelBuffers.size()
			mPixelBuffers[mV4L2Buf.index]->sec = mV4L2Buf.timestamp.tv_sec;		// set timestamp
			mPixelBuffers[mV4L2Buf.index]->usec = mV4L2Buf.timestamp.tv_usec;

		}
		return;
	}
/*** PTRS STREAMING ***/
	else if (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_PTRS)) {
		// queue buffers
		for (unsigned int index = 0; index < mPixelBuffers.size(); index++) {
			if ( PIXELBUFFERISLOCKED(mPixelBuffers[index]) ) continue;		// if PixelBuffer has some locks than don't touch it

			memset (&mV4L2Buf, 0, sizeof(v4l2_buffer));				// reset struct to 0s
			mV4L2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			mV4L2Buf.memory = V4L2_MEMORY_USERPTR;
			mV4L2Buf.m.userptr = (unsigned long) mPixelBuffers[index]->buf;
			mV4L2Buf.length = mPixelBuffers[index]->length;

			SETPIXELBUFFERFLAG(mPixelBuffers[index],PIXEL_BUFFER_INUSE_GRABBER_Q);
			if ( xioctl( mDevID, VIDIOC_QBUF, &mV4L2Buf) == -1) {
				V4L2DEV_WARNING("VIDIOC_QBUF failed\n");
				CLEARPIXELBUFFERFLAG(mPixelBuffers[index],PIXEL_BUFFER_INUSE_GRABBER_Q);
			}
		}

		// dequeue a filled PixelBuffer from driver
		memset (&mV4L2Buf, 0, sizeof(v4l2_buffer));					// reset struct to 0s
		mV4L2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		mV4L2Buf.memory = V4L2_MEMORY_USERPTR;
		if ( xioctl(mDevID, VIDIOC_DQBUF, &mV4L2Buf) == -1) {
			V4L2DEV_WARNING("VIDIOC_DQBUF failed\n");
			return;
		}
		else {
			CLEARPIXELBUFFERFLAG(mPixelBuffers[mV4L2Buf.index],PIXEL_BUFFER_INUSE_GRABBER_Q);
			mBuffersOrder.push_front(mV4L2Buf.index);					// say to the grabber what is the actual PixelBuffer
			mBuffersOrder.pop_back();							// so that mBuffersOrder.size() is never > mPixelBuffers.size()
			mPixelBuffers[mV4L2Buf.index]->sec = mV4L2Buf.timestamp.tv_sec;		// set timestamp
			mPixelBuffers[mV4L2Buf.index]->usec = mV4L2Buf.timestamp.tv_usec;
		}
		return;
	}
/*** READ/WRITE STREAMING ***/
	else if (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_READWRITE)) {
		// search a buffer without locks
		unsigned int pos = -1;
		for (unsigned int index = 0; index < mPixelBuffers.size(); index++) {
			if ( ! PIXELBUFFERISLOCKED(mPixelBuffers[index]) ) {			// if PixelBuffer has some locks than don't touch it
				pos = index;
				SETPIXELBUFFERFLAG(mPixelBuffers[pos],PIXEL_BUFFER_INUSE_GRABBER_Q);
				break;
			}
		}
		if (pos == -1)  { // if we get here: no buffer is available or mPixelBuffers.size()==0 
			V4L2DEV_CRITICAL("no buffers available\n");
			return;  
		}

		if (read(mDevID, mPixelBuffers[pos]->buf, mPixelBuffers[pos]->length) <0) {
			CLEARPIXELBUFFERFLAG(mPixelBuffers[pos],PIXEL_BUFFER_INUSE_GRABBER_Q);
			V4L2DEV_WARNING("read() error\n");
#ifdef V4L2_Device_Verbose
			std::cout << "on grab() - read : errno : "<< errnoToString(errno) << "\n";
#endif
			return;
		}
		CLEARPIXELBUFFERFLAG(mPixelBuffers[pos],PIXEL_BUFFER_INUSE_GRABBER_Q);
		mBuffersOrder.push_front(pos);					// say to the grabber what is the actual PixelBuffer
		mBuffersOrder.pop_back();						// so that mBuffersOrder.size() is never > mPixelBuffers.size()
		return;
	}
	// if we get here this is very bad ...
	assert(0);
}


bool V4L2_Device::internal_setup_io_MMAP (void) {
	// check if set up was already done
	if (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_MMAP) or 
	    GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_PTRS) or
	    GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_READWRITE     )    ) return false;

// *** try with mmap streaming *** //
	if (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_STREAMING) and GET_V4L2DEV_FLAG(VIDEO_CAPTURE_STREAMING_MMAP)) {

		v4l2_requestbuffers reqBufs;
		memset (&reqBufs, 0 , sizeof(v4l2_requestbuffers));
		reqBufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;	
		reqBufs.memory = V4L2_MEMORY_MMAP;
		reqBufs.count = mMaxNumBuffers;					// number of buffers we want to allocate

		if (xioctl (mDevID, VIDIOC_REQBUFS, &reqBufs) == -1) {
			V4L2DEV_WARNING("VIDIOC_REQBUFS failed\n");
			return false;
		}

		if (reqBufs.count < V4L2_MIN_NUM_BUFFERS) {
			V4L2DEV_WARNING("couldn't allocate all necessary buffers\n");
			return false;
		}

		SET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_MMAP);		// set flag: if mmap fails, internal_reset will know that unmap is needed

		while (mPixelBuffers.size()!= reqBufs.count) {			// push reqBufs.count new PixelBuffers in vector mPixelBuffers
			PixelBuffer* newBuf = new PixelBuffer();
			if (newBuf==NULL) {
				V4L2DEV_CRITICAL("out of memory\n");
				return false;
			}
			PIXELBUFFERCLEARSTRUCT(newBuf);						// reset PixelBuffer struct
			newBuf->width = mMaxWidth;
			newBuf->height = mMaxHeight;
			mPixelBuffers.push_back(newBuf);					// push new buffer in the vector
			mBuffersOrder.push_front(-1);						// make mBuffersOrder.size() = mPixelBuffers.size()
			// -1 means that PixelBuffers are not yet valid
		}

		for (unsigned int bufIndex = 0; bufIndex < reqBufs.count; bufIndex++) {
			v4l2_buffer queryBuf;
			memset (&queryBuf, 0 , sizeof(v4l2_buffer));
			queryBuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			queryBuf.memory = V4L2_MEMORY_MMAP;
			queryBuf.index = bufIndex;
      
			if (xioctl ( mDevID, VIDIOC_QUERYBUF, &queryBuf) == -1) {
				V4L2DEV_WARNING("VIDIOC_QUERYBUF failed\n");
				return false;
			}
      
			mPixelBuffers[bufIndex]->length = queryBuf.length;			// driver allocates memory in respect to the format
			mPixelBuffers[bufIndex]->buf = mmap ( NULL, queryBuf.length, PROT_READ | PROT_WRITE, MAP_SHARED, mDevID, queryBuf.m.offset);
      
			if ( mPixelBuffers[bufIndex]->buf == MAP_FAILED) {
				V4L2DEV_WARNING("mmap failed\n");
				return false;
			}
		}
		return true;
	}
	return false;
}  


bool V4L2_Device::internal_setup_io_PTRS (void) {
	// check if set up was already done
	if (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_MMAP) or 
	    GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_PTRS) or
	    GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_READWRITE     )    ) return false;

// *** try with ptrs streaming *** //
	if (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_STREAMING) and GET_V4L2DEV_FLAG(VIDEO_CAPTURE_STREAMING_PTRS) ) {
		v4l2_requestbuffers reqBuf;
		memset (&reqBuf, 0, sizeof (reqBuf));					// reset struct to 0s
		reqBuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		reqBuf.memory = V4L2_MEMORY_USERPTR;
		if (xioctl (mDevID, VIDIOC_REQBUFS, &reqBuf) == -1) {
			V4L2DEV_WARNING("VIDIOC_REQBUFS failed\n");
			return false;
		}

		// check if we know how to handle this pixelformat
		if (v4l2_pix_fmt_to_pixelbuffer_fmt(mImageFormat.fmt.pix.pixelformat) == PIXELBUFFER_FMT_NONE ) {
			V4L2DEV_CRITICAL("unknown pixelbuffer fmt\n");
			return false;
		}

		SET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_PTRS);			// set that we use this method
		// if VIDIOC_REQBUFS request was successful create the desidered number of PixelBuffers
		while (mPixelBuffers.size()!= mMaxNumBuffers) {				// push reqBufs.count new PixelBuffers in vector mPixelBuffers
			PixelBuffer* newBuf = new PixelBuffer();
			if (newBuf==NULL) {
				V4L2DEV_CRITICAL("out of memory\n");
				return false;
			}
			PIXELBUFFERCLEARSTRUCT(newBuf);						// reset PixelBuffer struct
			newBuf->width = mMaxWidth;
			newBuf->height = mMaxHeight;
			newBuf->length =  pixelbuffer_length ( v4l2_pix_fmt_to_pixelbuffer_fmt(mImageFormat.fmt.pix.pixelformat), mMaxWidth, mMaxHeight);
			newBuf->buf = (void*) malloc( newBuf->length );				// malloc memory in respect to format, width and height
			if (newBuf->buf==NULL) {
				V4L2DEV_CRITICAL("out of memory\n");
				return false;
			}
			mPixelBuffers.push_back(newBuf);						// push new buffer in the vector
			mBuffersOrder.push_front(-1);						// make mBuffersOrder.size() = mPixelBuffers.size()
			// -1 means that PixelBuffers are not yet valid
		}
		return true;
	}
	return false;
}  


bool V4L2_Device::internal_setup_io_READ (void) {
	// check if set up was already done
	if (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_MMAP) or 
	    GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_PTRS) or
	    GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_READWRITE     )    ) return false;

// *** try with normal (maybe slower) read/write *** //
	if (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_READWRITE)) {
		// check if we know how to handle this pixelformat
		if (v4l2_pix_fmt_to_pixelbuffer_fmt(mImageFormat.fmt.pix.pixelformat) == PIXELBUFFER_FMT_NONE ) {
			V4L2DEV_CRITICAL("unknown pixelbuffer fmt\n");
			return false;
		}

		SET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_READWRITE);			// set that we use this method

		// create the desidered number of PixelBuffers (in read write not many buffers are necessary)
		while (mPixelBuffers.size()!= mMaxNumBuffers) {			// push V4L2_RWMODEBUFFERS new PixelBuffers in vector mPixelBuffers
			PixelBuffer* newBuf = new PixelBuffer();
			if (newBuf==NULL) {
				V4L2DEV_CRITICAL("out of memory\n");
				return false;
			}
			PIXELBUFFERCLEARSTRUCT(newBuf);						// reset PixelBuffer struct
			newBuf->width = mMaxWidth;
			newBuf->height = mMaxHeight;
			newBuf->length =  pixelbuffer_length ( v4l2_pix_fmt_to_pixelbuffer_fmt(mImageFormat.fmt.pix.pixelformat), mMaxWidth, mMaxHeight);
			newBuf->buf = (void*) malloc( newBuf->length );	// malloc memory in respect to format, width and height
			if (newBuf->buf==NULL) {
				V4L2DEV_CRITICAL("out of memory\n");
				return false;
			}
			mPixelBuffers.push_back(newBuf);					// push new buffer in the vector
			mBuffersOrder.push_front(-1);						// make mBuffersOrder.size() = mPixelBuffers.size()
			// -1 means that PixelBuffers are not yet valid
		}
		return true;
	}
}


void V4L2_Device::internal_free_pixbufs_mem (void) {
	// free memory for vector mPixelBuffers
	for (int i=0; i< mPixelBuffers.size(); i++) {
		assert(mPixelBuffers[i]);
		while (PIXELBUFFERISLOCKED(mPixelBuffers[i])) {usleep(100);}		// if something works on mPixelBuffers[i] we must wait
		SETPIXELBUFFERFLAG(mPixelBuffers[i],PIXEL_BUFFER_INUSE_GRABBER_DESTROY);	// so that nobody can get again control on the PixelBuffer

		if (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_MMAP)) {		// mmap streaming case
			if (mPixelBuffers[i]->buf!=MAP_FAILED and mPixelBuffers[i]->buf!=NULL)
				munmap (mPixelBuffers[i]->buf, mPixelBuffers[i]->length);		// unmap memory
		}
		else if (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_PTRS)) {	// ptrs streaming case
			if (mPixelBuffers[i]->buf!=NULL)
				free (mPixelBuffers[i]->buf);						// free memory
		}
		else if (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_READWRITE)) {		// read()/write() streaming case
			if (mPixelBuffers[i]->buf!=NULL)
				free (mPixelBuffers[i]->buf);						// free memory
		}
		delete mPixelBuffers[i];
	}
	while (mPixelBuffers.size()!=0) mPixelBuffers.erase(mPixelBuffers.begin());	// erase all entries in mPixelBuffers
	CLEAR_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_MMAP);
	CLEAR_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_PTRS);
	CLEAR_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_READWRITE);
}  


void V4L2_Device::internal_reset () {

	mBuffersOrder.clear();		 // avoid grabber to give away a bad PixelBuffer
	mStreamFreq = -1.0f;

	if(mDevID>-1) {			// if video device was opened...
		// if streaming IO method were used before calling internal_reset() than streaming must be stopped
		if ( (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_MMAP)) or (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_PTRS)) ) {
			internal_activate_streaming(false);
		}

		// free memory for vector mPixelBuffers
		internal_free_pixbufs_mem ();

		// say to the driver we don't need anymore buffers (must be done after munmap!)
		v4l2_requestbuffers reqBuf;
		memset (&reqBuf, 0, sizeof (reqBuf));
		reqBuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_MMAP)) reqBuf.memory = V4L2_MEMORY_MMAP;
		else if (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_PTRS)) reqBuf.memory = V4L2_MEMORY_USERPTR;
		reqBuf.count = 0;	// !
		if (xioctl (mDevID, VIDIOC_REQBUFS, &reqBuf) == -1) V4L2DEV_CRITICAL("VIDIOC_REQBUFS failed\n");

		close(mDevID);								// close device
		mDevID = -1;								// reset device fd value
	}

	// free memory for vector mGrabberControls
	for (int i=0; i< mGrabberControls.size(); i++) {
		assert(mGrabberControls[i]);
		delete mGrabberControls[i];
	}
	while (mGrabberControls.size()!=0) mGrabberControls.erase(mGrabberControls.begin());

	CLEAR_V4L2DEV_FLAGS;							// reset flags
}



bool V4L2_Device::internal_get_device_capabilities () {
	v4l2_capability mCapability;
	memset (&mCapability, 0 , sizeof(v4l2_capability));				// reset struct to 0s
	if (xioctl( mDevID, VIDIOC_QUERYCAP, &mCapability)== -1 ) {
		V4L2DEV_WARNING("VIDIOC_QUERYCAP failed\n");
		return false;
	}

#ifdef V4L2_Device_Verbose
	std::cout << "\nRetrieving device capabilities ...\n"
		  << " - driver: " 	<< mCapability.driver << "\n"
		// version as of KERNEL_VERSION macro:
		// #define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
		// __u32 version = KERNEL_VERSION(0, 8, 1);
		  << " - version: "  << ((mCapability.version >> 16) & 0xFF) 
		  << "." << ((mCapability.version >> 8) & 0xFF) 
		  << "." << (mCapability.version & 0xFF) << "\n"
		  << v4l2_capabilities_to_string(mCapability.capabilities);
#endif

	// check for capture capablities
	if (!(mCapability.capabilities & V4L2_CAP_VIDEO_CAPTURE)) return false; 	// VIDEO_CAPTURE is necessary!
	// check for read()/write() IO
	if (mCapability.capabilities & V4L2_CAP_READWRITE) SET_V4L2DEV_FLAG(VIDEO_CAPTURE_READWRITE);
	// check for streaming IO capabilities, and if any for what kind of streaming
	if (mCapability.capabilities & V4L2_CAP_STREAMING) {
		SET_V4L2DEV_FLAG(VIDEO_CAPTURE_STREAMING);
		// first try to check for ptrs streaming
		v4l2_requestbuffers bufferRequest;
		memset (&bufferRequest, 0 , sizeof(v4l2_requestbuffers));
		bufferRequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		bufferRequest.memory = V4L2_MEMORY_USERPTR;
		if(!(xioctl( mDevID, VIDIOC_REQBUFS, &bufferRequest)== -1)) SET_V4L2DEV_FLAG(VIDEO_CAPTURE_STREAMING_PTRS);      
		// then check for streaming using mmap
		memset (&bufferRequest, 0 , sizeof(v4l2_requestbuffers));
		bufferRequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		bufferRequest.memory = V4L2_MEMORY_MMAP;
		if(!(xioctl( mDevID, VIDIOC_REQBUFS, &bufferRequest)==-1)) SET_V4L2DEV_FLAG(VIDEO_CAPTURE_STREAMING_MMAP);      
	}
	// some drivers say that they can use streaming IO but they actually can't in reality...
	if (! (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_STREAMING_PTRS) or 
	       GET_V4L2DEV_FLAG(VIDEO_CAPTURE_STREAMING_MMAP))   ) CLEAR_V4L2DEV_FLAG(VIDEO_CAPTURE_STREAMING);
#ifdef V4L2_Device_Verbose
	std::cout << " - I/O streaming: ";
	if (!GET_V4L2DEV_FLAG(VIDEO_CAPTURE_STREAMING)) std::cout << "NO\n";
	else {
		std::cout << "YES\n";
		std::cout << "    - Streaming with ptrs: \n";
		if (!GET_V4L2DEV_FLAG(VIDEO_CAPTURE_STREAMING_PTRS)) std::cout << "NO\n";
		else std::cout << "YES\n";
		std::cout << "    - Streaming with mmap: \n";
		if (!GET_V4L2DEV_FLAG(VIDEO_CAPTURE_STREAMING_MMAP)) std::cout << "NO\n";
		else std::cout << "YES\n";
	}
#endif
	return true;
}



bool V4L2_Device::internal_get_cropscale_capabilities () {
	memset (&mCropScaleCap, 0 , sizeof(v4l2_cropcap));
	if(xioctl( mDevID, VIDIOC_CROPCAP, &mCropScaleCap)== -1 ) {
		// should never get here using capture devices (but checking doesn't hurts)
		V4L2DEV_WARNING("no crop&scale support\n");
		return true;
	}

	SET_V4L2DEV_FLAG(VIDEO_CAPTURE_CROPSCALE);    
	v4l2_crop newCrop;						    			// set crop to device default
	memset (&newCrop, 0, sizeof (v4l2_crop));
	newCrop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	newCrop.c = mCropScaleCap.defrect;
	if ( xioctl( mDevID, VIDIOC_S_CROP, &newCrop) == -1) V4L2DEV_WARNING("VIDIOC_S_CROP failed\n");

#ifdef V4L2_Device_Verbose
	std::cout << "\nRetrieving crop&scale capabilities ..."
		  << " - type: " 		<< v4l2_crop_scale_opt_to_string(mCropScaleCap.type) << "\n"
		  << " - bounds: " 		<< v4l2_rect_struct_to_string(mCropScaleCap.bounds) << "\n"
		  << " - default rect: "	<< v4l2_rect_struct_to_string(mCropScaleCap.defrect) << "\n"
		  << " - pixel aspect: "	<< mCropScaleCap.pixelaspect.numerator << "/" << mCropScaleCap.pixelaspect.denominator
		  << "\n";
#endif
}



void V4L2_Device::internal_addCtrlIfAny (unsigned int V4L2_ctrlID) {
  
	v4l2_queryctrl queryCtrl;								// create a query struct and set ctrl id
	memset (&queryCtrl, 0, sizeof(v4l2_queryctrl));
	queryCtrl.id = V4L2_ctrlID;

	GrabberControlData* ctrlData = NULL;							// new struct ptr for control's data
	
	for (int i= 0; i< mGrabberControls.size(); i++) {  					// check if this ctrl was already registered
		if (mGrabberControls[i]->ID == v4l2_ctrl_id_to_grabber_ctrl_id(V4L2_ctrlID) ) return;
	}

	if ( -1 == xioctl(mDevID, VIDIOC_QUERYCTRL, &queryCtrl)) return;			// ctrl not supported

	// if ctrl is one we don't care about return
	if ( v4l2_ctrl_id_to_grabber_ctrl_id(V4L2_ctrlID) == GRABBER_CTRL_NONE) return;

	// check ctrl flags to see if it's disabled, grabbed...
	if ( (queryCtrl.flags & V4L2_CTRL_FLAG_DISABLED)
	     or (queryCtrl.flags & V4L2_CTRL_FLAG_GRABBED)
	     or (queryCtrl.flags & V4L2_CTRL_FLAG_READ_ONLY)
	     or (queryCtrl.flags & V4L2_CTRL_FLAG_INACTIVE)   ) return;

	ctrlData = new GrabberControlData();					// create the new struct and update queried data
	ctrlData->ID = v4l2_ctrl_id_to_grabber_ctrl_id(V4L2_ctrlID);
	ctrlData->min  = queryCtrl.minimum;
	ctrlData->max  = queryCtrl.maximum;
	ctrlData->step = queryCtrl.step;
	ctrlData->def  = queryCtrl.default_value;

	// try to retrieve also current value of control
	v4l2_control ctrlValue;
	ctrlValue.id = V4L2_ctrlID;
	if ( -1 == xioctl(mDevID, VIDIOC_G_CTRL, &ctrlValue)) {
		V4L2DEV_WARNING("VIDIOC_G_CTRL failed\n");
		ctrlData->value = 0;
	}
	else ctrlData->value = ctrlValue.value;				// set value to the retrieved one
	mGrabberControls.push_back(ctrlData);

#ifdef V4L2_Device_Verbose
	std::cout << grabber_ctrl_id_to_string(v4l2_ctrl_id_to_grabber_ctrl_id(V4L2_ctrlID)) << "*";
#endif
}



bool V4L2_Device::internal_get_streaming_params() {
	v4l2_streamparm streamP;
	memset (&streamP, 0 , sizeof(v4l2_streamparm));		// reset struct to 0s
	streamP.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;			// set the stream type
	if(xioctl( mDevID, VIDIOC_G_PARM, &streamP)== -1 ) {
		V4L2DEV_WARNING("VIDIOC_G_PARM failed\n");
		return false;
	}

	if (streamP.parm.capture.capability & V4L2_CAP_TIMEPERFRAME) SET_V4L2DEV_FLAG(VIDEO_CAPTURE_HAS_FRAME_SKIPPING_SUPPORT);
	if (streamP.parm.capture.capturemode & V4L2_MODE_HIGHQUALITY) SET_V4L2DEV_FLAG(VIDEO_CAPTURE_HAS_HIGHQ_STILLIMAGE_SUPPORT);

	float num, den;						// store frequency
	num = (float) streamP.parm.capture.timeperframe.numerator;
	den = (float) streamP.parm.capture.timeperframe.denominator;
	mStreamFreq = den/num;					// timeperframe.den/timeperframe/num

#ifdef V4L2_Device_Verbose
	std::cout << "\nRetrieving streaming parameters ...\n" 
		  << v4l2_capture_param_to_string(&(streamP.parm.capture)) << "\n";
#endif
	return true;
}


bool V4L2_Device::internal_get_image_format() {
	// Retrieving camera image format
	memset (&mImageFormat, 0 , sizeof(v4l2_format));
	mImageFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;		// set stream type
	if(xioctl( mDevID, VIDIOC_G_FMT, &mImageFormat)== -1 ) {
		V4L2DEV_WARNING("VIDIOC_G_FMT failed\n");
		return false;
	}
#ifdef V4L2_Device_Verbose
	std::cout << "\nRetrieving image format...\n"
		  << v4l2_pix_format_struct_to_string(&mImageFormat.fmt.pix) << "\n";
#endif
	return true;
}


bool V4L2_Device::internal_activate_streaming (bool activate) {
	// check if we really need to d/activate streaming
	if ( ! (GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_MMAP) or GET_V4L2DEV_FLAG(VIDEO_CAPTURE_USING_STREAMING_PTRS)) )
		return true;

	v4l2_buf_type bufType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (activate) {
		if ( xioctl(mDevID, VIDIOC_STREAMON, &bufType) == -1) {
			V4L2DEV_WARNING("VIDIOC_STREAMON failed\n");
			return false;
		}
	}
	else if ( xioctl( mDevID, VIDIOC_STREAMOFF, &bufType) == -1) {	// this also dequeues buffers from driver
		V4L2DEV_WARNING("VIDIOC_STREAMOFF failed\n");
		return false;
	}
	return true;
} 



// set image format
bool V4L2_Device::internal_set_format(PixelBufferFormat fmt) {
	if (mDevID==-1) return false;
	// first retrieve format data
	internal_get_image_format();
	// set fmt field to new value
	mImageFormat.fmt.pix.pixelformat = pixelbuffer_fmt_to_v4l2_pix_fmt(fmt);

	if ( xioctl(mDevID, VIDIOC_S_FMT, &mImageFormat) == -1) {
		V4L2DEV_WARNING("VIDIOC_S_FMT failed\n");
		return false;
	}
	return true;
}


PixelBufferFormat V4L2_Device::get_format() {
	if (mDevID==-1) return PIXELBUFFER_FMT_NONE;
	return v4l2_pix_fmt_to_pixelbuffer_fmt(mImageFormat.fmt.pix.pixelformat);
}


bool V4L2_Device::set_crop(CropData &cas) {
	if (mDevID==-1) return false;
	if (!GET_V4L2DEV_FLAG(VIDEO_CAPTURE_CROPSCALE)) return false;

	v4l2_crop crop;				// setup crop struct
	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	crop.c.left = cas.left;
	crop.c.top = cas.top;
	crop.c.width = cas.width;
	crop.c.height = cas.height;
  
	if ( xioctl(mDevID, VIDIOC_S_CROP, &crop) == -1) {
		V4L2DEV_WARNING("VIDIOC_S_CROP failed\n");
		return false;
	}
	// even when request succedes, the driver may change some value to fit hardware capabilities
	// return this info in cas 
	cas.left = crop.c.left;
	cas.top = crop.c.top;
	cas.width = crop.c.width;
	cas.height = crop.c.height;
	return true;
}


bool V4L2_Device::get_crop(CropData &cas) {
	if (mDevID==-1) return false;
	if (!GET_V4L2DEV_FLAG(VIDEO_CAPTURE_CROPSCALE)) return false;

	v4l2_crop crop;				// crop struct for request
	memset (&crop, 0 , sizeof(v4l2_crop));
	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
	if ( xioctl(mDevID, VIDIOC_G_CROP, &crop) == -1) {
		V4L2DEV_WARNING("VIDIOC_G_CROP failed\n");
		return false;
	}
	// even when request succedes, the driver may change some value to fit hardware capabilities
	// return this info in cas 
	cas.left = crop.c.left;
	cas.top = crop.c.top;
	cas.width = crop.c.width;
	cas.height = crop.c.height;
	return true;
}



