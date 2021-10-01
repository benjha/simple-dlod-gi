#ifndef		MODEL_OBJECT
#define		MODEL_OBJECT

#include	"3d_types.h"
#include	"BmpImage.h"

#define		DISPLAY_NORMAL		0
#define		DISPLAY_TEXTURED	1
#define		DISPLAY_GI			2
#define		DISPLAY_GI_X		3
#define		DISPLAY_GI_Y		4
#define		DISPLAY_GI_Z		5
#define		DISPLAY_NORMALS		6
#define		DISPLAY_NORMALS_X	7
#define		DISPLAY_NORMALS_Y	8
#define		DISPLAY_NORMALS_Z	9

enum {
	FILE_NOT_FOUND,
	MATERIAL_PROBLEMS,
	TEXTURE_MAP_PROBLEM,
	READING_PROBLEM,
	OK
};

class ModelObject {

public:
	GLint (*faces)[9];
	GLpoint *vertices;
	GLpoint *normals;
	GLpoint *textures;
	GLmaterial *materials;
	GLtexture *texture_maps;
  GLuint texture_name;
	GLint num_tex_maps;
	GLint (*material_ref)[2];
	GLint num_faces,num_points,num_materials,num_refs,num_tex,num_norm;
	GLboolean show_tex;
	GLshort *norm_temp;

	void MyMaterial(GLenum mode, GLfloat *f, GLfloat alpha);
	void SelectMaterial(GLint i);
	BOOL LoadDIB(char*file,DIB2D*dib);
	long ScanBytes(int pixWidth, int bitsPixel);
	BOOL ScaleImage(DIB2D&dib,GLTXTLOAD&p);
	void LoadTexture();


	ModelObject();
	~ModelObject();
	int LoadOGL(char *in);
	void Load3DS(char *in);
	void LoadOBJ(char *in);
	void ShowTextures(GLboolean show);
	void Normalize();
	void Display(int);
};



#endif
