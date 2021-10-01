
#pragma warning ( disable : 4244 )

#include <math.h>
#include "../Include/3d_types.h"

GLfloat dtor(GLfloat grados) {
  return(grados*(3.1415926536/180.0));
}

GLfloat rtod(GLfloat radianes) {
  return((radianes/3.1415926536)*180.0);
}

struct GLpoint normalize(struct GLpoint a) {
	GLfloat mag;
	mag=sqrt(sqlen(a));
	
	if (mag==0)
		return a;
	
	a.x/=mag;	a.y/=mag;	a.z/=mag;
  return(a);
}

struct GLpoint scale(struct GLpoint a, float s) {
	
  a.x*=s;a.y*=s;a.z*=s;
  return a;
}

struct GLpoint proj(struct GLpoint a, struct GLpoint b) {
	GLfloat pr;
	pr=dotprod(a,b)/sqlen(a);
	a.x*=pr;	a.y*=pr;	a.z*=pr;
  return(a);
}

struct GLpoint add(struct GLpoint a, struct GLpoint b) {
	
	a.x+=b.x;	a.y+=b.y;	a.z+=b.z;
  return(a);
}

struct GLpoint sub(struct GLpoint a, struct GLpoint b) {
	
	a.x-=b.x;	a.y-=b.y;	a.z-=b.z;
  return(a);
}

struct GLpoint crossprod(struct GLpoint a, struct GLpoint b) {
	GLpoint r;
	r.x=a.y*b.z-b.y*a.z;
	r.y=a.x*b.z-b.x*a.z;
	r.z=a.x*b.y-b.x*a.y;
  return(r);
}

struct GLpoint mult(float s, struct GLpoint u)
{
	u.x*s, u.y*s, u.z*s;
	return u;
}

struct GLpoint mult(struct GLpoint u, float s)
{
	u.x*s, u.y*s, u.z*s;
	return u;
}

struct GLpoint div(struct GLpoint u, float s)
{
	u.x/s, u.y/s, u.z/s;
	return u;
}

// triple scalar product (u dot (v cross w))
float TripleScalarProduct(struct GLpoint u, struct GLpoint v, struct GLpoint w)
{
	return float(	(u.x * (v.y*w.z - v.z*w.y)) +
					(u.y * (-v.x*w.z + v.z*w.x)) +
					(u.z * (v.x*w.y - v.y*w.x)) );
	//return u*(v^w);

}

struct GLpoint operator *(struct GLpoint u, struct GLpoint v)
{
	u.x*v.x, u.y*v.y, u.z*v.z;
	return u;
}

struct GLpoint operator *(struct GLpoint u, float v)
{
	u.x*v, u.y*v, u.z*v;
	return u;
}

struct GLpoint operator /(struct GLpoint u, struct GLpoint v) {
	u.x/v.x, u.y/v.y, u.z/v.z;
	return u;
}

struct GLpoint operator ==(struct GLpoint u, struct GLpoint v) {
	u.x=v.x, u.y = v.y, u.z=v.z;
	return u;
}
