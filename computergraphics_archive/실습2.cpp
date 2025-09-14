#include <iostream>
#include <gl/glew.h> 
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>
#include <array>

#define SIZE 800 

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Mouse(int button, int state, int x, int y);

float red = 0.0f, green = 0.5f, blue = 0.5f; 
std::random_device rd;
std::mt19937 mt(rd());
std::uniform_real_distribution<float> randcolor(0.0f, 1.0f);

struct NEMO {
	float r = randcolor(rd), g = randcolor(rd), b = randcolor(rd);
	int x1, x2, y1, y2 = 0;
};
std::array<NEMO, 4> nemo;

void main(int argc, char** argv) 
{
	
	glutInit(&argc, argv); 
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); 
	glutInitWindowPosition(100, 100); 
	glutInitWindowSize(800, 800); 
	glutCreateWindow("Example1"); 

	
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) 
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	nemo[0].x1 = -400, nemo[0].y1 = 400, nemo[0].x2 = 0, nemo[0].y2 = 0;
	nemo[1].x1 = 0, nemo[1].y1 = 400, nemo[1].x2 = 400, nemo[1].y2 = 0;
	nemo[2].x1 = -400, nemo[2].y1 = 0, nemo[2].x2 = 0, nemo[2].y2 = -400;
	nemo[3].x1 = 0, nemo[3].y1 = 0, nemo[3].x2 = 400, nemo[3].y2 = -400;
 
	glutDisplayFunc(drawScene); 
	glutReshapeFunc(Reshape); 
	glutMouseFunc(Mouse); 
	glutMainLoop(); 
}

GLvoid drawScene() { 
	glClearColor(red, green, blue, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); 
	for(int i = 0 ; i <nemo.size(); i++)
	{
		glColor3f(nemo[i].r, nemo[i].g, nemo[i].b);
		glRectf(nemo[i].x1, nemo[i].y1, nemo[i].x2, nemo[i].y2);
	}
	glutSwapBuffers();

}

GLvoid Reshape(int w, int h) { 
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-SIZE / 2, SIZE / 2, -SIZE / 2, SIZE / 2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

GLvoid Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		bool isleft = false;
		for(int i = 0; i < nemo.size(); i++)
		{
			if(x > nemo[i].x1 + 400 && x < nemo[i].x2 + 400 && y > 400 - nemo[i].y1 && y < 400 - nemo[i].y2)
			{
				nemo[i].r = randcolor(rd); 
				nemo[i].g = randcolor(rd);
				nemo[i].b = randcolor(rd);
				isleft = true;
			}
		}

		if(!isleft){
			red = randcolor(rd); 
			green = randcolor(rd);
			blue = randcolor(rd);
		}
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		bool isright = false;
		for (int i = 0; i < nemo.size(); i++)
		{
			if (x > nemo[i].x1 + 400 && x < nemo[i].x2 + 400 &&
				y > 400 - nemo[i].y1 && y < 400 - nemo[i].y2) 
			{
				if ((nemo[i].x2 - nemo[i].x1) > 60 && abs(nemo[i].y1 - nemo[i].y2) > 60) {
					nemo[i].x1 += 30;
					nemo[i].x2 -= 30;
					nemo[i].y1 -= 30;
					nemo[i].y2 += 30;
					isright = true;
				}
			}
		}

		if (!isright) {
			int num = 4;
			if (x > 0 && x < 400 && y > 0 && y < 400) {
				num = 0;
			}
			else if (x > 400 && x < 800 && y > 0 && y < 400) {
				num = 1;
			}
			else if (x > 0 && x < 400 && y > 400 && y < 600) {
				num = 2;
			}
			else if (x > 400 && x < 800 && y > 400 && y < 600) {
				num = 3;
			}
			if (num != 4) {
				if ((nemo[num].x2 - nemo[num].x1) < 400 && abs(nemo[num].y1 - nemo[num].y2) < 400) { 
					nemo[num].x1 -= 30;
					nemo[num].x2 += 30;
					nemo[num].y1 += 30;
					nemo[num].y2 -= 30;
				}
			}
		}
	}
	glutPostRedisplay(); 
}
