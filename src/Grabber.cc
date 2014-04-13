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

#include "Grabber.hh"

Grabber::Grabber(GrabberInitData* initData) {
	mPathToDev = "";			// set path to dev file
	mPathToDev += initData->pathToDev;

	mMaxWidth = initData->maxWidth;		// max image's width
	mMaxHeight = initData->maxHeight;	// max image's height
}


Grabber::~Grabber() {
	// free memory
	while (mGrabberControls.size()!=0) {
		delete mGrabberControls[0];
		mGrabberControls.erase (mGrabberControls.begin());
	}
	mBuffersOrder.clear();
}


const GrabberControlData* Grabber::get_ctrl_data (GrabberControlID id) {
	int pos = find_ctrl_index(id);
	if (pos>-1) return mGrabberControls[pos];
	// if we get here we didn't find the control with GrabberControlID id
	return NULL;
}


int Grabber::get_ctrl_value (GrabberControlID id) {
	const GrabberControlData* ctrlData= get_ctrl_data(id);
	if (ctrlData) return ctrlData->value;
	return -1;
}


PixelBuffer* Grabber::get_last_grabbed()  {
	// search the most recent PixelBuffer without locks
	//  ! if mBuffersOrder.size()==0 than (mBuffersOrder.begin()==mBuffersOrder.end()) and while loop is jumped
	pbIter=mBuffersOrder.begin();
	// -1 at beginning of list means PixelBuffers are not valid
	while ( pbIter!=mBuffersOrder.end()) {
		if (*pbIter == -1) { pbIter++; continue; }
		if ( !PIXELBUFFERISLOCKED(mPixelBuffers[*pbIter])) return mPixelBuffers[*pbIter];
		pbIter++; //  inc iterator
	}
	// if we get here all PixelBuffers had locks, sorry...
	GRABBER_WARNING("no pixbuf available\n");
	return NULL;
}


int Grabber::find_ctrl_index(GrabberControlID id)  {
	for (int i= 0; i< mGrabberControls.size(); i++) {
		if (mGrabberControls[i]->ID == id) return i;
	}
	// if we get here we didn't find the control with GrabberControlID id
	return -1;
}



