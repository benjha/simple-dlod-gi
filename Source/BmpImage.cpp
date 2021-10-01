
#include <stdio.h>
#include <string.h>
#include <GL/glut.h>
#include "../Include/BmpImage.h"

//#define BMP_IMAGE_SWAP_ROWS

BmpImage::BmpImage(){
	data = NULL;
	width = 0;
	height = 0;
	memCreated = 0;
}

int BmpImage::setData(unsigned char *pixelsData, int w, int h){
	int j,i=0;
	if((data = new unsigned char[w*h*3])==NULL)
		return 0;
	for (j=0; j<w*h*4; j+=4){
		data[i] = pixelsData[j];
		data[i+1] = pixelsData[j+1];
		data[i+2] = pixelsData[j+2];
		i+=3;
	}
	width = w;
	height = h;
	memCreated = 0;
	return 1;
}

void BmpImage::getData (unsigned char *pixelsData, int mode)
{
	int j=0, i=0;

	switch (mode) { 
	case GL_RGBA:
		for (j=0; j<width*height*3; j+=3){
			pixelsData[i] = data[j];
			pixelsData[i+1] = data[j+1];
			pixelsData[i+2] = data[j+2];
			pixelsData[i+3] = 255;
			i+=4;
		}
		break;
	case GL_RGB:
		for (j=0; j<width*height*3; j+=3){
			pixelsData[j] = data[j];
			pixelsData[j+1] = data[j+1];
			pixelsData[j+2] = data[j+2];
		}
		break;
	}
}

BmpImage::~BmpImage(){
	if(data && memCreated) delete(data);
}

#define bytesToInt(ptr, idx)     *((int *) & ptr [ idx ])
#define bytesToShort(ptr, idx)   *((short *) & ptr [ idx ])

void BmpImage::readBmpFile(char *filename){
	FILE *bmpFile;
	char buffer[64];
	int i, j, k, m;
	int fileSize;
	int dataOffset;
	int planes;
	int bitsPerPixel;
	int compression;
	int imageSize;
	int xRes;
	int yRes;
	int colorsUsed;
	int importantColors;
	int sample;
	int samplePtr;
	int offset, cacheOffset;
	int imageCacheSize;
	int samplesPerByte;
	int paddedWidth;
	int scanLine, filter, bitsToShift;
	unsigned char *colorTable=NULL;
	unsigned char *imageCache=NULL;

	bmpFile = fopen(filename, "rb");

	if(bmpFile){
		// Bitmap header
		fread(buffer, 1, 14, bmpFile);
		fileSize   = bytesToInt(buffer, 2);
		dataOffset = bytesToInt(buffer, 10);

		// Bitmap Info Header
		fread(buffer, 1, 40, bmpFile);
		width = bytesToInt(buffer, 4);
		height = bytesToInt(buffer, 8);
		
		planes = bytesToShort(buffer, 12);
		bitsPerPixel = bytesToShort(buffer, 14);
		compression = bytesToInt(buffer, 16);
		imageSize = bytesToInt(buffer, 20);
		xRes = bytesToInt(buffer, 24);
		yRes = bytesToInt(buffer, 28);
		colorsUsed = bytesToInt(buffer, 32);
		importantColors = bytesToInt(buffer, 36);

		// Bitmap pallete
		if(bitsPerPixel <= 8){
			if(colorsUsed == 0){
				colorsUsed = 1;
				for(i=0; i<bitsPerPixel; ++i){
					colorsUsed *= 2;
				}
			}
			colorTable = new unsigned char[colorsUsed*3];
			fread(colorTable, 1, colorsUsed*3, bmpFile);
		}

		// Bitmap Raster Data
		imageSize = width * height * 3;
		
		if(compression == 0){
			data = new unsigned char[imageSize];
			memCreated = 1;
			paddedWidth = ((width * bitsPerPixel + 31) / 32) * 4;
			imageCacheSize = paddedWidth * height * 3;
			imageCache = new unsigned char[imageCacheSize];
			fread(imageCache, 1, imageCacheSize, bmpFile);

			if(bitsPerPixel == 24){
				for(i=0; i<height; ++i){
					offset = i * width * 3 - 4;
#ifdef BMP_IMAGE_SWAP_ROWS
					cacheOffset = (height - i - 1) * paddedWidth;
#else
					cacheOffset = i * paddedWidth;
#endif
					for(j=0, k=cacheOffset; j<width; ++j) {
						offset += 6;
						data[offset--] = imageCache[k++];
						data[offset--] = imageCache[k++];
						data[offset--] = imageCache[k++];
					}
				}
			} else if(bitsPerPixel == 16){
				for(i=0; i<height; ++i){
					offset = i * width * 3 - 4;
#ifdef BMP_IMAGE_SWAP_ROWS
					cacheOffset = (height - i - 1) * paddedWidth;
#else
					cacheOffset = i * paddedWidth;
#endif
					for(j=0, k=cacheOffset; j<width; ++j) {
						offset += 6;
						data[offset--] = (imageCache[k]&0xF)<<4;
						data[offset--] = (imageCache[k++]&0xF)<<4;
						data[offset--] = (imageCache[k++]&0xF)<<4;
					}
				}
			} else if(bitsPerPixel == 8){
				for(i=0; i<height; ++i){
					offset = i * width * 3 - 4;
#ifdef BMP_IMAGE_SWAP_ROWS
					cacheOffset = (height - i - 1) * paddedWidth;
#else
					cacheOffset = i * paddedWidth;
#endif
					for(j=0; j<width; ++j){
						offset += 6;
						sample = imageCache[cacheOffset++]*3;
						data[offset--] = colorTable[sample++];
						data[offset--] = colorTable[sample++];
						data[offset--] = colorTable[sample];
					}
				}

			} else if(bitsPerPixel < 8){
				samplesPerByte = 8 / bitsPerPixel;
				scanLine = width / samplesPerByte;
				filter = 0;
				for(i=0; i<bitsPerPixel; ++i){
					filter = (filter << 1) | 1;
				}
				bitsToShift = 8 - bitsPerPixel;
				for(i=0; i<height; ++i){
					offset = i * width * 3 - 4;
#ifdef BMP_IMAGE_SWAP_ROWS
					cacheOffset = (height - i - 1) * paddedWidth;
#else
					cacheOffset = i * paddedWidth;
#endif
					for(j=0, k=0; j < width && k < paddedWidth; ++k){
						sample = imageCache[cacheOffset + k] & 0xFF;
						for(m=0; m<samplesPerByte && j < width; m++, j++){
							offset += 6;
							samplePtr = ((sample >> bitsToShift) & filter)*3;
							data[offset--] = colorTable[samplePtr++];
							data[offset--] = colorTable[samplePtr++];
							data[offset--] = colorTable[samplePtr];
							sample = sample << bitsPerPixel;
						}
					}
				} // for(i=0; i<height; ++i){
			} // else if(bitsPerPixel < 8)
			delete(imageCache);
			delete(colorTable);
		} // if(compression == 0)
	} // if(bmpFile)
}

