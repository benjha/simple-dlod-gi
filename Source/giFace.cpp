/*
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
*/

#pragma warning ( disable : 4244 )

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <fstream>
#include <ios>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include "../Include/giFace.h"
#include <GL/glut.h>
#include "../Include/BmpImage.h"
//#include "paintInterfaceIds.h"

using namespace std;
/*-------------------------------------------------------------------------------------------------*/
cGIface::cGIface (int width, int height)
{
	register x,y;

//	RenderTexture(char *strMode, int iWidth, int iHeight, GLenum target) : m_target(target)
	m_w =width;
	m_h=height;

	geometryImageRTT  = new RenderTexture ("float=16 rgb texture double", width, height, GL_TEXTURE_RECTANGLE_NV); 
	geometryImageTemp = new RenderTexture ("float=16 rgb texture double", width, height, GL_TEXTURE_RECTANGLE_NV); 
	gaussianAccum	  = new RenderTexture ("float=16 rgb texture double", width, height, GL_TEXTURE_RECTANGLE_NV);
	//RenderVertexArray(int nverts, GLint size, GLenum type = GL_FLOAT);
	geometryImageRTVVertex	 = new RenderVertexArray (width*height, 3, GL_HALF_FLOAT_NV);
	geometryImageRTVNormal	 = new RenderVertexArray (width*height, 3, GL_HALF_FLOAT_NV);
	geometryImageRTVTextures0 = new RenderVertexArray (width*height, 3, GL_FLOAT);

	numIndices = ((m_h-1) * (m_w+1) * 2);
	indices = new unsigned int [numIndices];
	unsigned int	*ptr = indices;
	for(y=0; y<m_h-1; y++) {
		for(x=0; x<m_w; x++) {
			*ptr++ = (y+1)*m_w + x;
			*ptr++ = y*m_w + x;
		}
		*ptr++ = (y+1)*m_w;
		*ptr++ = (y+1)*m_w + x-1;
	}

	float *texCoords = new float [m_h*m_w*3];
    float *tCoords = texCoords;
    for(y=0; y<m_h; y++) {
        for(int x=0; x<m_w; x++) {
            *tCoords++ = x*1.0;
            *tCoords++ = y*1.0;
			*tCoords++ = 0.0;
        }
    }
	kernel = NULL;
	geometryImageRTVTextures0->LoadData (texCoords);
	kernelTextureHId =0;
	offsetTexture = 0;
	delete texCoords, texCoords = NULL;

	BmpImage *texture = new BmpImage;
	texture->readBmpFile ("male.bmp");
	glGenTextures (1, &faceTexId);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, faceTexId);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D (GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, texture->width, texture->height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture->data);
	texture->readBmpFile ("maleNormalMap.bmp");
	glGenTextures (1, &normalMapId);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, normalMapId);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D (GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, texture->width, texture->height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture->data);
	delete texture;

	normalFlag=0; vertexFlag=0; jawId=0; expressionCont = 0;
	muscleParams = NULL;

	passthruShader = initShaders ("passthruVertex.glsl", "passthruPixel.glsl");
	glUseProgramObjectARB (passthruShader);
		glUniform1iARB (glGetUniformLocationARB (passthruShader, "GI"), 0);
		glUniform1iARB (glGetUniformLocationARB (passthruShader, "faceTex"), 1);
		glUniform1iARB (glGetUniformLocationARB (passthruShader, "normalMap"), 2);
		glUniform1iARB (glGetUniformLocationARB (passthruShader, "skullGI"), 3);
	glUseProgramObjectARB (0);

	renderTextured = initShaders ("passthruVertex.glsl", "passthruPixel2.glsl");
	glUseProgramObjectARB (renderTextured);
		glUniform1iARB (glGetUniformLocationARB (renderTextured, "GI"), 0);
		glUniform1iARB (glGetUniformLocationARB (renderTextured, "faceTex"), 1);
		glUniform1iARB (glGetUniformLocationARB (renderTextured, "skullGI"), 3);
	glUseProgramObjectARB (0);

	genGIAndSkull = initShaders ("renderPBufferVertex.glsl", "renderPBufferFragment.glsl");
	scaleFactor = glGetUniformLocationARB (genGIAndSkull, "scaleFactor");

	addCanvasAndGI = initShaders ("addCanvasAndGIVertex.glsl", "addCanvasAndGIPixel.glsl");
	glUseProgramObjectARB (addCanvasAndGI);
		glUniform1iARB (glGetUniformLocationARB (addCanvasAndGI, "GI"), 0);
		glUniform1iARB (glGetUniformLocationARB (addCanvasAndGI, "canvas"), 1);
	glUseProgramObjectARB (0);

	moveFeaturePoints = initShaders ("moveFeaturePointsVertex.glsl", "moveFeaturePointsPixel.glsl");
	moveXYZ = glGetUniformLocationARB (moveFeaturePoints, "moveXYZ");
	normal	= glGetUniformLocationARB (moveFeaturePoints, "normal");
	tangent = glGetUniformLocationARB (moveFeaturePoints, "tangent");
	degrees = glGetUniformLocationARB (moveFeaturePoints, "grados");

	glUseProgramObjectARB (moveFeaturePoints);
		glUniform1iARB (glGetUniformLocationARB (moveFeaturePoints, "GI"), 0);
		glUniform1iARB (glGetUniformLocationARB (moveFeaturePoints, "FPs"), 1);
		glUniform1iARB (glGetUniformLocationARB (moveFeaturePoints, "Normals"), 2);
		glUniform1iARB	(glGetUniformLocationARB (moveFeaturePoints, "Jaw"), 3);
	glUseProgramObjectARB (0);

	makeFaceHoles = initShaders ("makeFaceHolesVertex.glsl","makeFaceHolesPixel.glsl");
	glUseProgramObjectARB (makeFaceHoles);
		glUniform1iARB (glGetUniformLocationARB (makeFaceHoles, "GI"), 0);
		glUniform1iARB (glGetUniformLocationARB (makeFaceHoles, "faceHoles"), 1);
	glUseProgramObjectARB (0);

	gaussFiltering3x3 = initShaders ("addCanvasAndGIVertex.glsl","convolutionPixel3x3.glsl");
	glUseProgramObjectARB (gaussFiltering3x3);
		glUniform1iARB (glGetUniformLocationARB (gaussFiltering3x3, "kernelH"), 0);	
		glUniform1iARB (glGetUniformLocationARB (gaussFiltering3x3, "canvas"), 1);
		glUniform1iARB (glGetUniformLocationARB (gaussFiltering3x3, "gaussFiltering"), 1);
	glUseProgramObjectARB (0);

	gaussFiltering5x5 = initShaders ("addCanvasAndGIVertex.glsl","convolutionPixel5x5.glsl");
	glUseProgramObjectARB (gaussFiltering5x5);
		glUniform1iARB (glGetUniformLocationARB (gaussFiltering5x5, "kernelH"), 0);	
		glUniform1iARB (glGetUniformLocationARB (gaussFiltering5x5, "canvas"), 1);
		glUniform1iARB (glGetUniformLocationARB (gaussFiltering5x5, "gaussFiltering"), 1);
	glUseProgramObjectARB (0);

	gaussFiltering7x7 = initShaders ("addCanvasAndGIVertex.glsl","convolutionPixel7x7.glsl");
	glUseProgramObjectARB (gaussFiltering7x7);
		glUniform1iARB (glGetUniformLocationARB (gaussFiltering7x7, "kernelH"), 0);	
		glUniform1iARB (glGetUniformLocationARB (gaussFiltering7x7, "canvas"), 1);
		glUniform1iARB (glGetUniformLocationARB (gaussFiltering7x7, "gaussFiltering"), 1);
	glUseProgramObjectARB (0);

	gaussFiltering9x9 = initShaders ("addCanvasAndGIVertex.glsl","convolutionPixel9x9.glsl");
	glUseProgramObjectARB (gaussFiltering9x9);
		glUniform1iARB (glGetUniformLocationARB (gaussFiltering9x9, "kernelH"), 0);	
		glUniform1iARB (glGetUniformLocationARB (gaussFiltering9x9, "canvas"), 1);
		glUniform1iARB (glGetUniformLocationARB (gaussFiltering9x9, "gaussFiltering"), 1);
	glUseProgramObjectARB (0);

	gaussFiltering11x11 = initShaders ("addCanvasAndGIVertex.glsl","convolutionPixel11x11.glsl");
	glUseProgramObjectARB (gaussFiltering11x11);
		glUniform1iARB (glGetUniformLocationARB (gaussFiltering11x11, "kernelH"), 0);	
		glUniform1iARB (glGetUniformLocationARB (gaussFiltering11x11, "canvas"), 1);
		glUniform1iARB (glGetUniformLocationARB (gaussFiltering11x11, "gaussFiltering"), 1);
	glUseProgramObjectARB (0);

	gaussFiltering21x21 = initShaders ("addCanvasAndGIVertex.glsl","convolutionPixel21x21.glsl");
	glUseProgramObjectARB (gaussFiltering21x21);
		glUniform1iARB (glGetUniformLocationARB (gaussFiltering21x21, "kernelH"), 0);	
		glUniform1iARB (glGetUniformLocationARB (gaussFiltering21x21, "canvas"), 1);
		glUniform1iARB (glGetUniformLocationARB (gaussFiltering21x21, "gaussFiltering"), 1);
	glUseProgramObjectARB (0);

	passthruTex = initShaders ("passthruTexVertex.glsl", "passthruTexPixel.glsl");
	glUseProgramObjectARB (passthruTex);
		glUniform1iARB (glGetUniformLocationARB (passthruTex, "tex"), 0);
	glUseProgramObjectARB (0);

	openJawShader = initShaders ("openJawVertex.glsl", "openJawPixel.glsl");
	degreesJaw = glGetUniformLocationARB(openJawShader, "grados");
	glUseProgramObjectARB (openJawShader);
		glUniform1iARB (glGetUniformLocationARB (openJawShader, "GI"),  0);
		glUniform1iARB (glGetUniformLocationARB (openJawShader, "jaw"), 1);
	glUseProgramObjectARB (0);

	calcNormal = initShaders ("openJawVertex.glsl","calcNormalPixel.glsl");
	glUseProgramObjectARB (calcNormal);
		glUniform1iARB (glGetUniformLocationARB (calcNormal, "GI"),  0);
	glUseProgramObjectARB (0);

	animateShader = initShaders ("openJawVertex.glsl","animatePixel.glsl");
	muscleParamsAnimate1 = glGetUniformLocationARB (animateShader, "muscleParams");
	muscleParamsAnimate2 = glGetUniformLocationARB (animateShader, "muscleParams2");
	deltaTimeAnimate	 = glGetUniformLocationARB (animateShader, "deltaTime");

	glUseProgramObjectARB (animateShader);
		glUniform1iARB (glGetUniformLocationARB (animateShader, "GI"),  0);
		glUniform1iARB (glGetUniformLocationARB (animateShader, "MIZ"),  1);
		glUniform1iARB (glGetUniformLocationARB (animateShader, "Normals"),  2);
	glUseProgramObjectARB (0);

}

