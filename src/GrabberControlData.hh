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


#ifndef GrabberControlData_HH
#define GrabberControlData_HH

/*
  Controls wich may be retouched (for now) are:
  ctrl id			value		description
  GRABBER_CTRL_BRIGHTNESS		integer		Picture brightness, or more precisely, the black level.
  GRABBER_CTRL_CONTRAST		integer		Picture contrast or luma gain.
  GRABBER_CTRL_SATURATION		integer		Picture color saturation or chroma gain.
  GRABBER_CTRL_HUE		integer		Hue or color balance.
  GRABBER_CTRL_AUTO_WHITE_BALANCE	boolean		Automatic white balance (cameras).
  GRABBER_CTRL_RED_BALANCE	integer		Red chroma balance.
  GRABBER_CTRL_BLUE_BALANCE	integer		Blue chroma balance.
  GRABBER_CTRL_GAMMA		integer		Gamma adjust.
  GRABBER_CTRL_EXPOSURE		integer		Exposure (cameras). [Unit?]
  GRABBER_CTRL_AUTOGAIN		boolean		Automatic gain/exposure control.
  GRABBER_CTRL_GAIN		integer		Gain control.
  GRABBER_CTRL_HFLIP		boolean		Mirror the picture horizontally.
  GRABBER_CTRL_VFLIP		boolean		Mirror the picture vertically.
*/

enum GrabberControlID {
	GRABBER_CTRL_BRIGHTNESS,
	GRABBER_CTRL_CONTRAST,
	GRABBER_CTRL_SATURATION,
	GRABBER_CTRL_HUE,
	GRABBER_CTRL_AUTO_WHITE_BALANCE,
	GRABBER_CTRL_RED_BALANCE,
	GRABBER_CTRL_BLUE_BALANCE,
	GRABBER_CTRL_GAMMA,
	GRABBER_CTRL_EXPOSURE,
	GRABBER_CTRL_AUTOGAIN,
	GRABBER_CTRL_GAIN,
	GRABBER_CTRL_HFLIP,
	GRABBER_CTRL_VFLIP,
	GRABBER_CTRL_NONE	// a ctrl that may exist in Grabber implementation but that we don't care about
};

struct GrabberControlData {
	// members
	int min;
	int max;
	int step;
	int def;
	int value;
	GrabberControlID ID;
};


#endif /*GrabberControlData_HH*/