void BmpImage::writeBmpFile(char *filename){
	FILE *bmpFile;
	char buffer[64];
	char pad[4];
	int imageSize = width*height*3;
	int rowSize = width*3;
	int paddedWidth = ((width * 24 + 31) / 32) * 4;
	int paddingSize = paddedWidth - rowSize;
//	int ptr;
	pad[0] = pad[1] = pad[2] = pad[3] = 0;

	bmpFile = fopen(filename, "wb");

	if(bmpFile){
		// Bitmap header
		memset(buffer, 0, 14);
		buffer[0]='B'; buffer[1]='M';
		bytesToInt(buffer, 2) = (int) (54 + imageSize); // File size
		bytesToInt(buffer, 10) = (int) (54);            // Data offset
		fwrite(buffer, 1, 14, bmpFile);

		// Bitmap Info Header
		memset(buffer, 0, 40);
		bytesToInt(buffer, 0) = 40;             // Header length
		bytesToInt(buffer, 4) = width;          // Image width
		bytesToInt(buffer, 8) = height;         // Image height
		bytesToShort(buffer, 12) = 1;           // Number of planes
		bytesToShort(buffer, 14) = 24;          // Bits per pixel
		bytesToInt(buffer, 16) = 0;             // Compression = BI_RGB (None)
		bytesToInt(buffer, 20) = imageSize;     // Image size
		bytesToInt(buffer, 24) = 3780;          // Horizontal resolution (pix/m.)
		bytesToInt(buffer, 28) = 3780;          // Vertical resolution (pix/m.)
		bytesToInt(buffer, 32) = 0;             // Colors used (0 = maximum number of colors)
		bytesToInt(buffer, 36) = 0;             // Colors used (0 = maximum number of colors)
		fwrite(buffer, 1, 40, bmpFile);

		// Bitmap data
		for(int i=0, ptr=imageSize-rowSize; i<height; ++i){
#ifdef BMP_IMAGE_SWAP_ROWS
			ptr = imageSize-(rowSize*(i+1));
#else
			ptr = rowSize*i;
#endif
			for(int j=0; j<width; ++j){
				ptr+=2;
				fputc(data[ptr--], bmpFile);
				fputc(data[ptr--], bmpFile);
				fputc(data[ptr], bmpFile);
				ptr+=3;
			}
//			fwrite(&data[ptr], 1, rowSize, bmpFile);
			if(paddingSize > 0){
				fwrite(pad, 1, paddingSize, bmpFile);
			}
		}
		fclose(bmpFile);
	}
}

#undef bytesToInt
#undef bytesToShort
