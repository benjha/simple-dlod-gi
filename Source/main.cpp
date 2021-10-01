
#include <GL/glew.h>
#include <GL/glut.h>
#include <time.h>
#include <stdlib.h>
#include "../Include/Camera.h"
#include "../Include/GI.h"
#include "../Include/openEXR.h"
#include "../Include/giLod.h"

#define FPV  // First Person View

#define LOD
#ifdef LOD
	cGILod	*giModel;
	cGILod	*giModel2;
	cGILod	*giModel3;
#else
	cGI	*giModel;
#endif

//FrameSaver saver("bmp");
int saverFlag=0;
cStereoCamera	cam(2, 0.1);
int keyDirection[4]={0,0,0,0};

float sightMag	= 15.0;
float sightMag2 = 20.0;
float sightMag3 = 30.0;
float pos[]  = {0,0,15.0};
float dir[]  = {0,0,0};
float up[]	 = {0,1.0,0};

float z=2, x_=0, y_=0;
float rotX=0.0, rotY=0.0;
int lastX=0, lastY=0;
int boton;

clock_t t_act,t_ini;
long frame_count=0;
double factor=3900;
clock_t t_nvo,t_dif;
GLfloat elapsed;

void display(void)
{
	register x,y;
	t_nvo=glutGet(GLUT_ELAPSED_TIME);
	t_dif=t_nvo-t_act;
	t_act=t_nvo;
	elapsed=t_dif/factor;
	frame_count++;
	glClearColor (0.90, 0.90, 0.90, 1.0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef FPV
   cam.setCamera(LEFTCAM, MONO);
#endif
   
   glBegin (GL_QUADS);
		glColor3f (0,0,1);
		glVertex3f(-20, -0.5, 20);
		glColor3f (1,0,1);
		glVertex3f( 20, -0.5, 20);
		glColor3f (1,0,0);
		glVertex3f( 20, -0.5, -20);
		glColor3f (0,0.4,0);
		glVertex3f(-20, -0.5, -20);
   glEnd ();
/*   
   glPushMatrix ();
   glScalef (0.5,0.5,0.5);
   for (x=-16;x<16;x++)
	   for (y=-8;y<8;y++){
	   glPushMatrix ();
	   glTranslatef(x*0.5,y,0);
*/
   giModel->draw (GL_TRIANGLE_STRIP);
   giModel->DisplayTextures ();
/*
   glPushMatrix ();
   glTranslatef (1,0,-5);
   giModel2->draw (GL_TRIANGLE_STRIP);
   glPopMatrix ();
/*   
   glPushMatrix ();
   glTranslatef (-1,0,-10);
   giModel3->draw (GL_TRIANGLE_STRIP);
   glPopMatrix ();
*/

 /*	   glPopMatrix ();
	   }
   glPopMatrix ();
   
  */ 
   	if(frame_count%30==0) 
		if (t_dif!=0) {
			printf("%dfps/%dms %dprom/%dms, %f  \n",
				1000/(t_dif),t_dif,
				1000/((t_nvo-t_ini)/(frame_count+1)),
				(t_nvo-t_ini)/frame_count, elapsed );
		}

/*	if (saverFlag)
		saver.writeNextFrame ();
*/
	glutSwapBuffers ();
}

void idle (void)
{
	glLoadIdentity ();

#ifdef FPV
	if (keyDirection[1])
		cam.rotateView (0,-0.01, 0);
	if (keyDirection[2])
		cam.rotateView (0,0.01, 0);
	if (keyDirection[0])
		cam.moveCamera (0.0015, 0, 1);
	if (keyDirection[3])
		cam.moveCamera (-0.0015, 0, 1);
	cam.getPosAndDir (pos, dir);
#ifdef LOD
	giModel->upDateCameraPosAndDir (pos,dir);
	giModel->calcDistMipMap ();

//	giModel2->upDateCameraPosAndDir (pos,dir);
//	giModel2->calcDistMipMap ();
/*
	giModel3->upDateCameraPosAndDir (pos,dir);
	giModel3->calcDistMipMap ();
*/
#endif
#else
	glTranslatef (0,0, -z);
	glRotatef (lastY, 1, 0, 0);
	glRotatef (lastX, 0, 1, 0);
#endif
	glutPostRedisplay ();
}

void init (void) 
{
	GLenum err = glewInit();
	if (GLEW_OK != err) 
		return;

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
  
    glEnable(GL_DITHER);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

#ifdef LOD
   giModel = new cGILod (GI_WIDTH, GI_HEIGHT, sightMag);
   giModel->generateGI ("vaseTextured.cpp");
   giModel->generateMipMap ();
   giModel->delete3DModel ();
   giModel->upDateCameraPosAndDir (pos,dir);
   giModel->updateLPos ('z');
/*
   giModel2 = new cGILod (256, 256, sightMag2);
   giModel2->generateGI ("vaseTextured.cpp");
   giModel2->generateMipMap ();
   giModel2->delete3DModel ();
   giModel2->upDateCameraPosAndDir (pos,dir);
   giModel2->updateLPos (lpos);
/*
   giModel3 = new cGILod (GI_WIDTH/4.0, GI_HEIGHT/4.0, sightMag3);
   giModel3->generateGI ("vaseTextured.cpp");
   giModel3->generateMipMap ();
   giModel3->delete3DModel ();
   giModel3->upDateCameraPosAndDir (pos,dir);
*/
#else
   giModel = new cGI (GI_WIDTH, GI_HEIGHT);
   giModel->generateGI ("vaseTextured.cpp");
#endif

   t_act=glutGet(GLUT_ELAPSED_TIME);
   t_ini=t_act;

   cam.setPosition (pos, dir, up);

   //glPointSize (2);
   //glEnable (GL_POINT_SMOOTH);
}

void reshape (int w, int h)
{
	glViewport (0,0, w,h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	
	gluPerspective (45, w/h*1.0, 0.01, 5);
	glTranslatef (0,0,-1);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	//cam.moveCamera (0.0015, 0, 1);
}

void mouse (int button, int state, int x, int y)
{
	boton = button;
}

void motion (int x, int y)
{
#ifdef FPV
	if (boton == GLUT_LEFT_BUTTON){
		rotX = x*0.01*360.0;
		rotY = y*0.01*360.0;
	}

	if (boton == GLUT_RIGHT_BUTTON) 
		lastY<=y ? z+=0.05 : z-=0.05;

	if (boton == GLUT_MIDDLE_BUTTON)
		lastX<x ? x_+=0.01 : x_-=0.01;

	lastX = x;
	lastY = y;
#endif
}


void keys (unsigned char key, int x, int y)
{ 
	switch (key) {
		case 'l':
			glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
			break;
		case 'L':
			glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
			break;
		case ' ':
//			saverFlag=!saverFlag;	
			break;

	}
	giModel->updateLPos (key);
	
//	giModel2->updateLPos (lpos);
/*	giModel3->updateLPos (lpos);
*/
}


void specialKeys (int key, int x, int y)
{

	static char left=0, right=0, up=0, down=0;

	switch (key) {
	case GLUT_KEY_LEFT:
		if (keyDirection[2])
			keyDirection[2]=!keyDirection[2];
        
		left++;
		if (left==2){
			keyDirection[1] = !keyDirection[1];
			left=0;
			right=0;
		}
		
		break;
	case GLUT_KEY_RIGHT:
		if (keyDirection[1])
			keyDirection[1]=!keyDirection[1];
		
		right++;
		if (right==2){
			keyDirection[2] = !keyDirection[2];
			right=0;
			left=0;
		}
		
		break;
	case GLUT_KEY_UP:
		if (keyDirection[3])
			keyDirection[3]=!keyDirection[3];
		up++;
		if (up==2){
			keyDirection[0] = !keyDirection[0];
			up=0;
			down=0;
		}
		break;
	case GLUT_KEY_DOWN:
		if (keyDirection[0])
			keyDirection[0]=!keyDirection[0];
		down++;
		if (down==2){
			keyDirection[3] = !keyDirection[3];
			up=0;
			down=0;
		}
		break;
	}

}


int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize (512, 512); 
   glutInitWindowPosition (100,100);
   glutCreateWindow ("hello");
   glutReshapeFunc (reshape);
   init ();
   glutMouseFunc (mouse);
   glutMotionFunc (motion);
   glutKeyboardFunc (keys);
   glutSpecialFunc (specialKeys);
   glutDisplayFunc(display); 
   glutIdleFunc (idle);
 
   glutMainLoop();
   return 0;   
}
