/*
   Class Name:

      CTGALoader.

   Created by:

      Allen Sherrod (Programming Ace of www.UltimateGameProgramming.com).

   Description:

      This class loads a tga texture into this object and can save a tga.
*/


#include"../Include/CTgaLoader.h"
#include<string.h>


CTGALoader::CTGALoader()
{
   // Give everything default values.
  image = 0;
  type = 0;
  ID = -1;
  imageWidth = 0;
  imageHeight = 0;
}


CTGALoader::~CTGALoader()
{
   FreeImage();
}


bool CTGALoader::LoadTGA(char* file)
{
   FILE *pfile;
   unsigned char tempColor;
   unsigned char bitCount;
   int colorMode;
   long tgaSize;
   unsigned char unCompressHeader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
   unsigned char tgaHeader[12];
   unsigned char header[6];

   // Open the file.
   if(!file) return false;
   pfile = fopen(file, "rb");
   if(!pfile) return false;

   // Read the header.
   fread(tgaHeader, 1, sizeof(tgaHeader), pfile);

   // We only want to read uncompressed tga's.  Compare the header
   // with data that would be in a uncompressed header.
   if(memcmp(unCompressHeader, tgaHeader, sizeof(unCompressHeader)) != 0)
	   {
		   fclose(pfile);
		   return false;
	   }

   // Read image info.
   fread(header, 1, sizeof(header), pfile);

   // Calculate and save the width and height.
   imageWidth = header[1] * 256 + header[0];
   imageHeight = header[3] * 256 + header[2];

   bitCount = header[4];

   // calculate the color mode and image size.
   colorMode = bitCount / 8;
   tgaSize = imageWidth * imageHeight * colorMode;

   // Allocate data for the image and load it.
   image = new unsigned char[sizeof(unsigned char) * tgaSize];
   fread(image, sizeof(unsigned char), tgaSize, pfile);

   // Convert from BGR to RGB format.
   for(long index = 0; index < tgaSize; index += colorMode)
	   {
		   tempColor = image[index];
		   image[index] = image[index + 2];
		   image[index + 2] = tempColor;
	   }

   fclose(pfile);

   if(colorMode == 3) type = 3;
   else type = 4;

   return true;
}


void CTGALoader::FreeImage()
{
   // When the application is done delete all memory.
   if(image)
      {
         delete[] image;
         image = 0;
      }
}


bool CTGALoader::WriteTGA(char *file, int width, int height, unsigned char *outImage)
{
   FILE *pFile = 0;
   unsigned char tgaHeader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
   unsigned char header[6];
   unsigned char bits = 0;
   int colorMode = 0;
   unsigned char tempColors = 0;

   // Open file for output.
   pFile = fopen(file, "wb");

   // Check if the file opened or not.
   if(!pFile) { fclose(pFile); return false; }

   // Set the color mode, and the bit depth.
   colorMode = 3;
   bits = 24;

   // Save the width and height.
 	header[0] = width % 256;
 	header[1] = width / 256;
 	header[2] = height % 256;
	header[3] = height / 256;
	header[4] = bits;
	header[5] = 0;

   // Write the headers to the top of the file.
   fwrite(tgaHeader, sizeof(tgaHeader), 1, pFile);
   fwrite(header, sizeof(header), 1, pFile);

   // Now switch image from RGB to BGR.
   for(int index = 0; index < width * height * colorMode; index += colorMode)
      {
         tempColors = outImage[index];
         outImage[index] = outImage[index + 2];
         outImage[index + 2] = tempColors;
      }

   // Finally write the image.
   fwrite(outImage, width * height * colorMode, 1, pFile);

   // close the file.
   fclose(pFile);
   return true;
}


// Copyright September 2003
// All Rights Reserved!
// Allen Sherrod
// ProgrammingAce@UltimateGameProgramming.com
// www.UltimateGameProgramming.com