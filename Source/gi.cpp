
#pragma warning ( disable : 4244 )

#include <fstream>
#include <ios>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include "../Include/GI.h"


using namespace std;


cGI::cGI (int width, int height)
{
	register x,y;
	m_w = width; m_h = height;
	geometryImageTexId[0] = geometryImageTexId[1] = 0;
	geometryImageRTT  = new FramebufferObject ();
	CheckErrorsGL("BEGIN : Creating textures");
	geometryImageRTT->Bind(); // Bind framebuffer object.
		glGenTextures(2, geometryImageTexId); // create (reference to) a new texture
		for(x=0;x<2;x++) {
			glBindTexture(GL_TEXTURE_RECTANGLE_ARB, geometryImageTexId[x]);
			// (set texture parameters here)
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
			//create the texture
			glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_FLOAT_RGB32_NV, m_w, m_h,
							0, GL_RGB, GL_FLOAT, 0);
		}
		CheckErrorsGL("END : Creating textures");	
		CheckErrorsGL("BEGIN : Configuring FBO");
		// Attach texture to framebuffer color buffer
		geometryImageRTT->AttachTexture(GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, geometryImageTexId[0]);
		geometryImageRTT->AttachTexture(GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_RECTANGLE_ARB, geometryImageTexId[1]);
		geometryImageRTT->IsValid();
	geometryImageRTT->Disable ();
	CheckErrorsGL("END : Configuring FBO");

	geometryImageRTVVertex	 = new RenderVertexArray (m_w*m_h, 3, GL_HALF_FLOAT_NV);
	geometryImageRTVNormal	 = new RenderVertexArray (m_w*m_h, 3, GL_HALF_FLOAT_NV);

	numIndices = ((m_h-1) * (m_w+1) * 2);
	indices = new unsigned int [numIndices];
	unsigned int	*ptr = indices;
	register aux1, aux2;
	for(y=0; y<m_h-1; y++) {
		for(x=0; x<m_w; x++) {
			if (x==0) {
				aux1 = (y+1)*m_w + x;
				aux2 = y*m_w + x;
			}
			*ptr++ = (y+1)*m_w + x;
			*ptr++ = y*m_w + x;
		}
		*ptr++ = aux1;
		*ptr++ = aux2;
	}

	displayFloatTex = new ShaderObject ("passthruTexVertex.glsl", "passthruTexPixel.glsl");
	displayFloatTex->activate ();
		displayFloatTex->setUniformi ("tex", 0);
	displayFloatTex->deactivate ();
    
	lPos[0] = 0.0;
	lPos[1] = 2.0;
	lPos[2] = 6.0;
	lPos[3] = 0.0;

	displayGI = new ShaderObject ("displayGIVertex.glsl","displayGIPixel.glsl");
	
	displayGI->activate ();
		displayGI->setUniformfv	("lPos", lPos, 4);
		displayGI->setUniformf ("diffuseCoeff", 0.6f);
		//glUniform1f (glGetUniformLocation (displayGI->shader_id, "diffuseCoeff"), 0.6f);
	displayGI->deactivate ();

	genGI = new ShaderObject ("renderPBufferVertex.glsl", "renderPBufferFragment.glsl");
	genGI->activate ();
		genGI->setUniformf ("scaleFactor", 1.0);
	genGI->deactivate ();

	calcNormals = new ShaderObject ("passthruTexVertex.glsl", "calcNormalPixel.glsl");
	calcNormals->activate ();
		calcNormals->setUniformi ("GI", 0);
	calcNormals->deactivate ();

    SET_PROJECTION0_0 = glGenLists(1);
	glNewList (SET_PROJECTION0_0, GL_COMPILE);
	    glMatrixMode(GL_PROJECTION);
		glPushMatrix();
	    glLoadIdentity();
        glGetIntegerv(GL_VIEWPORT, vp);
		glViewport (0,0, m_w,m_h);
		glOrtho(0.0, m_w, 0.0, m_h, -1,1);
		glMatrixMode (GL_MODELVIEW);
		glLoadIdentity ();
	glEndList ();

	SET_PROJECTION0_1 = glGenLists(1);
	glNewList (SET_PROJECTION0_1, GL_COMPILE);
		glMatrixMode(GL_PROJECTION);
		glPopMatrix ();
		glMatrixMode(GL_MODELVIEW);
        glViewport(vp[0], vp[1], vp[2], vp[3]);
	glEndList ();

	ENABLE_VBO_STUFF = glGenLists(1);
	glNewList (ENABLE_VBO_STUFF, GL_COMPILE);
		glEnable (GL_BLEND);
		glEnable (GL_ALPHA_TEST);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glAlphaFunc (GL_NOTEQUAL, 0.0);
	glEndList ();
	
	DISABLE_VBO_STUFF = glGenLists(1);
	glNewList (DISABLE_VBO_STUFF, GL_COMPILE);
		glDisable (GL_ALPHA_TEST);
		glDisable (GL_BLEND);		
	glEndList ();

	model=NULL;

}
/*------------------------------------------------------------------------*/
cGI::~cGI ()
{
	geometryImageRTT->Unattach (GL_COLOR_ATTACHMENT0_EXT);
	geometryImageRTT->Unattach (GL_COLOR_ATTACHMENT1_EXT);

	FREE_MEMORY(geometryImageRTT);
	FREE_MEMORY(geometryImageRTVVertex);
	FREE_MEMORY(geometryImageRTVNormal);
	FREE_MEMORY(indices);
	FREE_MEMORY(model);

	FREE_MEMORY(displayFloatTex);
	FREE_MEMORY(displayGI);
	FREE_MEMORY(genGI);
	
	glDeleteLists (SET_PROJECTION0_0, SET_PROJECTION0_0+DISABLE_VBO_STUFF);
		
	glDeleteTextures (2, geometryImageTexId);
	
}
/*------------------------------------------------------------------------*/
int cGI::generateGI (char *filename)
{
	model = new ModelObject ();
	if (model->LoadOGL (filename)==FILE_NOT_FOUND){
		FREE_MEMORY (model);
		return FILE_NOT_FOUND;
	}
	model->Normalize ();

	geometryImageRTT->Bind(); 

		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);// Draw into the first texture
		glClearColor (0.0,0.0,0.0,1);
		glClear (GL_COLOR_BUFFER_BIT);
		glCallList (SET_PROJECTION0_0);
		genGI->activate ();
			genGI->setUniformf ("scaleFactor", 1.0);
			glScalef (m_w, m_h, 0.0f);
			model->Display(DISPLAY_GI);
			glLoadIdentity ();
			glFlush ();
		genGI->deactivate ();

		glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);//Draw into the second texture
	    
		glLoadIdentity ();
		genGI->activate ();
			glScalef (m_w, m_h, 0);
			model->Display(DISPLAY_NORMALS);
			glLoadIdentity ();
			glFlush ();
		genGI->deactivate ();

		geometryImageRTVVertex->Read (GL_COLOR_ATTACHMENT0_EXT, m_w, m_h);
		geometryImageRTVNormal->Read (GL_COLOR_ATTACHMENT1_EXT, m_w, m_h);
		glCallList (SET_PROJECTION0_1);
	geometryImageRTT->Disable ();
	
	glClearColor (1,1,1,1);

	return OK;
}
/*------------------------------------------------------------------------*/
void cGI::generateGIfromBuffer (Array2D<rgb> &pixels)
{
	unsigned int pixelTex=0;
	glGenTextures (1, &pixelTex);
	glBindTexture (GL_TEXTURE_RECTANGLE_ARB, pixelTex);
	glTexImage2D  (GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGB32_NV, m_w, m_h, 0, GL_RGB, GL_FLOAT, &pixels[0][0]);
	
	geometryImageRTT->Bind ();
		glClearColor (0.0,0.0,0.0,1);
		glClear (GL_COLOR_BUFFER_BIT);
		glCallList (SET_PROJECTION0_0);
		glDrawBuffer (GL_COLOR_ATTACHMENT0_EXT);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, pixelTex);
		displayFloatTex->activate ();
			DrawQuad(m_w, m_h, m_w, m_h);
		displayFloatTex->deactivate ();
		geometryImageRTVVertex->Read (GL_COLOR_ATTACHMENT0_EXT, m_w, m_h);

		glDrawBuffer (GL_COLOR_ATTACHMENT1_EXT);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, geometryImageTexId[0]);
		calcNormals->activate ();
			DrawQuad (m_w, m_h, m_w, m_h);
		calcNormals->deactivate ();
		geometryImageRTVNormal->Read (GL_COLOR_ATTACHMENT1_EXT, m_w, m_h);
		glCallList (SET_PROJECTION0_1);
	geometryImageRTT->Disable ();

	glDeleteTextures(1, &pixelTex);
	glClearColor (1,1,1,1);
}
/*------------------------------------------------------------------------*/
void cGI::draw (int primitive)
{
	geometryImageRTVVertex->SetPointer (VERTEX);
	geometryImageRTVNormal->SetPointer (NORMAL);
		
	glEnableVertexAttribArrayARB(VERTEX);
	glEnableVertexAttribArrayARB(NORMAL);
	glCallList (ENABLE_VBO_STUFF);
	
	displayGI->activate ();
		if (primitive==GL_POINTS) {
			glDrawArrays(primitive, 0, m_w*m_h);
		}
		else{
			glDrawElements(primitive, numIndices, GL_UNSIGNED_INT, indices);
		}
	displayGI->deactivate ();
	
	glDisableVertexAttribArrayARB(VERTEX);
	glDisableVertexAttribArrayARB(NORMAL);
	glCallList (DISABLE_VBO_STUFF);
		
}
/*------------------------------------------------------------------------*/
void cGI::DrawQuad (int w, int h, int tw, int th)
{
	glBegin(GL_QUADS);
	    glTexCoord2f(0,         0);         glVertex2f(0,        0);
		glTexCoord2f((float)tw, 0);         glVertex2f((float)w, 0);
		glTexCoord2f((float)tw, (float)th); glVertex2f((float)w, (float) h);
		glTexCoord2f(0,         (float)th); glVertex2f(0,        (float) h);
	glEnd();
}
/*------------------------------------------------------------------------*/
void cGI::DisplayTexture (GLuint texId, int w, int h)
{
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texId);
	displayFloatTex->activate ();
			DrawQuad(w, h, w, h);
	displayFloatTex->deactivate ();
}
/*------------------------------------------------------------------------*/
void cGI::DisplayTextures()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, m_w, 0.0, m_h, -1.0, 1.0);
     
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glScalef(0.25f, 0.25f, 1.0f);	 
	DisplayTexture(geometryImageTexId[0] , m_w, m_h);

	glTranslatef(m_w+5, 0.0f, 0.0f);
	DisplayTexture(geometryImageTexId[1] , m_w, m_h);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
