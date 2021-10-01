
#include	<stdio.h>
#include	<math.h>
#include	<GL/glut.h>
#include	"../Include/Camera.h"

cStereoCamera::cStereoCamera (double camAperture, double focalL)
{
	aperture = camAperture;
	focalLen = focalL;

	eyeSeparation = 0.005;

	pointRotate[0] = pointRotate[1] = pointRotate[2] = 0;
}

void cStereoCamera::setWidthHeight (int w, int h)
{
	width  = w;
	height = h;
	ratio = width / (double)height;
}

void cStereoCamera::normalize (float *p)
{
	double len;

	len = sqrt (p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);
	len > 0 ?
		p[0] /= len,
		p[1] /= len,
		p[2] /= len
	:
		p[0] = 0,
		p[1] = 0,
		p[2] = 0;
}

void cStereoCamera::setAperture (double aperture_)
{
	aperture = aperture_;
}

void cStereoCamera::setEyeSeparation (double separation)
{
	eyeSeparation = separation;
}

void cStereoCamera::setCamera (int whatCamera, int whatStereoVision)
{
	
	float r[3];
	double radians,wd2,ndfl;
	double left,right,top,bottom,nearx,farx=100;

	nearx = focalLen/10;

	radians = DTOR * aperture / 2;
	wd2     = nearx * tan(radians);
	ndfl    = nearx / focalLen;

	top		= wd2;
	bottom	= -wd2;	


	CROSSPROD(viewDirection,viewUpDir,r);
	normalize (r);
	r[0] *= eyeSeparation / 2.0;
	r[1] *= eyeSeparation / 2.0;
	r[2] *= eyeSeparation / 2.0;


	if (whatStereoVision==MONO) {
			gluLookAt(viewPosition[0]    , viewPosition[1]    , viewPosition[2],	
			  viewDirection[0], viewDirection[1], viewDirection[2],	
			  viewUpDir[0]  , viewUpDir[1]  , viewUpDir[2]);
	}
	else {
		switch (whatCamera) {
		case LEFTCAM:
		    left  = - ratio * wd2 + 0.5 * eyeSeparation * ndfl;
			right =   ratio * wd2 + 0.5 * eyeSeparation * ndfl;

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glFrustum(left,right,bottom,top,nearx,farx);

			glMatrixMode (GL_MODELVIEW);
			glLoadIdentity();
			if (whatStereoVision == ACTIVESTEREO){
				glDrawBuffer(GL_BACK_RIGHT);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}
				
			else if (whatStereoVision == PASSIVESTEREO){
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glViewport (0,0, width/2, height);
			}

			gluLookAt(viewPosition[0] - r[0], viewPosition[1] - r[1],viewPosition[2] - r[2],
						 - r[0] + viewDirection[0],
						 - r[1] + viewDirection[1],
						 - r[2] + viewDirection[2],
						viewUpDir[0], viewUpDir[1], viewUpDir[2]);
			break;
		case RIGHTCAM:
			left  =  -ratio * wd2 - 0.5 * eyeSeparation * ndfl;
			right =   ratio * wd2 - 0.5 * eyeSeparation * ndfl;

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glFrustum(left,right,bottom,top,nearx,farx);

			glMatrixMode (GL_MODELVIEW);
			glLoadIdentity();

			if (whatStereoVision == ACTIVESTEREO){
				glDrawBuffer(GL_BACK_LEFT);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}
				
			else if (whatStereoVision == PASSIVESTEREO){
				glViewport (width/2, 0, width/2, height);
			}

			gluLookAt(viewPosition[0] + r[0], viewPosition[1] + r[1],viewPosition[2] + r[2],
						 + r[0] + viewDirection[0],
						 + r[1] + viewDirection[1],
						 + r[2] + viewDirection[2],
						viewUpDir[0], viewUpDir[1], viewUpDir[2]);
			
			break;

		}
	}
}

void cStereoCamera::setPosition(float *pos, float *view, float *upDir)
{
	viewPosition[0] = pos[0];
	viewPosition[1] = pos[1];
	viewPosition[2] = pos[2];

	viewDirection[0] = view[0];
	viewDirection[1] = view[1];
	viewDirection[2] = view[2];

	viewUpDir[0] = upDir[0];
	viewUpDir[1] = upDir[1];
	viewUpDir[2] = upDir[2];
}


void cStereoCamera::moveCamera (float speed, float height, float heightView)
{
	//float aux=speed/speed;
/*
	if (viewPosition[0]>5) {
		viewPosition[0]=5;
		return;
	}
	else if (viewPosition[0]<-5){
		viewPosition[0]=-5;
		return;
	}
	else if (viewPosition[2]>5){
		viewPosition[2]=5;
		return;
	}
	else if (viewPosition[2]<-5){
		viewPosition[2]=-5;
		return;
	}
	else{
*/		vec[0] = viewDirection[0] - viewPosition[0];
		vec[1] = viewDirection[2] - viewPosition[2];

		viewPosition[0] = viewPosition[0] + vec[0]*speed;
		viewPosition[1] = (height+0.01); 
		viewPosition[2] = viewPosition[2] + vec[1]*speed;

		viewDirection[0] = viewDirection[0] + vec[0]*speed;
		viewDirection[1] = (heightView+0.01);
		viewDirection[2] = viewDirection[2] + vec[1]*speed;
//	}
}



void cStereoCamera::rotateView (float x, float y, float z)
{
	
	if (x) {
		vec[0] = viewDirection[1] - viewPosition[1];
		vec[1] = viewDirection[2] - viewPosition[2];
		viewDirection[2] = viewPosition[2] + sin(x)*vec[0] + cos(x)*vec[1];
		viewDirection[1] = viewPosition[1] + cos(x)*vec[0] - sin(x)*vec[1];
	}
	if (y) {
		vec[0] = viewDirection[0] - viewPosition[0];
		vec[1] = viewDirection[2] - viewPosition[2];
		viewDirection[2] = viewPosition[2] + sin(y)*vec[0] + cos(y)*vec[1];
		viewDirection[0] = viewPosition[0] + cos(y)*vec[0] - sin(y)*vec[1];
	}
	if (z) {
		vec[0] = viewDirection[0] - viewPosition[0];
		vec[1] = viewDirection[1] - viewPosition[1];
		viewDirection[0] = viewPosition[0] + sin(z)*vec[1] + cos(z)*vec[0];
		viewDirection[1] = viewPosition[1] + cos(z)*vec[1] - sin(z)*vec[0];
	}
}


void cStereoCamera::getPosAndDir (float *pos, float *dir)
{
	*pos = *viewPosition;
	*(pos+1) = *(viewPosition+1);
	*(pos+2) = *(viewPosition+2);

	*dir = *viewDirection;
	*(dir+1) = *(viewDirection+1);
	*(dir+2) = *(viewDirection+2);

}


void cStereoCamera::printPosAndDir (void)
{
	vec[0] = viewDirection[0] - viewPosition[0];
	vec[1] = viewDirection[2] - viewPosition[2];

	float mag = sqrt (vec[0]*vec[0] + vec[1]*vec[1]);

	printf ("Position: %f,%f,%f | PosX-DirX=%f | PosZ-DirZ=%f \n", viewPosition[0], viewPosition[1], viewPosition[2], abs (viewDirection[0]-viewPosition[0]), abs(viewDirection[2]-viewPosition[2]));
	printf ("Direction: %f,%f,%f\n", viewDirection[0], viewDirection[1], viewDirection[2]);
	printf ("Vec: %f %f mag: %f \n",abs(vec[0]), abs(vec[1]), mag); 

}
