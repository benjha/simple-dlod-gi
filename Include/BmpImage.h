

#ifndef __bmp_image
#define __bmp_image

class BmpImage{
public:
	BmpImage();
	~BmpImage();
	void readBmpFile(char *filename);  // Load image from a BMP file
	void writeBmpFile(char *filename); // Save image to a BMP file
	int  setData (unsigned char *pixelsData, int w, int h);
	void getData (unsigned char *pixelsData, int mode);
	unsigned char *data;               // Image data
	int width;                         // Image width
	int height;                        // Image height
private:
	int memCreated;
};

#endif