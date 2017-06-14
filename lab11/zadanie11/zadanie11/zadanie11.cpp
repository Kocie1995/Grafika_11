#include "stdafx.h"
#include <GL/glut.h>

#include <math.h>

#include <stdlib.h>

#include <time.h>

#include <sys/types.h>

#include <stdio.h>
#include "colors.h"

#define NUM_PARTICLES 1000 /* Number of particles */

#define NUM_DEBRIS 70 /* Number of debris */

/* GLUT menu entries */

#define PAUSE 0

#define NORMALIZE_SPEED 1

#define QUIT 2

// wskaz�wki jako�ci generacji mg�y

GLint fog_hint = GL_DONT_CARE;

// pocz�tek i koniec oddzia�ywania mg�y liniowej

GLfloat fog_start = 3.0;
GLfloat fog_end = 20.0;

// g�sto�� mg�y

GLfloat fog_density = 0.5;

// rodzaj mg�y

GLfloat fog_mode = GL_LINEAR;

// k�ty obrotu

GLfloat rotatex = 0.0;
GLfloat rotatey = 0.0;

// wska�nik naci�ni�cia lewego przycisku myszki

int button_state = GLUT_UP;

// po�o�enie kursora myszki

int button_x, button_y;



GLfloat vertex[11 * 3] =
{
	0.0f,   -.250f, 0.0f,
	0.0, -.250f,-0.70,
	-0.45,-.250f,-0.54,
	-0.69,-.250f,-0.12,
	-0.61,-.250f, 0.35,
	-0.24,-.250f, 0.66,
	0.24,-.250f, 0.66,
	0.61,-.250f, 0.35,
	0.69,-.250f,-0.12,
	0.45,-.250f,-0.54,
	0.0,   1.0,  0.0,
};
int triangles[18 * 3] =
{
	1,2,0,
	2,3,0,
	3,4,0,
	4,5,0,
	5,6,0,
	6,7,0,
	7,8,0,
	8,9,0,
	9,1,0,
	2,1,10,
	3,2,10,
	4,3,10,
	5,4,10,
	6,5,10,
	7,6,10,
	8,7,10,
	9,8,10,
	1,9,10,
};