/*-------------------------------------------------------------------------------------------------*/
cGIface::~cGIface()
{
	if (geometryImageRTVVertex) 
		delete geometryImageRTT,
		geometryImageRTT=NULL;

	if (geometryImageTemp) 
		delete geometryImageTemp,
		geometryImageTemp=NULL;
	
	if (gaussianAccum)
		delete gaussianAccum,
		gaussianAccum= NULL;

	if (geometryImageRTVVertex) {
		delete geometryImageRTVVertex;
		geometryImageRTVVertex=NULL;
	}
	if (geometryImageRTVNormal) {
		delete geometryImageRTVNormal;
		geometryImageRTVNormal=NULL;
	}
	if (geometryImageRTVTextures0){
		delete geometryImageRTVTextures0;
		geometryImageRTVTextures0 = NULL;
	}	

	if (indices)
		delete	indices; indices=NULL;

	if (kernel) delete  kernel, kernel = NULL;

	if (passthruShader) {
		glDeleteObjectARB (passthruShader);
		glDeleteObjectARB (genGIAndSkull);
		glDeleteObjectARB (addCanvasAndGI);
		glDeleteObjectARB (Handler);
		glDeleteObjectARB (gaussFiltering3x3);
		glDeleteObjectARB (gaussFiltering5x5);
		glDeleteObjectARB (gaussFiltering7x7);
		glDeleteObjectARB (gaussFiltering9x9);
		glDeleteObjectARB (gaussFiltering11x11);
		glDeleteObjectARB (gaussFiltering21x21);
		glDeleteObjectARB (moveFeaturePoints);
		glDeleteObjectARB (makeFaceHoles);
		glDeleteObjectARB (passthruTex);
		glDeleteObjectARB (openJawShader);
		glDeleteObjectARB (renderTextured);
		glDeleteObjectARB (animateShader);
		glDeleteObjectARB (calcNormal);
	}

	passthruShader = genGIAndSkull = addCanvasAndGI = vertexHandler = openJawShader = renderTextured = 0;
	Handler = gaussFiltering3x3 = gaussFiltering5x5 = gaussFiltering7x7 = passthruTex = 0;
	gaussFiltering9x9 = gaussFiltering11x11 = gaussFiltering21x21 =	moveFeaturePoints=	makeFaceHoles =0;
	animateShader = calcNormal = 0;
	
	unsigned int id = 1;
	glDeleteTextures (1, &id);
	glDeleteTextures (1, &faceTexId);
	glDeleteTextures (1, &normalMapId);
	glDeleteTextures (1, &jawId);
	glDeleteTextures (1, &kernelTextureHId);

	if (kernel) delete [] kernel, kernel = NULL;
}

