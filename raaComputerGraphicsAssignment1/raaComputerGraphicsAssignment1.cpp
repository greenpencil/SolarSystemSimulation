#include <stdio.h>
#include <time.h>
#include <stdlib.h>
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
unsigned long g_ulGrid = 0;
int g_aiLastMouse[2];
int g_aiStartMouse[2];
bool g_bExplore = false;
bool g_bFly = false;

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

unsigned int g_uiLastTime = 0;

const static unsigned int csg_uiNumSpheres = 10;

float g_afPos[3 * csg_uiNumSpheres];
float g_afCol[3 * csg_uiNumSpheres];
float g_afSizes[csg_uiNumSpheres];

float g_moonColour[3];

float g_fRot_moon_orbit = 0.0f;
float g_fRot_moon = 0.0f;
float g_fRot_earth = 0.0f;

typedef struct _vector3d {
	float x;
	float y;
	float z;
} vector3d;

vector3d createVector(float x, float y, float z);

typedef struct _node
{
	float radius;
	float mass;

	vector3d postion;
	vector3d velocity;

	// define our node type
	_node *nNext = 0;
	_node *nLast = 0;

	int content = 0;
	int refCount;

} node;

node *head;
node *tail;

node* createElement(float radius, float mass, vector3d postion, vector3d velocity);


void addToEndOfList(node *n);
void addToStartOfList(node *n);
void addToPostionInList(node *n, int pos);

void insertBefore(node* at, node *newNode);

void removeFromList(node *nodeToDelete);

void traverseList(node *head);
void printNode(node *n);

int countElements();


node* getAt(int iPos);

void ref(node *n)
{
	if (n)n->refCount++;
}



// Little code as possible, should be looping through list
void display()
{
	glEnable(GL_NORMALIZE);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	camApply(g_Camera);

	// comment out this line to remove grid
	gridDraw(g_ulGrid);

	// this is a placeholder, you should replace it with instructions to draw your planet system
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_LIGHTING);
	/*for (unsigned int i = 0; i < csg_uiNumSpheres; i++)
	{
	glPushMatrix();
	glTranslatef(g_afPos[i * 3], g_afPos[i * 3+1], g_afPos[i * 3+2]);
	colToMat(g_afCol+3*i);
	glutSolidSphere(g_afSizes[i], 10, 10);
	glPopMatrix();
	}*/

	glPushMatrix();
	//glRotatef(degToRad(g_fRot_earth), 0.0f, 1.0f, 0.0f);

	glTranslatef(head->postion.x, head->postion.y, head->postion.z);
	//colToMat(g_afCol + 3 * 1);
	drawSphere(80.0f, 10, 10);
	//glutSolidSphere(10.0f, 10, 10);
	glPopMatrix();


	glPushMatrix();
	//glRotatef(degToRad(g_fRot_moon_orbit), 0.0f, 1.0f, 0.0f);
	//glRotatef(degToRad(g_fRot_moon), 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, 150.0f);

	//colToMat(g_moonColour, 1.0f);
	drawSphere(20.0f, 10, 10);
	//glutSolidSphere(20.0f, 10, 10);
	glPopMatrix();


	for (int i = 0; i < countElements(); i++)
	{
		node *n = getAt(i);
		if (n != head)
		{
			glPushMatrix();
			glTranslatef(n->postion.x, n->postion.y, n->postion.z);
			drawSphere(n->radius, 10, 10);
			glPopMatrix();
		}

	}


	glPopAttrib();
	// end placeholder

	glFlush();
	glutSwapBuffers();
}

// idle function happens when glut isn't doing anything
void idle()
{
	mouseMotion();

	// simulation progression code should go here

	//g_fRot_moon_orbit += 30.0f;
	//g_fRot_earth += 20.0f;
	//g_fRot_moon += 20.0f;

	head->postion.x = head->postion.x + head->velocity.x;
	head->postion.y = head->postion.y + head->velocity.y;
	head->postion.z = head->postion.z + head->velocity.z;

	for (int i = 0; i < countElements(); i++)
	{
		node *n = getAt(i);
		if(n != head)
		{
			n->postion.x = n->postion.x + n->velocity.x;
			n->postion.y = n->postion.y + n->velocity.y;
			n->postion.z = n->postion.z + n->velocity.z;
		}
		
	}


	glutPostRedisplay();
}

