
#ifndef __OPENEXR_H__
#define __OPENEXR_H__

#pragma warning( disable : 4290 )


#include <ImfOutputFile.h>
#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfArray.h>

#include <iostream>

using namespace std;
using namespace Imf;
using namespace Imath;

struct rgb {
	float red;
	float green;
	float blue;
};


class cOpenEXR {
public:
	cOpenEXR (int width, int height);
	~cOpenEXR ();
	void writeFile	(char *fileName);
	void readFile	(char *fileName);
	void getData	(Array2D<rgb> &buffer);
	void setData	(Array2D<rgb> &buffer);
	Array2D<rgb> data;
	int m_w, m_h;
};


#endif __OPENEXR_H__