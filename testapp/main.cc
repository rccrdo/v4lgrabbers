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

#include <string>
#include <iostream>
#include <list>

#include "Debug.hh"
#include "Defaults.hh"
#include "GrabberInitData.hh"
#include "V4L2_Device.hh"
#include "V4L1_Device.hh"
#include "PixelBuffer.hh"

#undef writetofile
#define writetofile

int main(int argc, char **argv) {  

	GrabberInitData d;
	d.pathToDev ="/dev/video0";
	d.maxWidth = 640;
	d.maxHeight = 480;
	d.maxNumBuffers = 6;
	
	Grabber* dev2=new V4L2_Device(&d);
	dev2->init();
	
	Grabber* dev1 =new V4L1_Device(&d);
	dev1->init();  
	
	PixelBuffer* p2 =NULL;
	PixelBuffer* p1 = NULL;
	FILE *out;
	int bright1=0;
	int bright2=0;
#ifdef writetofile
	for (int frame =0; frame < 10; frame ++) {
#else
		for (int frame =0; frame < 500; frame ++) {
#endif
			std::cout << " - frame: " << frame << "\n";
			std::stringstream path;
			dev1->grab();
			dev2->grab();
			std::cout << " - dev1->get_last_grabbed()\n";
			while ( (p1 = dev1->get_last_grabbed())==NULL);
			std::cout << " - dev1->length: " << p1->length <<"\n";
#ifdef writetofile
			path.str("");
			path << "v4l1_" << frame << ".ppm";
			out = fopen( path.str().c_str() ,"wb");
			fprintf(out,"P6\n%i %i\n65535\n", 640, 480);
			for (int i=0; i< p1->length; i++) {
				unsigned int c = ((unsigned char*)p1->buf)[i];
				fwrite( &c, 3, 1, out);
			}
			fclose(out);
#endif

			std::cout << " - dev2->get_last_grabbed()\n";
			while ( (p2 = dev2->get_last_grabbed())==NULL);
			std::cout << " - dev2->length: " << p2->length <<"\n";
#ifdef writetofile
			path.str("");
			path << "v4l2_" << frame << ".ppm";
			out = fopen( path.str().c_str() ,"wb");
			fprintf(out,"P6\n%i %i\n65535\n", 640, 480);
			for (int i=0; i< p2->length; i++) {
				unsigned int c = ((unsigned char*)p2->buf)[i];
				fwrite( &c, 3, 1, out);
			}
			fclose(out);
#endif
			
			usleep(100000);
		}
	}

	return 0;
};