void Normal(GLfloat *n, int i)
{
	GLfloat v1[3], v2[3];
	// obliczenie wektor�w na podstawie wsp�rz�dnych wierzcho�k�w tr�jk�t�w
	v1[0] = vertex[3 * triangles[3 * i + 1] + 0] - vertex[3 * triangles[3 * i + 0] + 0];
	v1[1] = vertex[3 * triangles[3 * i + 1] + 1] - vertex[3 * triangles[3 * i + 0] + 1];
	v1[2] = vertex[3 * triangles[3 * i + 1] + 2] - vertex[3 * triangles[3 * i + 0] + 2];
	v2[0] = vertex[3 * triangles[3 * i + 2] + 0] - vertex[3 * triangles[3 * i + 1] + 0];
	v2[1] = vertex[3 * triangles[3 * i + 2] + 1] - vertex[3 * triangles[3 * i + 1] + 1];
	v2[2] = vertex[3 * triangles[3 * i + 2] + 2] - vertex[3 * triangles[3 * i + 1] + 2];
	// obliczenie waktora normalnego przy pomocy iloczynu wektorowego
	n[0] = v1[1] * v2[2] - v1[2] * v2[1];
	n[1] = v1[2] * v2[0] - v1[0] * v2[2];
	n[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

// obs�uga przycisk�w myszki

void MouseButton(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		// zapami�tanie stanu lewego przycisku myszki
		button_state = state;

		// zapami�tanie po�o�enia kursora myszki
		if (state == GLUT_DOWN)
		{
			button_x = x;
			button_y = y;
		}
	}
}
// rozmiary bry�y obcinania

const GLdouble left = -2.0;
const GLdouble right = 2.0;
const GLdouble bottom = -2.0;
const GLdouble top = 2.0;


// obs�uga ruchu kursora myszki

void MouseMotion(int x, int y)
{
	if (button_state == GLUT_DOWN)
	{
		rotatey += 30 * (right - left) / glutGet(GLUT_WINDOW_WIDTH) *(x - button_x);
		button_x = x;
		rotatex -= 30 * (top - bottom) / glutGet(GLUT_WINDOW_HEIGHT) *(button_y - y);
		button_y = y;
		glutPostRedisplay();
	}
}

/* A particle */

struct particleData

{

	float position[3];

	float speed[3];

	float color[3];

};

typedef struct particleData particleData;

/* A piece of debris */

struct debrisData

{

	float position[3];

	float speed[3];

	float orientation[3]; /* Rotation angles around x, y, and z axes */

	float orientationSpeed[3];

	float color[3];

	float scale[3];

};

typedef struct debrisData debrisData;

/* Globals */

particleData particles[NUM_PARTICLES];

debrisData debris[NUM_DEBRIS];

int fuel = 0; /* "fuel" of the explosion */

float angle = 0.0; /* camera rotation angle */

				   /* Light sources and

				   material */

GLfloat light0Amb[4] = { 1.0, 0.6, 0.2, 1.0 };

GLfloat light0Dif[4] = { 1.0, 0.6, 0.2, 1.0 };

GLfloat light0Spec[4] = { 0.0, 0.0, 0.0, 1.0 };

GLfloat light0Pos[4] = { 0.0, 0.0, 0.0, 1.0 };

GLfloat light1Amb[4] = { 0.0, 0.0, 0.0, 1.0 };

GLfloat light1Dif[4] = { 1.0, 1.0, 1.0, 1.0 };

GLfloat light1Spec[4] = { 1.0, 1.0, 1.0, 1.0 };

GLfloat light1Pos[4] = { 0.0, 5.0, 5.0, 0.0 };

GLfloat materialAmb[4] = { 0.25, 0.22, 0.26, 1.0 };

GLfloat materialDif[4] = { 0.63, 0.57, 0.60, 1.0 };

GLfloat materialSpec[4] = { 0.99, 0.91, 0.81, 1.0 };

GLfloat materialShininess = 27.8;

int wantNormalize = 0; /* Speed vector normalization flag */

int wantPause = 0; /* Pause flag */

				   /*

				   * newSpeed

				   *

				   * Randomize a new speed vector.

				   *

				   */

void

newSpeed(float dest[3])

{

	float x;

	float y;

	float z;

	float len;

	x = (2.0 * ((GLfloat)rand()) / ((GLfloat)RAND_MAX)) - 1.0;

	y = (2.0 * ((GLfloat)rand()) / ((GLfloat)RAND_MAX)) - 1.0;

	z = (2.0 * ((GLfloat)rand()) / ((GLfloat)RAND_MAX)) - 1.0;


	/*

	* Normalizing the speed vectors gives a "fireball" effect

	*

	*/

	if (wantNormalize)

	{

		len = sqrt(x * x + y * y + z * z);

		if (len)

		{

			x = x / len;

			y = y / len;

			z = z / len;

		}

	}

	dest[0] = x;

	dest[1] = y;

	dest[2] = z;

}

/*

* newExplosion

*

* Create a new explosion.

*

*/

void newExplosion(void)

{

	int i;

	for (i = 0; i < NUM_PARTICLES; i++)

	{

		particles[i].position[0] = 0.0;

		particles[i].position[1] = 0.0;

		particles[i].position[2] = 0.0;

		particles[i].color[0] = 1.0;

		particles[i].color[1] = 1.0;

		particles[i].color[2] = 0.5;

		newSpeed(particles[i].speed);

	}

	for (i = 0; i < NUM_DEBRIS; i++)

	{

		debris[i].position[0] = 0.0;

		debris[i].position[1] = 0.0;

		debris[i].position[2] = 0.0;

		debris[i].orientation[0] = 0.0;

		debris[i].orientation[1] = 0.0;

		debris[i].orientation[2] = 0.0;

		debris[i].color[0] = 0.7;

		debris[i].color[1] = 0.7;

		debris[i].color[2] = 0.7;

		debris[i].scale[0] = (2.0 *

			((GLfloat)rand()) / ((GLfloat)RAND_MAX)) - 1.0;

		debris[i].scale[1] = (2.0 *

			((GLfloat)rand()) / ((GLfloat)RAND_MAX)) - 1.0;

		debris[i].scale[2] = (2.0 *

			((GLfloat)rand()) / ((GLfloat)RAND_MAX)) - 1.0;

		newSpeed(debris[i].speed);

		newSpeed(debris[i].orientationSpeed);

	}

	fuel = 100;

}

/*

* display

*

* Draw the scene.

*

*/

void display(void)

{

	int i;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	/* Place the camera */

	glTranslatef(0.0, 0.0, -10.0);

	glRotatef(angle, 0.0, 1.0, 0.0);


	// w��czenie efektu mg�y
	glEnable(GL_FOG);

	// wskaz�wki jako�ci generacji mg�y
	glHint(GL_FOG_HINT, fog_hint);

	// kolor mg�y
	glFogfv(GL_FOG_COLOR, Green);


	// g�sto�� mg�y
	glFogf(GL_FOG_DENSITY, fog_density);

	// rodzaj mg�y
	glFogf(GL_FOG_MODE, fog_mode);
	// pocz�tek i koniec oddzia�ywania mg�y liniowej
	glFogf(GL_FOG_START, fog_start);
	glFogf(GL_FOG_END, fog_end);

	/* If no explosion, draw cube */

	if (fuel == 0)

	{




		glEnable(GL_LIGHTING);

		glDisable(GL_LIGHT0);

		glEnable(GL_DEPTH_TEST);

		glBegin(GL_TRIANGLES);
		for (int i = 0; i < 18; i++)
		{
			GLfloat n[3];
			Normal(n, i);




			glNormal3fv(n);
			glVertex3fv(&vertex[3 * triangles[3 * i + 0]]);
			glVertex3fv(&vertex[3 * triangles[3 * i + 1]]);
			glVertex3fv(&vertex[3 * triangles[3 * i + 2]]);
		}

		// wy��czenie efektu mg�y
		glDisable(GL_FOG);

		GLfloat var[4];
		// g�sto�� mg�y
		glGetFloatv(GL_FOG_DENSITY, var);

		// pocz�tek i koniec oddzia�ywania mg�y liniowej
		glGetFloatv(GL_FOG_START, var);
		glGetFloatv(GL_FOG_END, var + 1);

		// koniec definicji sze�cianu RGB
		glEnd();

	}

	if (fuel > 0)

	{

		glPushMatrix();

		glDisable(GL_LIGHTING);

		glDisable(GL_DEPTH_TEST);

		glBegin(GL_POINTS);

		for (i = 0; i < NUM_PARTICLES; i++)

		{

			glColor3fv(particles[i].color);

			glVertex3fv(particles[i].position);

		}

		glEnd();

		glPopMatrix();

		glEnable(GL_LIGHTING);

		glEnable(GL_LIGHT0);

		glEnable(GL_DEPTH_TEST);

		glNormal3f(0.0, 0.0, 1.0);

		for (i = 0; i < NUM_DEBRIS; i++)

		{

			glColor3fv(debris[i].color);

			glPushMatrix();

			glTranslatef(debris[i].position[0],

				debris[i].position[1],

				debris[i].position[2]);

			glRotatef(debris[i].orientation[0], 1.0, 0.0, 0.0);

			glRotatef(debris[i].orientation[1], 0.0, 1.0, 0.0);

			glRotatef(debris[i].orientation[2], 0.0, 0.0, 1.0);

			glScalef(debris[i].scale[0],

				debris[i].scale[1],

				debris[i].scale[2]);

			glBegin(GL_TRIANGLES);

			glVertex3f(0.0, 0.5, 0.0);

			glVertex3f(-0.25, 0.0, 0.0);

			glVertex3f(0.25, 0.0, 0.0);

			glEnd();

			glPopMatrix();

		}

	}

	glutSwapBuffers();

}

/*

* keyboard

*

* Keyboard callback.

*

*/

void keyboard(unsigned char key,

	int x,

	int y)

{

	switch (key)

	{
	case '+':

		fog_density += 1.5;

		break;

	case '-':
		if (fog_density > 0.5)
			fog_density -= 1.5;

		break;

	case ' ':

		newExplosion();

		break;

	case 27:

		exit(0);

		break;

	case 'p':

		wantPause = 1 - wantPause;

		break;


	}
}

/*

* idle

*

* Update animation variables.

*

*/

void idle(void)

{

	int i;

	if (!wantPause)

	{

		if (fuel > 0)

		{

			for (i = 0; i < NUM_PARTICLES; i++)

			{

				particles[i].position[0] += particles[i].speed[0] * 0.2;

				particles[i].position[1] += particles[i].speed[1] * 0.2;

				particles[i].position[2] += particles[i].speed[2] * 0.2;

				particles[i].color[0] -= 1.0 / 500.0;

				if (particles[i].color[0] < 0.0)

				{

					particles[i].color[0] = 0.0;

				}

				particles[i].color[1] -= 1.0 / 100.0;

				if (particles[i].color[1] < 0.0)

				{

					particles[i].color[1] = 0.0;

				}

				particles[i].color[2] -= 1.0 / 50.0;

				if (particles[i].color[2] < 0.0)

				{

					particles[i].color[2] = 0.0;

				}

			}

			for (i = 0; i < NUM_DEBRIS; i++)

			{

				debris[i].position[0] += debris[i].speed[0] * 0.1;

				debris[i].position[1] += debris[i].speed[1] * 0.1;

				debris[i].position[2] += debris[i].speed[2] * 0.1;

				debris[i].orientation[0] += debris[i].orientationSpeed[0] * 10;

				debris[i].orientation[1] += debris[i].orientationSpeed[1] * 10;

				debris[i].orientation[2] += debris[i].orientationSpeed[2] * 10;

			}

			--fuel;

		}

		angle += 0.3; /* Always continue to rotate the camera */

	}

	glutPostRedisplay();

}

/*

* reshape

*

* Window reshape callback.

*

*/

void reshape(int w,

	int h)

{

	glViewport(0.0, 0.0, (GLfloat)w, (GLfloat)h);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(45.0, (GLfloat)w / (GLfloat)h, 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);

}

/*

* menuSelect

*

* Menu selection callback.

*

*/

void menuSelect(int value)

{

	switch (value)

	{

	case PAUSE:

		wantPause = 1 - wantPause;

		break;

	case NORMALIZE_SPEED:

		wantNormalize = 1 - wantNormalize;

		break;

	case QUIT:

		exit(0);

		break;

	}

}

/*

* main

*

* Setup OpenGL and hand over to GLUT.

*

*/

int main(int argc,

	char *argv[])

{


	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);

	glutCreateWindow("Explosion demo");

	glutKeyboardFunc(keyboard);

	glutIdleFunc(idle);

	glutDisplayFunc(display);

	glutReshapeFunc(reshape);

	srand(time(NULL));

	glEnable(GL_LIGHT0);

	glEnable(GL_LIGHT1);

	glLightfv(GL_LIGHT0, GL_AMBIENT, light0Amb);

	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Dif);

	glLightfv(GL_LIGHT0, GL_SPECULAR, light0Spec);

	glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);

	glLightfv(GL_LIGHT1, GL_AMBIENT, light1Amb);

	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1Dif);

	glLightfv(GL_LIGHT1, GL_SPECULAR, light1Spec);

	glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);

	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmb);

	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDif);

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpec);

	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);

	glEnable(GL_NORMALIZE);


	// obs�uga przycisk�w myszki
	glutMouseFunc(MouseButton);

	// obs�uga ruchu kursora myszki
	glutMotionFunc(MouseMotion);

	glutCreateMenu(menuSelect);

	glutAddMenuEntry("Pause", PAUSE);

	glutAddMenuEntry("Toggle normalized speed vectors", NORMALIZE_SPEED);

	glutAddMenuEntry("Quit", QUIT);

	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutMainLoop();

	return 0;

}