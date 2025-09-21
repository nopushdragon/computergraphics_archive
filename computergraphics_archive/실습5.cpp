#include <iostream>
#include <gl/glew.h> //--- �ʿ��� ������� include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <vector>
#include <random>

std::random_device rd;  //--- ���� ����
std::mt19937 mt(rd());
std::uniform_real_distribution<float> rdxy(-1.0f, 1.0f); // 0.0 ~ 1.0 ������ �յ� ����
std::uniform_real_distribution<float> rdcolor(0.0f, 1.0f);

#define SIZEW 800
#define SIZEH 800
#define TimerSpeed 60 //--- Ÿ�̸� �ӵ� ���� (60fps)

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Timer(int value);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid MouseMove(int x, int y);
GLvoid Keyboard(unsigned char key, int x, int y);

struct NEMO{
	float x, y; // ��ġ
	float r, g, b; // ����
};
std::vector<NEMO> nemo(40);

struct ERASER{
	float x, y; // ��ġ
	float r=0.0f, g=0.0f, b=0.0f;	// ����
	float size = 0.04f;
}eraser;

float size = 0.02f;
bool mouseDown = false;

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

	for(int i = 0 ; i < nemo.size() ; i++){
		nemo[i].x = rdxy(mt);
		nemo[i].y = rdxy(mt);
		nemo[i].r = rdcolor(mt);
		nemo[i].g = rdcolor(mt);
		nemo[i].b = rdcolor(mt);
	}

	glutDisplayFunc(drawScene); // ��� �Լ��� ����
	glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
	glutTimerFunc(1000 / TimerSpeed, Timer, 1); //--- Ÿ�̸� �ݹ��Լ� ���� (60 FPS)
	glutMouseFunc(Mouse);
	glutMotionFunc(MouseMove);
	glutKeyboardFunc(Keyboard);
	glutMainLoop(); // �̺�Ʈ ó�� ����
}

GLvoid drawScene() { //--- �ݹ� �Լ�: ��� �ݹ� �Լ� {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); // ������ ������ ��ü�� ĥ�ϱ�
	
	for(int i = 0 ; i < nemo.size() ; i++){
		glColor3f(nemo[i].r, nemo[i].g, nemo[i].b);
		glRectf(nemo[i].x - size,nemo[i].y -size, nemo[i].x + size, nemo[i].y +size);
	}

	if(mouseDown){
		glColor3f(eraser.r, eraser.g, eraser.b);
		glRectf(eraser.x - eraser.size,eraser.y - eraser.size, eraser.x + eraser.size, eraser.y + eraser.size);;
	}


	glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h) { //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ� {
	glViewport(0, 0, w, h);
}

GLvoid Timer(int value) { //--- �ݹ� �Լ�: Ÿ�̸� �ݹ� �Լ�

	if (mouseDown) {
		for (int i = 0; i < nemo.size(); i++) {
			 
			if (eraser.x - eraser.size < nemo[i].x + size &&
				eraser.x + eraser.size > nemo[i].x - size &&
				eraser.y - eraser.size < nemo[i].y + size &&
				eraser.y + eraser.size > nemo[i].y - size) {
				eraser.size += 0.005f;
				eraser.r = nemo[i].r;
				eraser.g = nemo[i].g;
				eraser.b = nemo[i].b;
				nemo.erase(nemo.begin() + i);
				break;
			}
		}
	}
	glutPostRedisplay(); // ȭ�� ���� ��û
	glutTimerFunc(1000 / TimerSpeed, Timer, 1); // Ÿ�̸� �缳�� (60 FPS)
}

GLvoid Mouse(int button, int state, int x, int y) { //--- �ݹ� �Լ�: ���콺 �ݹ� �Լ�
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		mouseDown = true;

		float mx = (2.0f * x / SIZEW) - 1.0f;
		float my = 1.0f - (2.0f * y / SIZEH);

		eraser.x = mx;
		eraser.y = my;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		mouseDown = false;
		eraser.r = 0.0f;
		eraser.g = 0.0f;
		eraser.b = 0.0f;
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		float mx = (2.0f * x / SIZEW) - 1.0f;
		float my = 1.0f - (2.0f * y / SIZEH);

		if(nemo.size() < 40){
			NEMO n;
			n.x = mx;
			n.y = my;
			n.r = rdcolor(mt);
			n.g = rdcolor(mt);
			n.b = rdcolor(mt);
			nemo.push_back(n);
			eraser.size -= 0.005f;
		}
	}
}

GLvoid MouseMove(int x, int y) {
	if (mouseDown) {
		float mx = (2.0f * x / SIZEW) - 1.0f;
		float my = 1.0f - (2.0f * y / SIZEH);
		eraser.x = mx;
		eraser.y = my;
	}
}

GLvoid Keyboard(unsigned char key, int x, int y) { //--- �ݹ� �Լ�: Ű���� �ݹ� �Լ�
	if (key == 'r') { 
		for(int i = 0 ; i < nemo.size() ; i++){
			nemo.pop_back();
		}
		for(int i = 0 ; i < 40 ; i++){
			NEMO n;
			n.x = rdxy(mt);
			n.y = rdxy(mt);
			n.r = rdcolor(mt);
			n.g = rdcolor(mt);
			n.b = rdcolor(mt);
			nemo.push_back(n);
		}

		eraser.size = 0.04f;
		eraser.r = 0.0f;
		eraser.g = 0.0f;
		eraser.b = 0.0f;
	}
}