/*-------------------------------------------------------------------------------------------------*/
char *cGIface::loadShaderCode (char *fileName)
{	
	std::ifstream file;
	unsigned int i=0;
	unsigned long len;
	int result;
	char* cheiderSource;
#ifdef WIN32
    struct _stat f_status;
#else
    struct stat f_status;
#endif
	file.open(fileName, std::ios_base::in);
	if(!file) 
		return NULL;
	result = _stat (fileName, &f_status);
	if (result!=0)
		return NULL;
	len = f_status.st_size;
	if (len==0) 
		return NULL; 
	cheiderSource = new char[len+1];
	memset(cheiderSource, ' ', len+1);
	if (cheiderSource == NULL) 
		return NULL;
	
	cheiderSource[len] = 0; 
	while (file.good()) {   
		cheiderSource[i++] = file.get(); 
		if (i>len) i=len;
	}
	cheiderSource[i] = NULL; 
	file.close ();
	return  cheiderSource;
}
/*-------------------------------------------------------------------------------------------------*/
GLhandleARB cGIface::initShaders (char *fileNameVertex, char *fileNameFragment)
{
	const char *Shader;
	GLhandleARB vertexHandler, fragmentHandler;
	GLhandleARB programObject;

	if ((Shader = (const char *)loadShaderCode (fileNameVertex))==NULL){
		fprintf(stderr, "Unable to Load Vertex Shader\n");
		exit (1);
	}
	vertexHandler = glCreateShaderObjectARB (GL_VERTEX_SHADER_ARB);
	glShaderSourceARB	(vertexHandler  , 1, &Shader, NULL);
	delete [] Shader;
	Shader = NULL;
	if ((Shader = (const char *)loadShaderCode (fileNameFragment))==NULL){
		fprintf(stderr, "Unable to Load  Shader\n");
		exit (1);
	}
	fragmentHandler = glCreateShaderObjectARB (GL_FRAGMENT_SHADER_ARB);
	glShaderSourceARB (fragmentHandler  , 1, &Shader, NULL);

	delete [] Shader;
	Shader = NULL;

	glCompileShaderARB	(vertexHandler);
	glCompileShaderARB (fragmentHandler);

	programObject = glCreateProgramObjectARB ();
	glAttachObjectARB (programObject, vertexHandler);
	glAttachObjectARB (programObject, fragmentHandler);
	
	glLinkProgramARB (programObject);

	glDetachObjectARB (programObject,vertexHandler);
	glDetachObjectARB (programObject,fragmentHandler);

	glDeleteObjectARB(vertexHandler);
	glDeleteObjectARB(fragmentHandler);

	
	int progLinkSuccess;
    glGetObjectParameterivARB(programObject, GL_OBJECT_LINK_STATUS_ARB, &progLinkSuccess);
	
	if (!progLinkSuccess) {

		glGetObjectParameterivARB(programObject, GL_OBJECT_INFO_LOG_LENGTH_ARB, &progLinkSuccess);
		Shader = new const char [progLinkSuccess];
		glGetInfoLogARB (programObject, progLinkSuccess, NULL, (char*)Shader);
		fprintf (stdout, "\n%s\n", Shader);
		fprintf(stderr, "Shader could not be linked\n");
		delete [] Shader;
		Shader = NULL;
        exit(1);
	}

	return programObject;
}
/*-------------------------------------------------------------------------------------------------*/
void cGIface::setRenderMode (int renderMode_)
{
/*	switch (renderMode_) {
		case ID_TEXTURED: case ID_REALISTIC:
			glPolygonMode (GL_FRONT, GL_FILL);
			glUseProgramObjectARB (passthruShader);
				glUniform1i	(renderMode, renderMode_);
			glUseProgramObjectARB (0);
			break;
	}
*/	
}
/*-------------------------------------------------------------------------------------------------*/
void cGIface::draw (int primitive)
{
	if (vertexFlag==1) {
		geometryImageTemp->Activate ();
			geometryImageRTVVertex->Read (GL_FRONT, geometryImageTemp->GetWidth(), geometryImageTemp->GetHeight());
		geometryImageTemp->Deactivate ();
		vertexFlag=0;
	}

	if (normalFlag==1) {
		geometryImageRTT->Activate ();
			geometryImageRTVNormal->Read (GL_BACK,  geometryImageRTT->GetWidth(), geometryImageRTT->GetHeight());
		geometryImageRTT->Deactivate ();
		normalFlag=0;
	}

	glActiveTexture(GL_TEXTURE0);
	geometryImageTemp->Bind (WGL_FRONT_LEFT_ARB);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, faceTexId);