/*------------------------------------------------------------------------*/
void cGI::updateLPos (int c)
{
	switch (c) {
		case 'x':
			lPos[0]--;
			break;
		case 'X':
			lPos[0]++;
			break;
		case 'y':
			lPos[1]--;
			break;
		case 'Y':
			lPos[1]++;
			break;
		case 'z':
			lPos[2]--;
			break;
		case 'Z':
			lPos[2]++;
			break;
	}
	displayGI->activate ();
		displayGI->setUniformfv ("lPos", lPos,4);
	displayGI->deactivate ();
}
/*------------------------------------------------------------------------*/
void cGI::delete3DModel ()
{
	FREE_MEMORY (model);
}
/*------------------------------------------------------------------------*/
/*
void cGI::applyBlur ()
{
   	geometryImageRTT->Bind(); 
		glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);// Draw into the first texture
		glCallList (SET_PROJECTION0_0);
		
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, geometryImageTexId[1]);
		
		blur3x3->activate ();
				DrawQuad(m_w, m_h, m_w, m_h);
		blur3x3->deactivate ();
		geometryImageRTVNormal->Read (GL_COLOR_ATTACHMENT1_EXT, m_w, m_h);

		glCallList (SET_PROJECTION0_1);
	geometryImageRTT->Disable ();
}
/*------------------------------------------------------------------------*/
void cGI::getGI (Array2D<rgb> &pixels)
{
	geometryImageRTT->Bind ();
		glReadBuffer (GL_COLOR_ATTACHMENT0_EXT);
		glReadPixels (0,0, m_w, m_h, GL_RGB,GL_FLOAT, &pixels[0][0]);
	geometryImageRTT->Disable ();
}
/*------------------------------------------------------------------------*/
void cGI::displayOGL ()
{
	model->Display (DISPLAY_NORMAL);
}
/*------------------------------------------------------------------------*/