#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <mmsystem.h>
#include "raaCamera/raaCamera.h"
#include "raaUtilities/raaUtilities.h"
#include "raaMaths/raaMaths.h"
#include "raaMaths/raaVector.h"
#include "raaMaths/raaMatrix.h"

raaCameraInput g_Input;
raaCamera g_Camera;
unsigned long g_ulGrid=0;
int g_aiLastMouse[2];
int g_aiStartMouse[2];
bool g_bExplore=false;
bool g_bFly=false;

void gridInit();
void display();
void idle();
void reshape(int iWidth, int iHeight);
void keyboard(unsigned char c, int iXPos, int iYPos);
void keyboardUp(unsigned char c, int iXPos, int iYPos);
void sKeyboard(int iC, int iXPos, int iYPos);
void sKeyboardUp(int iC, int iXPos, int iYPos);
void mouse(int iKey, int iEvent, int iXPos, int iYPos);
void mouseMotion();
void myInit();

unsigned int g_uiLastTime=0;

const static unsigned int csg_uiNumSpheres = 100;

float g_afPos[3 * csg_uiNumSpheres];
float g_afCol[3 * csg_uiNumSpheres];
float g_afSizes[csg_uiNumSpheres];

void display()
{
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	camApply(g_Camera);

	// comment out this line to remove grid
	gridDraw(g_ulGrid);

	// this is a placeholder, you should replace it with instructions to draw your planet system
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_LIGHTING);
	for (unsigned int i = 0; i < csg_uiNumSpheres; i++)
	{
		glPushMatrix();
		glTranslatef(g_afPos[i * 3], g_afPos[i * 3+1], g_afPos[i * 3+2]);
		colToMat(g_afCol+3*i);
		glutSolidSphere(g_afSizes[i], 10, 10);
		glPopMatrix();
	}
	glPopAttrib();
	// end placeholder

	glFlush(); 
	glutSwapBuffers();
}

void idle()
{
	mouseMotion();

	// simulation progression code should go here

	glutPostRedisplay();
}

void reshape(int iWidth, int iHeight)
{
	glViewport(0, 0, iWidth, iHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0f, ((float)iWidth)/((float)iHeight), 0.1f, 10000.0f);
	glMatrixMode(GL_MODELVIEW);
	glutPostRedisplay();
}

void keyboard(unsigned char c, int iXPos, int iYPos)
{
	switch(c)
	{
	case 'w':
		camInputTravel(g_Input, tri_pos);
		break;
	case 's':
		camInputTravel(g_Input, tri_neg);
		break;
	}
}

void keyboardUp(unsigned char c, int iXPos, int iYPos)
{
	switch(c)
	{
		case 'w':
		case 's':
			camInputTravel(g_Input, tri_null);
			break;
		case 'f':
			camInputFly(g_Input, !g_Input.m_bFly);
			break;
	}
}

void sKeyboard(int iC, int iXPos, int iYPos)
{
	switch(iC)
	{
		case GLUT_KEY_UP:
			camInputTravel(g_Input, tri_pos);
			break;
		case GLUT_KEY_DOWN:
			camInputTravel(g_Input, tri_neg);
			break;
	}
}

void sKeyboardUp(int iC, int iXPos, int iYPos)
{
	switch(iC)
	{
		case GLUT_KEY_UP:
		case GLUT_KEY_DOWN:
			camInputTravel(g_Input, tri_null);
			break;
	}
}

void mouse(int iKey, int iEvent, int iXPos, int iYPos)
{
	if (iKey == GLUT_LEFT_BUTTON)
	{
		camInputMouse(g_Input, (iEvent == GLUT_DOWN) ? true : false);
		if (iEvent == GLUT_DOWN)camInputSetMouseStart(g_Input, iXPos, iYPos);
	}
}

void motion(int iXPos, int iYPos)
{
	if(g_Input.m_bMouse) camInputSetMouseLast(g_Input, iXPos, iYPos);
}

void mouseMotion()
{
	camProcessInput(g_Input, g_Camera);
	glutPostRedisplay();
}

void myInit()
{
	float afGridColour[] = { 0.3f, 0.3f, 0.1f, 0.3f };
	initMaths();

	for(unsigned int i=0;i<csg_uiNumSpheres;i++)
	{
		vecRand(-600.0f, 600.0f, g_afPos + 3 * i);
		vecRand(0.3f, 1.0f, g_afCol + 3 * i);
		g_afSizes[i]=randFloat(10.0f, 80.0f);
	}

	camInit(g_Camera);
	camInputInit(g_Input);
	camInputExplore(g_Input, true);
	gridInit(g_ulGrid, afGridColour, -500, 500, 50.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


int main(int argc, char* argv[])
{
	glutInit(&argc, (char**)argv); 

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0,0);
	glutInitWindowSize(512,384);
	glutCreateWindow("raaAssignment1-2016");

	myInit();

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutSpecialFunc(sKeyboard);
	glutSpecialUpFunc(sKeyboardUp);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMainLoop();
	return 0;
}
