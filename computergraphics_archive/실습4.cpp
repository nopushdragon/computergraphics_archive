#include <iostream>
#include <gl/glew.h> //--- �ʿ��� ������� include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <vector>
#include <random>

#define SIZEW 800
#define SIZEH 800
#define TimerSpeed 60

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_real_distribution<float> randcolor(0.0f, 1.0f);

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Timer(int value);

void init(bool b);

struct NEMO {
	float firstx, firsty;
	float x, y;
	float size;
	float r, g, b;
	bool b3 = false;
};
std::vector <NEMO> nemo;
bool is1 = false, is2 = false, is3 = false, is4 = false, is5 = false;


void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv); // glut �ʱ�ȭ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // ���÷��� ��� ����
	glutInitWindowPosition(100, 100); // �������� ��ġ ����
	glutInitWindowSize(SIZEW, SIZEH); // �������� ũ�� ����
	glutCreateWindow("Example1"); // ������ ����

	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) { // glew �ʱ�ȭ 
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	glutDisplayFunc(drawScene); // ��� �Լ��� ����
	glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(1000 / TimerSpeed, Timer, 1);
	glutMouseFunc(Mouse);
	glutMainLoop(); // �̺�Ʈ ó�� ����
}

GLvoid drawScene() { //--- �ݹ� �Լ�: ��� �ݹ� �Լ� {
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f); 
	glClear(GL_COLOR_BUFFER_BIT); // ������ ������ ��ü�� ĥ�ϱ�

	for (int i = 0; i < nemo.size();i++) {
		glColor3f(nemo[i].r, nemo[i].g, nemo[i].b);
		glRectf(nemo[i].x - nemo[i].size, nemo[i].y + nemo[i].size, nemo[i].x + nemo[i].size, nemo[i].y - nemo[i].size);
	}

	glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h) { //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ� {
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y) {

	switch (key) {
	case '1':
		init(is1);
		is1 = !is1;
		break;
	case '2':
		init(is2);
		is2 = !is2;
		break;
	case '3':
		init(is3);
		is3 = !is3;
		break;
	case '4':
		init(is4);
		is4 = !is4;
		break;
	case'5':
		init(is5);
		is5 = !is5;
		break;
	case's':
		init(NULL);
		break;
	case'm':
		for (int i = 0; i < nemo.size();i++) {
			nemo[i].x = nemo[i].firstx;
			nemo[i].y = nemo[i].firsty;
		}
		break;
	case 'r':
		init(NULL);
		while (!nemo.empty()) {
			nemo.pop_back();
		}
		break;
	case 'q':
		exit(0);
		break;
	}
}

GLvoid Mouse(int button, int state, int x, int y) {
	float mx = (float)x / (SIZEW / 2) - 1.0f;
	float my = (SIZEH - (float)y) / (SIZEH / 2) - 1.0f;

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		nemo.push_back({ mx,my, mx,my,0.1f,randcolor(mt),randcolor(mt) ,randcolor(mt) });
	}
}

GLvoid Timer(int value) {
	while (nemo.size() > 5) {
		nemo.pop_back();
	}
	
	if (is3) {
		for (int i = 0; i < nemo.size(); i++) {
			std::uniform_real_distribution<float> rdsize(0.005f, 0.02f);
			float addsize = rdsize(mt);

			if (nemo[i].b3 == false) {
				nemo[i].size += addsize;
				if (nemo[i].size >= 0.2f) nemo[i].b3 = true;
			}
			else {
				nemo[i].size -= addsize;
				if (nemo[i].size <= 0.01f) nemo[i].b3 = false;
			}
		}
	}
	else if (is4) {
		for (int i = 0; i < nemo.size(); i++) {
			nemo[i].r = randcolor(mt);
			nemo[i].g = randcolor(mt);
			nemo[i].b = randcolor(mt);
		}
	}

	glutTimerFunc(1000 / TimerSpeed, Timer, 1); //--- Ÿ�̸� �ݹ��Լ� ���� (60 FPS)
	glutPostRedisplay();
}

void init(bool b) {
	if (b != is1) is1 = false;
	if (b != is2) is2 = false;
	if (b != is3) is3 = false;
	if (b != is4) is4 = false;
	if (b != is5) is5 = false;
}