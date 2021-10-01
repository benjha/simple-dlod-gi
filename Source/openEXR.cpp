
#include "../Include/openEXR.h"

cOpenEXR::cOpenEXR (int width, int height) : m_w (width), m_h (height)
{
	data.resizeErase (m_w,m_h);
}

cOpenEXR::~cOpenEXR ()
{
}

void cOpenEXR::writeFile (char *fileName)
{
    Header header (m_w, m_h);
	header.channels().insert ("R", Channel (FLOAT));
    header.channels().insert ("G", Channel (FLOAT));
	header.channels().insert ("B", Channel (FLOAT));
    
    OutputFile file (fileName, header);

    FrameBuffer frameBuffer;

	frameBuffer.insert ("R",					 // name
						Slice (FLOAT,				 // type
						(char *) &data[0][0].red,	 // base
						sizeof (data[0][0]) * 1,	 // xStride
						sizeof (data[0][0]) * m_w)); // yStride

	frameBuffer.insert ("G",					 // name
						Slice (FLOAT,				 // type
						(char *) &data[0][0].green,	 // base
						sizeof (data[0][0]) * 1,	 // xStride
						sizeof (data[0][0]) * m_w)); // yStride

	frameBuffer.insert ("B",					 // name
						Slice (FLOAT,				 // type
						(char *) &data[0][0].blue,	 // base
						sizeof (data[0][0]) * 1,	 // xStride
						sizeof (data[0][0]) * m_w)); // yStride


    file.setFrameBuffer (frameBuffer);
    file.writePixels (m_h);
}

void cOpenEXR::readFile (char *fileName)
{
    InputFile file  (fileName);

    Box2i dw = file.header().dataWindow();
    m_w  = dw.max.x - dw.min.x + 1;
    m_h = dw.max.y - dw.min.y + 1;
    int dx = dw.min.x;
    int dy = dw.min.y;

    data.resizeErase (m_h, m_w);

    FrameBuffer frameBuffer;

    frameBuffer.insert ("R",					 // name
                        Slice (FLOAT,				 // type
			       (char *) &data[0][0].red,	 // base
				sizeof (data[0][0]) * 1,	 // xStride
				sizeof (data[0][0]) * m_w)); // yStride

    frameBuffer.insert ("G",					 // name
                        Slice (FLOAT,				 // type
			       (char *) &data[0][0].green,	 // base
				sizeof (data[0][0]) * 1,	 // xStride
				sizeof (data[0][0]) * m_w)); // yStride

	frameBuffer.insert ("B",					 // name
                        Slice (FLOAT,				 // type
			       (char *) &data[0][0].blue,	 // base
				sizeof (data[0][0]) * 1,	 // xStride
				sizeof (data[0][0]) * m_w)); // yStride

    file.setFrameBuffer (frameBuffer);
//    file.readPixels (dw.min.y, dw.max.y);
	file.readPixels (0, m_h-1);
}

void cOpenEXR::getData (Array2D<rgb> &buffer)
{
	register i,j;

	for (i=0;i<m_w;i++)
		for (j=0;j<m_h;j++){
			buffer[i][j].red	= data[i][j].red;
			buffer[i][j].green	= data[i][j].green;
			buffer[i][j].blue	= data[i][j].blue;
		}
}

void cOpenEXR::setData (Array2D<rgb> &buffer)
{
	register i,j;

	for (i=0;i<m_w;i++)
		for (j=0;j<m_h;j++){
			data[i][j].red	= buffer[i][j].red;
			data[i][j].green= buffer[i][j].green;
			data[i][j].blue	= buffer[i][j].blue;
		}
}