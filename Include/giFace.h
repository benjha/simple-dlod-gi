

#ifndef		__GI_FACE_H__
#define		__GI_FACE_H__

#include	"pbuffer.h"	
#include	"ModelObject.h"
#include	"BmpImage.h"
#include	"RenderTexture.h"
#include	"renderVertexArray.h"

#define		GI_WIDTH	512
#define		GI_HEIGHT	512

#define		FACTOR	32768.0f		// TIFF factor
//#define		FACTOR	255.0f

#define FACIALPOINTS		32
#define FACIALPOINTSFAP		!FACIALPOINTSBLACK
#define FAPPOINTS			18
#define FAP					6

#define MAX_EXPRESSIONS		2

enum {
	ID_4L,	ID_4R,
	ID_3L,	ID_3R,
	ID_9,
	ID_5L,	ID_5R,
	ID_8U,
	ID_8D
};

enum {
	ID_REFRESH_NORMALS,
	ID_REFRESH_VERTICES
};
//(y*width+x)*3
/*
typedef struct _facialPoint_
{
	float	pos[3];
	float	id;
	int		index;
} sFacialPoint;

typedef struct _fap_
{
	char id[6];
	float magnitude;
} sFAP;

typedef struct faceCoords_
{
	//float (*textures)[3];
	float normals[3];
	float vertex [4];
} sFaceCoords;
*/
class cGIface {

public:
	cGIface ();
	cGIface (int width, int height);
	~cGIface ();
		
	void	generateGIface (ModelObject *model);
	
	void	makeFacialHoles (int flag);
	void	openJaw			(float deg);

	void	moveFeaturePointsSet (int whatSet, PBuffer *canvas, float *moveXYZ);
	void	moveFeaturePointsSet (int whatSet, PBuffer *canvas, float normal, float tan, float deg);
	
	void draw		(int primitive);

	void DrawQuad (int w, int h, int tw, int th);
	void DrawQuad2 (int w, int h, int tw, int th);

	void DisplayTextures();
	void addGIAndCanvas (PBuffer *canvas);
	void filterControlPointsMask (PBuffer *canvas);
	void makeGaussianfilter (int size, float sigma, PBuffer *canvas);
	
	void setRenderMode (int renderMode_);

	void calcNormals ();
	void setFlag (int whatFlag);

	void setCanvas (PBuffer *canvas);
	void setExpression (PBuffer *MIZ, float *muscleParams);
	void setDeltaTime  (float delta);
	void setTime (unsigned int time_);
	void animate (float deltaTime,float frames);

	GLhandleARB	initShaders (char *fileNameVertex, char *fileNameFragment);
	char		*loadShaderCode (char *fileName);

	GLhandleARB passthruTex;

	RenderTexture		*geometryImageRTT;
	RenderTexture		*geometryImageTemp;

private:
	
	unsigned int	*indices;
	int				numIndices;
	float			*kernel;
	unsigned int	kernelTextureHId;
	unsigned int	offsetTexture, faceTexId, normalMapId, jawId;
	int				sizeFilter;

	int				normalFlag; // para decir que re-calcule normales
	int				vertexFlag;

	float			deltaTime;

	// Pixels y Vertex Buffers para Geometria Original
	RenderVertexArray	*geometryImageRTVVertex;
	RenderVertexArray	*geometryImageRTVNormal;
	RenderVertexArray	*geometryImageRTVTextures0;
	RenderVertexArray	*geometryImageRTVTextures1;

	// Pixels Buffer para la Geometria bajo los efectos de Canvas
	
	RenderTexture		*gaussianAccum;
	
	void DisplayTexture(PBuffer			*pbuffer, GLenum buffer, int w, int h);
	void DisplayTexture(RenderTexture	*pbuffer, GLenum buffer, int w, int h);
	
	int m_w, m_h;

	RenderTexture *MIZs[2];
	float (*muscleParams)[3];
	unsigned int time[2];
	int expressionCont;

	// Cheiders Used
	int kernelH, gaussFilterFlag;
	GLhandleARB passthruShader;
	GLhandleARB renderTextured;
	int renderMode;

	GLhandleARB genGIAndSkull;
	int			scaleFactor;

	GLhandleARB addCanvasAndGI;
	GLhandleARB vertexHandler;
	GLhandleARB fragmentHandler;
	GLhandleARB Handler;

	GLhandleARB gaussFiltering3x3;
	GLhandleARB gaussFiltering5x5;
	GLhandleARB gaussFiltering7x7;
	GLhandleARB gaussFiltering9x9;
	GLhandleARB gaussFiltering11x11;
	GLhandleARB gaussFiltering21x21;

	GLhandleARB moveFeaturePoints;
	int			moveXYZ;
	int			normal;
	int			tangent;
	int			degrees;

	GLhandleARB makeFaceHoles;
	

	GLhandleARB openJawShader;
	int degreesJaw;

	GLhandleARB calcNormal;

	GLhandleARB animateShader;
	int muscleParamsAnimate1;
	int muscleParamsAnimate2;
	int deltaTimeAnimate;

};

//void initOGLSL (void);

#endif