//	glActiveTexture(GL_TEXTURE2);
//	geometryImageRTT->Bind (WGL_BACK_LEFT_ARB);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, normalMapId);

	glActiveTexture(GL_TEXTURE3);
	geometryImageTemp->Bind (WGL_BACK_LEFT_ARB);

	geometryImageRTVVertex->SetPointer (VERTEX);
	geometryImageRTVNormal->SetPointer (NORMAL);
	geometryImageRTVTextures0->SetPointer (TEX0);
		
	glEnableVertexAttribArrayARB(VERTEX);
	glEnableVertexAttribArrayARB(NORMAL);
	glEnableVertexAttribArrayARB(TEX0);
	
	glUseProgramObjectARB (passthruShader);		
		glDrawElements(primitive, numIndices, GL_UNSIGNED_INT, indices);
	glUseProgramObjectARB (0);		
	
	glDisableVertexAttribArrayARB(VERTEX);
	glDisableVertexAttribArrayARB(NORMAL);
	glDisableVertexAttribArrayARB(TEX0);
		
	geometryImageTemp->Release (WGL_FRONT_LEFT_ARB);
//	geometryImageRTT->Release (WGL_BACK_LEFT_ARB);
	geometryImageTemp->Release (WGL_BACK_LEFT_ARB);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, 0);
