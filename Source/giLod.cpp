
#pragma warning ( disable : 4244 )

#include <fstream>
#include <ios>
#include <sys/types.h> 
#include <sys/stat.h>
#include <GL/glew.h>
#include "../Include/giLod.h"

using namespace std;

cGILod::cGILod (int width, int height, float sightMag) : cGI (width, height)
{
	register x,y;
	
	mipMap_w = m_w + m_w / 2;
	mipMap_h = m_h;

	mipMapTexId[0]=mipMapTexId[1]=0;
	geometryImageRTTMipMap = new FramebufferObject ();
	CheckErrorsGL("BEGIN : Creating textures");
	geometryImageRTTMipMap->Bind ();
		glGenTextures(2, mipMapTexId); // create (reference to) a new texture
		for(x=0;x<2;x++) {
			glBindTexture(GL_TEXTURE_RECTANGLE_ARB, mipMapTexId[x]);
			// (set texture parameters here)
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
			//create the texture
			glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_FLOAT_RGB16_NV, mipMap_w, mipMap_h,
							0, GL_RGB, GL_FLOAT, 0);
		}
		CheckErrorsGL("END : Creating textures");	
		CheckErrorsGL("BEGIN : Configuring FBO");
		geometryImageRTTMipMap->AttachTexture(GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, mipMapTexId[0]);
		geometryImageRTTMipMap->AttachTexture(GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_RECTANGLE_ARB, mipMapTexId[1]);
		geometryImageRTTMipMap->IsValid();
	geometryImageRTTMipMap->Disable ();
	CheckErrorsGL("END : Configuring FBO");

	geometryImageRTTMipMapDist = new FramebufferObject ();
	CheckErrorsGL("BEGIN : Creating textures");
	geometryImageRTTMipMapDist->Bind ();
		glGenTextures(2, mipMapDistTexId); // create (reference to) a new texture
		for(x=0;x<2;x++) {
			glBindTexture(GL_TEXTURE_RECTANGLE_ARB, mipMapDistTexId[x]);
			// (set texture parameters here)
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
			//create the texture
			glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_FLOAT_RGBA16_NV, mipMap_w, mipMap_h,
							0, GL_RGBA, GL_FLOAT, 0);
		}
		CheckErrorsGL("END : Creating textures");	
		CheckErrorsGL("BEGIN : Configuring FBO");
		geometryImageRTTMipMapDist->AttachTexture(GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, mipMapDistTexId[0]);
		geometryImageRTTMipMapDist->AttachTexture(GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_RECTANGLE_ARB, mipMapDistTexId[1]);
		geometryImageRTTMipMapDist->IsValid();
	geometryImageRTTMipMapDist->Disable ();
	CheckErrorsGL("END : Configuring FBO");
	
	FREE_MEMORY(geometryImageRTVVertex);
	FREE_MEMORY(geometryImageRTVNormal);
	FREE_MEMORY(indices);

	geometryImageRTVVertex	 = new RenderVertexArray (mipMap_w*mipMap_h, 4, GL_HALF_FLOAT_NV);
	geometryImageRTVNormal	 = new RenderVertexArray (mipMap_w*mipMap_h, 4, GL_HALF_FLOAT_NV);

	numIndices = ((mipMap_h-1) * (mipMap_w+1) * 2);
	indices = new unsigned int [numIndices];
	unsigned int	*ptr = indices;
	register aux1, aux2;
	for(y=0; y<mipMap_h-1; y++) {
		for(x=0; x<mipMap_w; x++) {
			if (x==0) {
				aux1 = (y+1)*mipMap_w + x;
				aux2 = y*mipMap_w + x;
			}
			*ptr++ = (y+1)*mipMap_w + x;
			*ptr++ = y*mipMap_w + x;
		}
		*ptr++ = aux1;
		*ptr++ = aux2;
//		aux1 = (y+1)*mipMap_w;
//		aux2 = (y+1)*mipMap_w + x-1;
	}

	numIndicesPoints = ((mipMap_h) * (mipMap_w));
	indicesPoints = new unsigned int [numIndicesPoints];
	ptr = indicesPoints;
	for(y=0; y<mipMap_h*mipMap_w; y++)
		*ptr++ = y;


	calcDistanceMipMap = new ShaderObject ("passthruTexVertex.glsl", "distanceMipMapPixel.glsl");
	calcDistanceMipMap->activate ();
		calcDistanceMipMap->setUniformi ("mipMap", 0);
		calcDistanceMipMap->setUniformi ("mipMapNormal", 1);
		calcDistanceMipMap->setUniformi ("LODSelection", 2);
	calcDistanceMipMap->deactivate ();

	genLODSelectionImage = new ShaderObject ("passthruTexVertex.glsl", "genLODSelectionPixel.glsl");

	glNewList (SET_PROJECTION1_0, GL_COMPILE);
	    glMatrixMode(GL_PROJECTION);
		glPushMatrix();
	    glLoadIdentity();
        glGetIntegerv(GL_VIEWPORT, vp);
		glViewport (0,0, mipMap_w,mipMap_h);
		glOrtho(0.0, mipMap_w, 0.0, mipMap_h, -1,1);
		glMatrixMode (GL_MODELVIEW);
		glLoadIdentity ();
		glClearColor (0.0,0.0,0.0,1);
		glClear (GL_COLOR_BUFFER_BIT);
	glEndList ();

	glNewList (SET_PROJECTION1_1, GL_COMPILE);
		glMatrixMode(GL_PROJECTION);
		glPopMatrix ();
		glMatrixMode(GL_MODELVIEW);
        glViewport(vp[0], vp[1], vp[2], vp[3]);
	glEndList ();

	lodLevels = 1;
	sightMagnitude = sightMag;
	sightLOD=0.0;

}
/*------------------------------------------------------------------------------*/
cGILod::~cGILod ()
{
	FREE_MEMORY (geometryImageRTTMipMap);
	FREE_MEMORY (geometryImageRTTMipMapDist);
	FREE_MEMORY (calcDistanceMipMap);
	FREE_MEMORY (genLODSelectionImage);
	FREE_MEMORY (indicesPoints);

}
/*------------------------------------------------------------------------------*/
//#define DETAIL 6
void cGILod::generateMipMap () 
{
	register x=m_w,y=m_h,j, flag=0;
	float auxSight=0.0;
	
	geometryImageRTTMipMap->Bind();
		
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);// Draw into the first texture

		glCallList (SET_PROJECTION1_0);

		genGI->activate ();
			genGI->setUniformf ("scaleFactor", 1.0);
			glScalef (m_w, m_h, 0.0f);	
			model->Display(DISPLAY_GI);	
			for (j=m_w; j>4;j/=2) {
					glLoadIdentity ();
					glTranslatef (m_w,m_h-y,0);
					x/=2; y/=2;			
					glScalef (x, y, 0);
					model->Display(DISPLAY_GI);
					lodLevels++;
			}			
			glLoadIdentity ();
			glFlush ();
		genGI->deactivate ();

		x=m_w,y=m_h, flag=0;
		glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);	
		glClearColor (0.0,0,0,1);
		glClear (GL_COLOR_BUFFER_BIT);

		genGI->activate ();
			genGI->setUniformf ("scaleFactor", 1.0);
			glScalef (m_w, m_h, 0.0f);	
			model->Display(DISPLAY_NORMALS);
			for (j=m_w; j>4;j/=2) {
					glLoadIdentity ();
					glTranslatef (m_w,m_h-y,0);
					x/=2; y/=2;	
					glScalef (x, y, 0);
					model->Display(DISPLAY_NORMALS);	
			}
		genGI->deactivate ();

		glLoadIdentity ();

		geometryImageRTVVertex->Read (GL_COLOR_ATTACHMENT0_EXT, mipMap_w, mipMap_h);
		geometryImageRTVNormal->Read (GL_COLOR_ATTACHMENT1_EXT,  mipMap_w, mipMap_h);

		glCallList (SET_PROJECTION1_1);
	geometryImageRTTMipMap->Disable ();
	
	x=m_w,y=m_h, flag=0;
	sightLOD = sightMagnitude/lodLevels;
	auxSight=sightLOD;
	
	
	geometryImageRTTMipMapDist->Bind ();
		glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);// Draw into the first texture
		glCallList (SET_PROJECTION1_0);
		genLODSelectionImage->activate ();
			glScalef (m_w, m_h, 0.0f);
			genLODSelectionImage->setUniformf ("xMag", sightLOD);
			DrawQuad (m_w, m_h, m_w, m_h);
			auxSight+=sightLOD;
			for (j=m_w; j>4;j/=2) {
				glLoadIdentity ();
				glTranslatef (m_w,m_h-y,0);
				x/=2; y/=2;
				genLODSelectionImage->setUniformf ("xMag", auxSight);
				DrawQuad (x,y,x,y);	
				auxSight+=sightLOD;
			}
		genLODSelectionImage->deactivate ();
		glLoadIdentity ();
		glCallList (SET_PROJECTION1_1);
	geometryImageRTTMipMapDist->Disable ();
	
	float *pixels = new float [lodLevels];
	for (j=0;j<lodLevels;j++){
		pixels[j] = sightLOD*j;
	}

	glGenTextures (1,&mipMapDistTexId[2]);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, mipMapDistTexId[2]);
	
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, lodLevels,1,
							0, GL_LUMINANCE, GL_FLOAT, pixels);
	CheckErrorsGL("END : Creating textures");

	delete [] pixels;

	calcDistanceMipMap->activate ();
		calcDistanceMipMap->setUniformf ("sightLOD", sightLOD);
	calcDistanceMipMap->deactivate ();

	glClearColor (1,1,1,1);
}
/*------------------------------------------------------------------------*/
void cGILod::DisplayTextures ()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, mipMap_w, 0.0, mipMap_h, -1.0, 1.0);
     
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

	
    glLoadIdentity();

	
    glScalef(0.25f + 0.25/2.0f, 0.25f, 1.0f);	 
	//glTranslatef(0.0, mipMap_h, 0.0f);
	DisplayTexture (mipMapDistTexId[0], mipMap_w, mipMap_h);

	glTranslatef(mipMap_w, 0, 0.0f);
	DisplayTexture (mipMapDistTexId[1], mipMap_w, mipMap_h);

	
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
/*------------------------------------------------------------------------*/
void cGILod::calcDistMipMap ()
{
	geometryImageRTTMipMapDist->Bind ();
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);// Draw into the first texture
		glCallList (SET_PROJECTION1_0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_RECTANGLE_ARB, mipMapTexId[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture (GL_TEXTURE_RECTANGLE_ARB, mipMapTexId[1]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture (GL_TEXTURE_RECTANGLE_ARB, mipMapDistTexId[1]);
		calcDistanceMipMap->activate ();
			DrawQuad(mipMap_w, mipMap_h, mipMap_w, mipMap_h);
		calcDistanceMipMap->deactivate ();
		glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_RECTANGLE_ARB, 0);
		glCallList (SET_PROJECTION1_1);

		geometryImageRTVVertex->Read (GL_COLOR_ATTACHMENT0_EXT, mipMap_w, mipMap_h);
//		geometryImageRTVNormal->Read (GL_COLOR_ATTACHMENT1_EXT,  mipMap_w, mipMap_h);

	geometryImageRTTMipMapDist->Disable ();

	glClearColor (1,1,1,1);
}
/*------------------------------------------------------------------------*/
void cGILod::upDateCameraPosAndDir (float *camPos_, float *camDir_)
{
	calcDistanceMipMap->activate ();
		calcDistanceMipMap->setUniformfv ("camPos", camPos_, 3);
		calcDistanceMipMap->setUniformfv ("camDir", camDir_, 3);
	glUseProgram (0);	
}