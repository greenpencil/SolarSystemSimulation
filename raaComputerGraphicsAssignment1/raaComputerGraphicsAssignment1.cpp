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
bool simulationStatus = false;
float initalSpeed = 0.004f;

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

void speedUpSimulation();
void speedDownSimulation();
void makeNewPlanet();

unsigned long g_ulLastTime = 0;
float g_fFrameTime = 0.0f;
float deltaMultipler = 0.0f;

const int trailLength = 200;
const float dampCoef = 0.98;


unsigned int g_uiLastTime = 0;

const static unsigned int csg_uiNumSpheres = 100;

const float G = -0.98f;

float g_afPos[3 * csg_uiNumSpheres];
float g_afCol[3 * csg_uiNumSpheres];
float g_afSizes[csg_uiNumSpheres];

float g_moonColour[3];

float rUp[3] = { 0,1,0 };

typedef struct _node
{
	float radius;
	float mass;
	// x, y, z 0, 1, 2

	float position[3];
	float velocity[3];
	float colour[4];

	float trail[trailLength][3];

	// define our node type
	_node *nNext = 0;
	_node *nLast = 0;

	int content = 0;
	int refCount;

} node;

node *head;
node *tail;

node* createElement(float radius, float mass, float posX, float posY, float posZ, float velX, float velY, float velZ);

void detectCollision(node *n);

void addToEndOfList(node *n);
void addToStartOfList(node *n);
void addTopositionInList(node *n, int pos);

void insertBefore(node* at, node *newNode);

void removeFromList(node *nodeToDelete);

void traverseList(node *head);
void printNode(node *n);
void changePos(node *n);

void changeVelocity(node *n);

int countElements();


node* getAt(int iPos);

void printFVec(float vector[3])
{
	printf("x: %f, y: %f, z: %f\n", vector[0], vector[1], vector[2]);
}

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
	//gridDraw(g_ulGrid);

	// this is a placeholder, you should replace it with instructions to draw your planet system
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_LIGHTING);

	glEnable(GL_LINE_SMOOTH);



	for (int i = 0; i < countElements(); i++)
	{
		node *n = getAt(i);
		glPushMatrix();
		glTranslatef(n->position[0], n->position[1], n->position[2]);
		float normalColour[3];
		vecNormalise(n->colour, normalColour);
		colToMat(normalColour, 1.0f);

		glutSolidSphere(n->radius, 10, 10);
		//drawSphere(n->radius, 10, 10);
		glPopMatrix();

		glDisable(GL_LIGHTING);
		glColor3ub(n->colour[0], n->colour[1], n->colour[2]);
		glLineWidth(1.0f);

		glBegin(GL_LINES);
		for (int j = 0; j < trailLength; j++)
		{
			//glTranslatef(n->trail[j][0], n->trail[j][1], n->trail[j][2]);
			//glutSolidTeapot(10);
			glVertex3f(n->trail[j][0], n->trail[j][1], n->trail[j][2]);
			//glutSolidSphere(1.0, 50, 50);
			//drawSphere(10, 10, 10);
		}
		glEnd();
		glEnable(GL_LIGHTING);

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

	unsigned long ulTime = timeGetTime();
	g_fFrameTime = (ulTime - g_ulLastTime);
	g_ulLastTime = ulTime;

	changePos(head);

	for (int i = 0; i < countElements(); i++)
	{
		node *n = getAt(i);
		if(n != head)
		{
			changeVelocity(n);
			changePos(n);
			detectCollision(n);
			
		}
		
	}

	//f = g * (m[body]*m[other body]) / ((s[body]-s[other body])*(s[body]-s[other body]))


	glutPostRedisplay();
}

//not working
void detectCollision(node *n)
{
	float distance = vecDistance(n->position, head->position);
	if (distance < head->radius)
	{
		//printf("hit the sun \n");
	}
}

float calcGravity(node *thisBody, node *bodyToCalcAgainst)
{

	float aGravity = G * (thisBody->mass * bodyToCalcAgainst->mass) / (vecDistance(thisBody->position, bodyToCalcAgainst->position) * vecDistance(thisBody->position, bodyToCalcAgainst->position));
	return aGravity;
}