/*
	glActiveTexture(GL_TEXTURE1);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, 0);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, 0);
*/
}
/*-------------------------------------------------------------------------------------------------*/
#define ID_FROM_FILE 1
void cGIface::generateGIface (ModelObject *model)
{
	geometryImageRTT->Activate();
		
		glDrawBuffer (GL_FRONT);

		glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    glOrtho(0.0, m_w+m_w/2, 0.0, m_h, -1.0, 1.0);
	    glMatrixMode(GL_MODELVIEW);

		glClearColor (0.0,0,0,1);
		glClear (GL_COLOR_BUFFER_BIT);
	    glViewport(0, 0, m_w, m_h);

		glUseProgramObjectARB (genGIAndSkull);
			glUniform1fARB (scaleFactor, 1.0);
			glScalef (GI_WIDTH, GI_HEIGHT, 0);
			model->Display(DISPLAY_GI);
			glLoadIdentity ();
			glFlush ();
		glUseProgramObjectARB (0);

		glDrawBuffer (GL_BACK);
		
	    glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    glOrtho(0.0, m_w+m_w/2, 0.0, m_h, -1.0, 1.0);
	    glMatrixMode(GL_MODELVIEW);

		glClearColor (0.0,0,0,1);
		glClear (GL_COLOR_BUFFER_BIT);
	    glViewport(0, 0, m_w, m_h);

		glUseProgramObjectARB (genGIAndSkull);
			glScalef (GI_WIDTH, GI_HEIGHT, 0);
			model->Display(DISPLAY_NORMALS);
			glLoadIdentity ();
			glFlush ();
		glUseProgramObjectARB (0);

	geometryImageRTT->Deactivate();

	geometryImageTemp->Activate();
		glDrawBuffer (GL_FRONT);
		glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    glOrtho(0.0, m_w, 0.0, m_h, -1.0, 1.0);
	    glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glClearColor (0.0,0,0,1);
		glClear (GL_COLOR_BUFFER_BIT);

		glDrawBuffer (GL_BACK);
		glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    glOrtho(0.0, m_w, 0.0, m_h, -1.0, 1.0);
	    glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glClearColor (0.0,0,0,1);
		glClear (GL_COLOR_BUFFER_BIT);

		glUseProgramObjectARB (genGIAndSkull);
			glScalef (GI_WIDTH, GI_HEIGHT, 0);
			glUniform1fARB (scaleFactor, 0.97);
			model->Display(DISPLAY_GI);
			glLoadIdentity ();
			glFlush ();
			glUniform1fARB (scaleFactor, 1.0);
		glUseProgramObjectARB(0);
	geometryImageTemp->Deactivate ();

	gaussianAccum->Activate ();
		glDrawBuffer (GL_FRONT);
		glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    glOrtho(0.0, m_w, 0.0, m_h, -1.0, 1.0);
	    glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glClearColor (0.0,0,0,1);
		glClear (GL_COLOR_BUFFER_BIT);
		glDrawBuffer (GL_BACK);
		glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    glOrtho(0.0, m_w, 0.0, m_h, -1.0, 1.0);
	    glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glClearColor (0.0,0,0,1);
		glClear (GL_COLOR_BUFFER_BIT);
	gaussianAccum->Deactivate ();

	makeFacialHoles (ID_FROM_FILE);

	geometryImageRTT->Activate ();
		geometryImageRTVVertex->Read (GL_FRONT, geometryImageRTT->GetWidth(), geometryImageRTT->GetHeight());
	geometryImageRTT->Deactivate ();

	geometryImageRTT->Activate ();
		geometryImageRTVNormal->Read (GL_BACK,  geometryImageRTT->GetWidth(), geometryImageRTT->GetHeight());
	geometryImageRTT->Deactivate ();
}
/*-------------------------------------------------------------------------------------------------*/
void cGIface::DrawQuad(int w, int h, int tw, int th)
{
    glBegin(GL_QUADS);

	glMultiTexCoord2f (GL_TEXTURE0, 0,0); 
	glMultiTexCoord2f (GL_TEXTURE1, 0,0); 
	glVertex2f(0,        0);

	glMultiTexCoord2f (GL_TEXTURE0, (float)tw,0); 
	glMultiTexCoord2f (GL_TEXTURE1, (float)tw,0); 
	glVertex2f((float)w, 0);

	glMultiTexCoord2f (GL_TEXTURE0, (float)tw, (float)th); 
	glMultiTexCoord2f (GL_TEXTURE1, (float)tw, (float)th); 
	glVertex2f((float)w, (float) h);

	glMultiTexCoord2f (GL_TEXTURE0, 0,         (float)th); 
	glMultiTexCoord2f (GL_TEXTURE1, 0,         (float)th); 
	glVertex2f(0,         (float)h);
/*
	
    glTexCoord2f(0,         0);         glVertex2f(0,        0);
    glTexCoord2f((float)tw, 0);         glVertex2f((float)w, 0);
    glTexCoord2f((float)tw, (float)th); glVertex2f((float)w, (float) h);
    glTexCoord2f(0,         (float)th); glVertex2f(0,        (float) h);
*/	
	glEnd();
}
/*-------------------------------------------------------------------------------------------------*/
void cGIface::DrawQuad2(int w, int h, int tw, int th)
{
    glBegin(GL_QUADS);

	glMultiTexCoord2f (GL_TEXTURE0, 0,0); 
	glMultiTexCoord2f (GL_TEXTURE1, 0,0); 
	glMultiTexCoord2f (GL_TEXTURE2, 0,0); 
	glVertex2f(0,        0);

	glMultiTexCoord2f (GL_TEXTURE0, (float)tw,0); 
	glMultiTexCoord2f (GL_TEXTURE1, (float)tw,0); 
	glMultiTexCoord2f (GL_TEXTURE2, (float)tw,0); 
	glVertex2f((float)w, 0);


	glMultiTexCoord2f (GL_TEXTURE0, (float)tw, (float)th); 
	glMultiTexCoord2f (GL_TEXTURE1, (float)tw, (float)th); 
	glMultiTexCoord2f (GL_TEXTURE2, (float)tw, (float)th); 
	glVertex2f((float)w, (float) h);


	glMultiTexCoord2f (GL_TEXTURE0, 0,         (float)th); 
	glMultiTexCoord2f (GL_TEXTURE1, 0,         (float)th); 
	glMultiTexCoord2f (GL_TEXTURE2, 0,         (float)th); 
	glVertex2f(0,         (float)h);

	glEnd();
}
/*-------------------------------------------------------------------------------------------------*/
void cGIface::DisplayTexture(RenderTexture *pbuffer, GLenum buffer, int w, int h)
{
	pbuffer->Bind(buffer);
	glUseProgramObjectARB(passthruTex);
			DrawQuad(w+w/2, h, w, h);
	glUseProgramObjectARB(0);
	pbuffer->Release(buffer);
}
/*-------------------------------------------------------------------------------------------------*/
void cGIface::DisplayTexture(PBuffer *pbuffer, GLenum buffer, int w, int h)
{
	glUseProgramObjectARB(passthruTex);
	pbuffer->Bind(buffer);
		cGIface::DrawQuad(w, h, m_w, m_h);
	pbuffer->Release(buffer);
	glUseProgramObjectARB(0);
}
/*-------------------------------------------------------------------------------------------------*/
void cGIface::DisplayTextures()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, m_w, 0.0, m_h, -1.0, 1.0);
 
    // position
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glScalef(0.25f, 0.25f, 1.0f);	 
/*	
	DisplayTexture(geometryImageRTT, WGL_FRONT_LEFT_ARB, m_w, m_h);
	
	glTranslatef(m_w, 0.0f, 0.0f);
	DisplayTexture(geometryImageRTT, WGL_BACK_LEFT_ARB, m_w, m_h);

	glTranslatef(m_w, 0.0f, 0.0f);
*/	DisplayTexture(geometryImageTemp, WGL_FRONT_LEFT_ARB, m_w, m_h);

	/*
	glTranslatef(m_w, 0.0f, 0.0f);
	DisplayTexture(gaussianAccum, WGL_BACK_LEFT_ARB, m_w, m_h);
*/
//	if (expressionCont){*/
	//glTranslatef(m_w, 0.0f, 0.0f);
	//DisplayTexture(gaussianAccum, WGL_BACK_LEFT_ARB, m_w, m_h);
