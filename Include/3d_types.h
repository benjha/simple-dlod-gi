#ifdef WIN32
	#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

#if !defined CLOTH_TYPES

#define CLOTH_TYPES


struct GLpoint
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
};		

#define SPRINGS_MAX 8
struct GLsprings {
	int num_springs;
	int spring[SPRINGS_MAX];
	GLfloat rest[SPRINGS_MAX];
	GLfloat current[SPRINGS_MAX];
};

struct GLmaterial {
 GLfloat ambient[3];
 GLfloat diffuse[3];
 GLfloat specular[3];
 GLfloat emission[3];
 GLfloat alpha;
 GLfloat phExp;
 int   texture;
};

struct GLtexture {
 char name[256];
 GLint  id;
};

struct DIB2D{
 BITMAPINFOHEADER *Info;
 RGBQUAD *palette;
 BYTE    *bits;
};

struct GLTXTLOAD{
 GLint format;
 GLint perpixel;
 GLint Width;
 GLint Height;
 BYTE* bits;
};


#define	square(x)		((x)*(x))
#define sqlen(p)		(square(p.x)+square(p.y)+square(p.z))
#define sqdist(p,q)		(square(p.x-q.x)+square(p.y-q.y)+square(p.z-q.z))
#define dotprod(a,b)	((a).x*(b).x + (a).y*(b).y + (a).z*(b).z)
#define igualaEsc(a,b)	(a.x=a.y=a.z=b);
#define igualaVec(a,b)	(a.x=b.x, a.y=b.y, a.z=b.z);
#define multEsc(a,b)	(a.x*b, a.y*b, a.z*b);
#define multVec(a,b)	(a.x*b.x, a.y*b.y, a.z*b.z);






GLfloat dtor(GLfloat grados);
GLfloat rtod(GLfloat radianes);
GLpoint normalize(struct GLpoint a);
GLpoint scale (struct GLpoint,float);
GLpoint proj(struct GLpoint a, struct GLpoint b);
GLpoint add(struct GLpoint a, struct GLpoint b);
GLpoint sub(struct GLpoint a, struct GLpoint b);
GLpoint crossprod(struct GLpoint a, struct GLpoint b);

float mult(struct GLpoint  u, struct GLpoint  v);
GLpoint mult(float s, struct GLpoint  u);
GLpoint mult(struct GLpoint  u, float s);
GLpoint div(struct GLpoint  u, float s);
float TripleScalarProduct(struct GLpoint  u, struct GLpoint  v, struct GLpoint  w);

struct GLpoint operator ==(struct GLpoint , struct GLpoint );
struct GLpoint operator +(struct GLpoint , struct GLpoint );
struct GLpoint operator -(struct GLpoint , struct GLpoint );
struct GLpoint operator /(struct GLpoint , struct GLpoint );
struct GLpoint operator *(struct GLpoint , struct GLpoint );
struct GLpoint operator *(struct GLpoint , float );




#endif
