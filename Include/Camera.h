
#ifndef		__STEREO_OPTIONS_H__
#define		__STEREO_OPTIONS_H__


#define		MONO			1
#define		PASSIVESTEREO	2
#define		ACTIVESTEREO	3

#define		RIGHTCAM		0
#define		LEFTCAM			!RIGHTCAM

#ifndef		PI
	#define		PI				3.141592653589793238462643f
#endif
#ifndef		DTOR
	#define		DTOR			0.0174532925f
#endif
#ifndef		RTOD
	#define		RTOD			57.2957795f
#endif

#define		STEREO_OPC		1
#define		ACTIVESTEREOr	2
#define		ACTIVESTEREOl	3
#define		PASSIVESTEREOr	4
#define		PASSIVESTEREOl	5

#define CROSSPROD(p1,p2,p3) \
   p3[0] = p1[1]*p2[2] - p1[2]*p2[1]; \
   p3[1] = p1[2]*p2[0] - p1[0]*p2[2]; \
   p3[2] = p1[0]*p2[1] - p1[1]*p2[0]


class cStereoCamera {
public:
	cStereoCamera (double camAperture, double focalL);

	void	setPosition(float *pos, float *view, float *upDir);
	void	setWidthHeight (int w, int h);
	void	normalize (float *p);
	void	setAperture (double aperture_);
	void	setEyeSeparation (double separation);
	
	void	setCamera (int whatCamera, int whatStereoVision);
	void	moveCamera (float speed, float height, float heightView);
	void	rotateView (float x, float y, float z);

	void	getPosAndDir (float *pos, float *dir);
	void	printPosAndDir (void);

private:

	float	viewPosition[3];
	float	viewDirection[3];
	float	viewUpDir[3];
	float	pointRotate[3];
	double	focalLen;
	double	aperture;
	double	eyeSeparation, ratio;
	int		width, height;
	float   vec[3];	

};



#endif