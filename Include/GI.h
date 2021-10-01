
#ifndef __GI_H__
#define __GI_H__

#include	"frameBufferObject.h"
#include	"glErrorUtil.h"
#include	"renderVertexArray.h"
#include	"ModelObject.h"
#include	"ShaderObject.h"
#include	"openEXR.h"


#define		GI_WIDTH	512
#define		GI_HEIGHT	512

#define FREE_MEMORY(ptr)	\
    if (ptr) {				\
		delete ptr;			\
        ptr=NULL;			\
    }

#define DELETE_SHADER(ptr)			\
	if (ptr) {						\
		glDeleteProgram (ptr);	\
		ptr = 0;					\
	}


class cGI {
public:
	cGI (int width, int height);
	~cGI ();
	int		generateGI (char *filename);
	void	generateGIfromBuffer (Array2D<rgb> &pixels);
	void	draw		(int primitive);
	void	DrawQuad (int w, int h, int tw, int th);
	virtual void	DisplayTextures();
	void	updateLPos (int c);
	void	delete3DModel ();
	//void	applyBlur ();
	int		getWidth () { return m_w; }
	int		getHeight () { return m_h; }

	void	getGI (Array2D<rgb> &pixels);
	void	displayOGL ();

protected:

	// OpenGL 2.0 defs
	ShaderObject *displayFloatTex, *displayGI, *genGI;
	ShaderObject *calcNormals;
	
	RenderVertexArray	*geometryImageRTVVertex;
	RenderVertexArray	*geometryImageRTVNormal;
	
	int		m_w, m_h, scaleFactor;
	int		numIndices, numIndicesPoints;
	unsigned int *indices,*indicesPoints;

	void DisplayTexture(GLuint texId, int w, int h);

	int vp[4];

	ModelObject *model;
	char *fileName;

	FramebufferObject	*geometryImageRTT; //stores vertices [0] & normals [1]
	unsigned int		geometryImageTexId[2]; 

	unsigned int 	SET_PROJECTION0_0,	SET_PROJECTION0_1,	SET_PROJECTION1_0,
					SET_PROJECTION1_1,	ENABLE_VBO_STUFF,	DISABLE_VBO_STUFF;


private:
	
	int mainFrameBuffer;
	float lPos[4];
};

#endif __GI_H__