void reshape(int iWidth, int iHeight)
{
	glViewport(0, 0, iWidth, iHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0f, ((float)iWidth) / ((float)iHeight), 0.1f, 10000.0f);
	glMatrixMode(GL_MODELVIEW);
	glutPostRedisplay();
}

void keyboard(unsigned char c, int iXPos, int iYPos)
{
	switch (c)
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
	switch (c)
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
	switch (iC)
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
	switch (iC)
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
	if (g_Input.m_bMouse) camInputSetMouseLast(g_Input, iXPos, iYPos);
}

void mouseMotion()
{
	camProcessInput(g_Input, g_Camera);
	glutPostRedisplay();
}

void myInit()
{
	head = createElement(80, 80, createVector(0.0f, 0.0f, 0.0f), createVector(0.0f, 0.0f, 1.0f));
	addToStartOfList(head);

	float afGridColour[] = { 0.3f, 0.3f, 0.1f, 0.3f };

	initMaths();

	for (unsigned int i = 0; i < csg_uiNumSpheres; i++)
	{
		node* nElement = createElement(20, 20, createVector(randFloat(-600.0f, 600.0f), randFloat(-600.0f, 600.0f), randFloat(-600.0f, 600.0f)), createVector(0.0f, 0.0f, 1.0f));
		addToEndOfList(nElement);
	}

	printf("Elements: %i", countElements());

	// colour between 1 and 255 must be divded by /255
	// 170 becomes 0.6862745098
	g_moonColour[0] = 0.6862745098f;
	g_moonColour[1] = 0.6862745098f;
	g_moonColour[2] = 0.6862745098f;

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
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(512, 384);
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

vector3d createVector(float x, float y, float z)
{
	vector3d *v3d = new vector3d;
	v3d->x = x;
	v3d->y = y;
	v3d->z = z;

	return *v3d;
}

node* createElement(float radius, float mass, vector3d postion, vector3d velocity)
{
	node *newNode = new node;
	newNode->radius = radius;
	newNode->mass = mass;
	newNode->postion = postion;
	newNode->velocity = velocity;
	return newNode;
}

void addToEndOfList(node *n)
{
	tail->nNext = n;
	n->nLast = tail;
	tail = n;
}

void addToStartOfList(node *n)
{
	if (tail == 0)
	{
		tail = head = n;
		head->nLast = 0;
	}
	else {
		head->nLast = n;
	}
	n->nNext = head;
	head = n;
}

void addToPostionInList(node *n, int pos)
{
	insertBefore(n, getAt(pos));
}

void insertBefore(node *newNode, node* at)
{
	newNode->nLast = at->nLast;
	at->nLast->nNext = newNode;
	newNode->nNext = at;
	at->nLast = newNode;
}


void traverseList(node *head)
{
	node *current;
	current = head;
	while (current != 0)
	{
		current = current->nNext;
	}
}

node* getAt(int iPos)
{
	node *n = head;

	while (iPos-- && n) n = n->nNext;

	return n;
}


int countElements()
{
	int total = 0;
	node *current = head;
	while (current != 0)
	{
		total++;
		current = current->nNext;
	}

	return total;
}

void printNode(node *n)
{
	printf("Value %d \n", n->content);
}

void removeFromList(node *nodeToDelete) {
	if (nodeToDelete == head)
	{
		node *newHead = head->nNext;
		head->nNext = 0;
		newHead->nLast = 0;
		delete(head);
		head = newHead;
	}
	else if (nodeToDelete == tail) {
		node *newTail = tail->nLast;
		tail->nLast = 0;
		newTail->nNext = 0;
		delete(tail);
		tail = newTail;
	}
	else {
		node *lastNode = nodeToDelete->nLast;
		node *nextNode = nodeToDelete->nNext;

		lastNode->nNext = nextNode;
		nextNode->nLast = lastNode;

		delete(nodeToDelete);
	}
}