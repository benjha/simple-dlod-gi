#include <GL/glew.h>
#include <GL/glut.h>
#include "../Include/GI.h"
#include "../Include/openEXR.h"
#include "../Include/giLod.h"


cGILod *m=NULL;

int lastx=0, lasty=0,boton;

void display(void)
{
	glClearColor (1.0, 1.0, 0.0, 1.0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glPushMatrix();
	glRotatef(lasty,1.0,0.0,0.0);
	glRotatef(lastx,0.0,1.0,0.0);	

	
	glPopMatrix ();


	glutSwapBuffers ();
}

void idle (void)
{
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

	
	m = new cGILod (512,512,10);

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
}

void mouse( int button, int state, int x, int y )
{
    boton= button;
}

void move(int x, int y)
{

	switch (boton) {
	case 0:
		lastx = x;
		lasty = y;
		break;
	}
}


int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize (512, 512); 
   glutInitWindowPosition (100, 100);
   glutCreateWindow ("hello");
   glutReshapeFunc (reshape);
   glutMouseFunc(mouse);
   glutMotionFunc (move);
   init ();
   glutDisplayFunc(display); 
   glutIdleFunc (idle);
   glutMainLoop();
   
   return 0;   /* ANSI C requires main to return int. */
}