void changeVelocity(node *n)
{
	/*printf("G = %f, n->mass = %f, head->mass = %f (n->mass * head->mass) = %f\n", G, n->mass, head->mass, (n->mass * head->mass));

	printf("n->position: ");
	printFVec(n->position);
	printf("head->position: ");
	printFVec(head->position);

	printf("vecDistance(n->position, head->position): %f  vecDistance(n->position, head->position) * vecDistance(n->position, head->position): %f\n", vecDistance(n->position, head->position), vecDistance(n->position, head->position) * vecDistance(n->position, head->position));*/
	

	for (int i = 0; i < countElements(); i++)
	{
		node* bodyToCheck = getAt(i);
		if (bodyToCheck != n)
		{
			float aGravity = calcGravity(n, bodyToCheck);

			//printf("aGravity: %f\n", aGravity);

			float normPos[3];


			//printf("Before Velocity: ");
			//printFVec(n->velocity);

			vecNormalise(n->position, normPos);
			vecScalarProduct(normPos, aGravity, normPos);


			float adjustAcc[3];
			vecScalarProduct(normPos, g_fFrameTime * deltaMultipler * dampCoef, adjustAcc);


			vecAdd(n->velocity, adjustAcc, n->velocity);
		}
	}
	
	//printf("After Velocity: ");
	//printFVec(n->velocity);

}

void changePos(node *n)
{
	float adjustVelocity[3];
	vecScalarProduct(n->velocity, g_fFrameTime * deltaMultipler, adjustVelocity);
	vecAdd(n->position, adjustVelocity, n->position);
	int i;
	for (i = trailLength-1; i > 0; i--) {
		n->trail[i][0] = n->trail[i - 1][0];
		n->trail[i][1] = n->trail[i - 1][1];
		n->trail[i][2] = n->trail[i - 1][2];
	}
	vecSet(n->position[0], n->position[1], n->position[2], n->trail[0]);
}


void myInit()
{
	head = createElement(100.0f, 80000.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	// 255, 240, 79
	float r = 255;
	float b = 240;
	float g = 79;
	vecSet(r, b, g, head->colour);
	addToStartOfList(head);

	float afGridColour[] = { 0.3f, 0.3f, 0.1f, 0.3f };

	initMaths();

	for (unsigned int i = 0; i < csg_uiNumSpheres; i++)
	{
		makeNewPlanet();
	}


	g_ulLastTime = timeGetTime();

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

void makeNewPlanet()
{
	float xVelocity[3];
	//(-30.0f*Sun radius, -22.0f, -30.0f*Sun radius)
	//	(30.0f*Sun radius, 22.0f, 30.0f*Sun radius)

	node* nElement = createElement(randFloat(5.0f, 50.0f), randFloat(1.0f, 1000.0f),
		randFloat(-30.0f * head->radius, 30.0f*head->radius), randFloat(-22.0f, 22.0f), randFloat(-10.0f * head->radius, 10.0f*head->radius),
		0.0f, 0.0f, 0.0f);

	float r = randFloat(0, 255.0f);
	float b = randFloat(0, 255.0f);
	float g = randFloat(0, 255.0f);
	vecSet(r, b, g, nElement->colour);

	vecCrossProduct(rUp, nElement->position, xVelocity);
	vecNormalise(xVelocity, xVelocity);
	float randVecRes = randFloat(1.0f, 300.0f);

	vecSet(xVelocity[0] * randVecRes, xVelocity[1] * randVecRes, xVelocity[2] * randVecRes, nElement->velocity);
	addToEndOfList(nElement);
}

void speedUpSimulation() {
	deltaMultipler = deltaMultipler + 0.0001f;
	printf("Current Simulation Speed: %f\n", deltaMultipler * 1000);
}
void speedDownSimulation() {
	deltaMultipler = deltaMultipler - 0.0001f;

	printf("Current Simulation Speed: %f\n", deltaMultipler * 1000);
}

void stopStartSimulation() {
	if (simulationStatus)
	{
		simulationStatus = false;
		deltaMultipler = 0.0f;
	}
	else
	{
		simulationStatus = true;
		deltaMultipler = initalSpeed;
	}
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
	case '[':
		speedUpSimulation();
		break;
	case ']':
		speedDownSimulation();
		break;
	case 'p':
		stopStartSimulation();
		break;
	case 'o':
		makeNewPlanet();
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

node* createElement(float radius, float mass, float posX, float posY, float posZ, float velX, float velY, float velZ)
{
	node *newNode = new node;
	newNode->radius = radius;
	newNode->mass = mass;
	//newNode->position = new int[3];
	newNode->position[0] = posX;
	newNode->position[1] = posY;
	newNode->position[2] = posZ;
	newNode->velocity[0] = velX;
	newNode->velocity[1] = velY;
	newNode->velocity[2] = velZ;
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

void addTopositionInList(node *n, int pos)
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