//	}

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
/*-------------------------------------------------------------------------------------------------*/
void cGIface::addGIAndCanvas (PBuffer *canvas)
{
	geometryImageTemp->Activate ();
		glDrawBuffer (GL_FRONT);
	
		glActiveTexture(GL_TEXTURE0);
		geometryImageRTT->Bind ();

		glActiveTexture(GL_TEXTURE1);
		canvas->Bind (WGL_FRONT_LEFT_ARB);

		glUseProgramObjectARB(addCanvasAndGI);
			DrawQuad(m_w, m_h, m_w, m_h);
		glUseProgramObjectARB(0);
		
		geometryImageRTT->Release ();
		canvas->Release (WGL_FRONT_LEFT_ARB);

	geometryImageTemp->Deactivate ();
}
/*-------------------------------------------------------------------------------------------------*/
void cGIface::setCanvas (PBuffer *canvas)
{
	gaussianAccum->Activate ();
		glDrawBuffer (GL_BACK);
		DisplayTexture(canvas, WGL_FRONT_LEFT_ARB, m_w, m_h);
	gaussianAccum->Deactivate ();
}
/*-------------------------------------------------------------------------------------------------*/
const float dosPiSQRT = 2.506628275;
void cGIface::makeGaussianfilter (int size, float sigma, PBuffer *canvas)
{
	sizeFilter = size;
	int u = (sizeFilter-1)/2;
	static int s=0;
	static float sigmaStatic = 0;
	register x;//, y;
	static char flag = 0;
	
	if (s!=size) {
		if (!kernel) {
			kernel = new float  [sizeFilter];
		}
		else {
			delete [] kernel, kernel = NULL;
			kernel = new float [sizeFilter];
		}
		flag = 1;
	}
	else 
		flag = 0;

	if (sigmaStatic!=sigma) {
		for (x=-u;x<=u;x++) {
			kernel[x+u]=exp(- (( x*x )/(2*sigma*sigma)) ) / (dosPiSQRT*sigma);
		}
		flag = 1;
	}
	
	if (!kernelTextureHId)
		glGenTextures (1, &kernelTextureHId);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, kernelTextureHId);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D  (GL_TEXTURE_RECTANGLE_NV, 0, GL_LUMINANCE, sizeFilter, 1, 0, GL_LUMINANCE, GL_FLOAT, kernel);

	if (flag) {
		gaussianAccum->Activate ();
			glDrawBuffer (GL_BACK);
			DisplayTexture(canvas, WGL_FRONT_LEFT_ARB, m_w, m_h);
		gaussianAccum->Deactivate ();
	}
	
	gaussianAccum->Activate ();
		glDrawBuffer (GL_FRONT);
		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_RECTANGLE_NV, kernelTextureHId);

		glActiveTexture (GL_TEXTURE1);
		gaussianAccum->Bind (WGL_BACK_LEFT_ARB);

		switch (size) {
			case 3:
				glUseProgramObjectARB(gaussFiltering3x3);
					DrawQuad(m_w, m_h, m_w, m_h);
				glUseProgramObjectARB(0);
				break;
			case 5:
				glUseProgramObjectARB(gaussFiltering5x5);
					DrawQuad(m_w, m_h, m_w, m_h);
				glUseProgramObjectARB(0);
				break;
			case 7:
				glUseProgramObjectARB(gaussFiltering7x7);
					DrawQuad(m_w, m_h, m_w, m_h);
				glUseProgramObjectARB(0);
				break;
			case 9:
				glUseProgramObjectARB(gaussFiltering9x9); 
					DrawQuad(m_w, m_h, m_w, m_h);
				glUseProgramObjectARB(0);
				break;
			case 11:
				glUseProgramObjectARB(gaussFiltering11x11);
					DrawQuad(m_w, m_h, m_w, m_h);
				glUseProgramObjectARB(0);
				break;
			case 21:
				glUseProgramObjectARB(gaussFiltering21x21);
					DrawQuad(m_w, m_h, m_w, m_h);
				glUseProgramObjectARB(0);
				break;
			}

		gaussianAccum->Release (WGL_BACK_LEFT_ARB);
		
		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_RECTANGLE_NV, 0);
	gaussianAccum->Deactivate ();

	gaussianAccum->Activate ();
		glDrawBuffer (GL_BACK);
		DisplayTexture(gaussianAccum, WGL_FRONT_LEFT_ARB, m_w, m_h);
	gaussianAccum->Deactivate ();

	s=sizeFilter;
	sigmaStatic=sigma;
}
/*-------------------------------------------------------------------------------------------------*/
void cGIface::moveFeaturePointsSet (int whatSet, PBuffer *canvas, float *move)
{
	geometryImageTemp->Activate ();
		glDrawBuffer (GL_FRONT);

		glActiveTexture(GL_TEXTURE0);
		geometryImageRTT->Bind ();

		glActiveTexture(GL_TEXTURE1);
		gaussianAccum->Bind (WGL_FRONT_LEFT_ARB);

		glUseProgramObjectARB(moveFeaturePoints);
			glUniform3fv	(moveXYZ,1, move);
			DrawQuad(m_w, m_h, m_w, m_h);
		glUseProgramObjectARB(0);
		
		geometryImageRTT->Release ();
		gaussianAccum->Release (WGL_FRONT_LEFT_ARB);

	geometryImageTemp->Deactivate ();
}
/*-------------------------------------------------------------------------------------------------*/
void cGIface::moveFeaturePointsSet (int whatSet, PBuffer *canvas, float norm, float tan, float deg)
{
	if (!jawId) {
		BmpImage *jawImage = new BmpImage ();
		jawImage->readBmpFile ("jaw.bmp");
		glGenTextures (1, &jawId);
		glBindTexture (GL_TEXTURE_RECTANGLE_NV, jawId);
		glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D  (GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB,jawImage->width,jawImage->height, 0, GL_RGB, GL_UNSIGNED_BYTE, jawImage->data);
		
		delete jawImage;
		jawImage = NULL;
	}

	geometryImageTemp->Activate ();
	
		glDrawBuffer (GL_FRONT);

		glActiveTexture(GL_TEXTURE0);
		geometryImageRTT->Bind (WGL_FRONT_LEFT_ARB); // vertex coords

		glActiveTexture(GL_TEXTURE1);
		glBindTexture (GL_TEXTURE_RECTANGLE_NV, jawId);

		glUseProgramObjectARB(openJawShader);
			glUniform1fARB	(degreesJaw, deg);	
			DrawQuad(m_w, m_h, m_w, m_h);
		glUseProgramObjectARB(0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture (GL_TEXTURE_RECTANGLE_NV, 0);
		
		geometryImageRTT->Release (WGL_FRONT_LEFT_ARB);
	geometryImageTemp->Deactivate ();


	geometryImageTemp->Activate ();
		glDrawBuffer (GL_FRONT);
		glActiveTexture(GL_TEXTURE0);
		geometryImageTemp->Bind (WGL_FRONT_LEFT_ARB); // vertex coords

		glActiveTexture(GL_TEXTURE1);
		gaussianAccum->Bind (WGL_BACK_LEFT_ARB);

		glActiveTexture(GL_TEXTURE2);
		geometryImageRTT->Bind (WGL_BACK_LEFT_ARB); //normal coords

		glActiveTexture(GL_TEXTURE3);
		glBindTexture (GL_TEXTURE_RECTANGLE_NV, jawId);

		glUseProgramObjectARB(moveFeaturePoints);
			glUniform3fARB	(moveXYZ,0,0,0);
			glUniform1fARB	(normal, norm);
			glUniform1fARB	(tangent,tan);
			glUniform1fARB	(degrees, deg);	
			DrawQuad2 (m_w, m_h, m_w, m_h);
		glUseProgramObjectARB(0);
		
		geometryImageTemp->Release (WGL_FRONT_LEFT_ARB);
		geometryImageRTT->Release (WGL_BACK_LEFT_ARB);
		gaussianAccum->Release (WGL_BACK_LEFT_ARB);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture (GL_TEXTURE_RECTANGLE_NV, 0);

	geometryImageTemp->Deactivate ();

	setFlag (ID_REFRESH_VERTICES);

}
/*-------------------------------------------------------------------------------------------------*/
void cGIface::filterControlPointsMask (PBuffer *canvas)
{
}
/*-------------------------------------------------------------------------------------------------*/
void cGIface::makeFacialHoles (int flag)
{
	static unsigned int faceHolesId=0; 

	if (flag==ID_FROM_FILE) {
		BmpImage *faceHolesImage = new BmpImage ();
		faceHolesImage->readBmpFile("faceHoles.bmp");
		if (!faceHolesId)
			glGenTextures (1, &faceHolesId);
		glBindTexture (GL_TEXTURE_RECTANGLE_NV, faceHolesId);
		glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D  (GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB,faceHolesImage->width,faceHolesImage->height, 0, GL_RGB, GL_UNSIGNED_BYTE, faceHolesImage->data);
	
		delete faceHolesImage;
		faceHolesImage = NULL;
	}
 
	geometryImageRTT->Activate ();
		glDrawBuffer (GL_FRONT);
		glActiveTexture(GL_TEXTURE0);
		geometryImageRTT->Bind (WGL_FRONT_LEFT_ARB); // vertex coords

		glActiveTexture(GL_TEXTURE1);
		flag==ID_FROM_FILE? 
			glBindTexture (GL_TEXTURE_RECTANGLE_NV, faceHolesId)
			:
			gaussianAccum->Bind (WGL_BACK_LEFT_ARB);

		glUseProgramObjectARB(makeFaceHoles);
			DrawQuad(m_w, m_h, m_w, m_h);
		glUseProgramObjectARB(0);

		geometryImageRTT->Release (WGL_FRONT_LEFT_ARB);

		flag==ID_FROM_FILE ?
			glActiveTexture (GL_TEXTURE1),
			glBindTexture (GL_TEXTURE_RECTANGLE_NV, 0)
			:
			gaussianAccum->Release (WGL_FRONT_LEFT_ARB);

	geometryImageRTT->Deactivate ();

	geometryImageTemp->Activate ();
		glDrawBuffer (GL_FRONT);
		DisplayTexture(geometryImageRTT, WGL_FRONT_LEFT_ARB, m_w, m_h);
	geometryImageTemp->Deactivate ();

	if (flag==ID_FROM_FILE) {
		glDeleteTextures (1, &faceHolesId);
		faceHolesId = 0;
	}
	
	setFlag (ID_REFRESH_VERTICES);
}
/*-------------------------------------------------------------------------------------------------*/
void cGIface::openJaw (float deg)
{
	unsigned int jawId = 0;

	BmpImage *jawImage = new BmpImage ();
	jawImage->readBmpFile ("jaw.bmp");

	glGenTextures (1, &jawId);
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, jawId);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
	glTexImage2D  (GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB,jawImage->width,jawImage->height, 0, GL_RGB, GL_UNSIGNED_BYTE, jawImage->data);
	
	delete jawImage;
	jawImage = NULL;

	geometryImageTemp->Activate ();
		glDrawBuffer (GL_FRONT);

		glActiveTexture(GL_TEXTURE0);
		geometryImageRTT->Bind (WGL_FRONT_LEFT_ARB); // vertex coords
/*
		glActiveTexture(GL_TEXTURE1);
		geometryImageRTT->Bind (WGL_FRONT_LEFT_ARB); // vertex coords
*/
		glActiveTexture(GL_TEXTURE1);
		glBindTexture (GL_TEXTURE_RECTANGLE_NV, jawId);

		glUseProgramObjectARB(openJawShader);
			glUniform1fARB	(degreesJaw, deg);	
			DrawQuad(m_w, m_h, m_w, m_h);
		glUseProgramObjectARB(0);

		geometryImageRTT->Release (WGL_FRONT_LEFT_ARB);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture (GL_TEXTURE_RECTANGLE_NV, 0);
	geometryImageTemp->Deactivate ();

	glDeleteTextures (1, &jawId);
	jawId = 0;

}
/*-------------------------------------------------------------------------------------------------*/
void cGIface::calcNormals ()
{
	geometryImageRTT->Activate ();
		glDrawBuffer (GL_BACK);
		
		glActiveTexture (GL_TEXTURE0);
		geometryImageTemp->Bind (WGL_FRONT_LEFT_ARB);
		
		glUseProgramObjectARB(calcNormal);
			DrawQuad (m_w, m_h, m_w, m_h);
		glUseProgramObjectARB(0);

		geometryImageTemp->Release ();
	geometryImageRTT->Deactivate ();
}
/*-------------------------------------------------------------------------------------------------*/
void cGIface::setFlag (int whatFlag)
{
	switch (whatFlag) {
		case ID_REFRESH_NORMALS:
			calcNormals ();
			normalFlag=1;
			break;
		case ID_REFRESH_VERTICES:
			vertexFlag = 1;
	}
}
/*-------------------------------------------------------------------------------------------------*/
void cGIface::setExpression (PBuffer *MIZ, float *muscleParam)
{
	if (expressionCont < MAX_EXPRESSIONS) {
								
		MIZs[expressionCont] = new RenderTexture ("float=16 rgb texture single", m_w, m_h, GL_TEXTURE_RECTANGLE_NV); 

		MIZs[expressionCont]->Activate ();
//			glDrawBuffer (GL_FRONT);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0.0, m_w, 0.0, m_h, -1.0, 1.0);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glClearColor (0.0,0,0,1);
			glClear (GL_COLOR_BUFFER_BIT);
		MIZs[expressionCont]->Deactivate();

		MIZs[expressionCont]->Activate ();
//			glDrawBuffer (GL_FRONT);
			DisplayTexture(gaussianAccum, WGL_BACK_LEFT_ARB, m_w, m_h);
		MIZs[expressionCont]->Deactivate ();

		if (!muscleParams)
			muscleParams = new float [MAX_EXPRESSIONS][3];
		muscleParams[expressionCont][0] = muscleParam[0];
		muscleParams[expressionCont][1] = muscleParam[1];
		muscleParams[expressionCont][2] = muscleParam[2];
		expressionCont++;
		}
}
/*-------------------------------------------------------------------------------------------------*/
void cGIface::setTime (unsigned int time_)
{
	static int cont=0;
//	time = new unsigned int [MAX_EXPRESSIONS];

	time[cont++] = time_;

	if (cont==MAX_EXPRESSIONS)
		cont=0;
}
/*-------------------------------------------------------------------------------------------------*/
/*
GLhandleARB animateShader;
int muscleParamsAnimate1;
int muscleParamsAnimate2;
int deltaTimeAnimate;
*/
//extern int animateButton;
void cGIface::setDeltaTime (float delta) {
	static float timeCont= 0.0;

	if (timeCont>=time[1]){
//		animateButton=0;
		timeCont=0.0;
	}
		timeCont+=delta;

}
/*-------------------------------------------------------------------------------------------------*/
extern float fps;
void cGIface::animate (float deltaTime, float fps)
{
	
	static float MPs1[3];
	static float MPs2[3];
	static char flagMax=0;
	float totFrames = time[1]*fps;
	static float frame=0.0;
/*
	if (animateButton==0) {
	flagMax=0;
	frame=0.0;
	}
*/
	MPs1[0]= muscleParams[0][0];
	MPs1[1]= muscleParams[0][1];
	MPs1[2]= muscleParams[0][2];

	MPs2[0]= muscleParams[1][0];
	MPs2[1]= muscleParams[1][1];
	MPs2[2]= muscleParams[1][2];

	frame+=1/totFrames;

		geometryImageTemp->Activate ();
			glActiveTexture (GL_TEXTURE0);
			geometryImageRTT->Bind (WGL_FRONT_LEFT_ARB);
			glActiveTexture (GL_TEXTURE1);
			MIZs[0]->Bind (WGL_FRONT_LEFT_ARB);
			glActiveTexture (GL_TEXTURE2);
			geometryImageRTT->Bind (WGL_BACK_LEFT_ARB);
			
			glUseProgramObjectARB(animateShader);
				glUniform3fvARB	(muscleParamsAnimate1,1, MPs1);
				glUniform3fvARB	(muscleParamsAnimate2,1, MPs2);
				glUniform1fARB  (deltaTimeAnimate, frame);
				DrawQuad (m_w,m_h,m_w,m_h);
			glUseProgramObjectARB(0);

			geometryImageRTT->Release (WGL_FRONT_LEFT_ARB);
			MIZs[0]->Release (WGL_FRONT_LEFT_ARB);
			geometryImageRTT->Release(WGL_BACK_LEFT_ARB);

		geometryImageTemp->Deactivate ();

	setFlag (ID_REFRESH_VERTICES);
}