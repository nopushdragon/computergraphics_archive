#include <iostream>
#include <gl/glew.h> //--- �ʿ��� ������� include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid timer_randcolor(int value);

float red = 1.0f, green = 1.0f, blue = 1.0f; //--- ������ �����ϴ� ��������
bool istimer = false;
std::random_device rd;
std::mt19937 mt(rd());
std::uniform_real_distribution<float> randcolor(0.0f, 1.0f);

void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv); // glut �ʱ�ȭ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // ���÷��� ��� ����
	glutInitWindowPosition(100, 100); // �������� ��ġ ����
	glutInitWindowSize(800, 600); // �������� ũ�� ����
	glutCreateWindow("Example1"); // ������ ����

	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) // glew �ʱ�ȭ 
	{ 
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	glutDisplayFunc(drawScene); // ��� �Լ��� ����
	glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
	glutKeyboardFunc(Keyboard); //--- Ű���� �Է� �ݹ��Լ� ����
	glutMainLoop(); // �̺�Ʈ ó�� ����
}

GLvoid drawScene() { //--- �ݹ� �Լ�: ��� �ݹ� �Լ� {
	glClearColor(red, green, blue, 1.0f); // �������� ��blue���� ����
	glClear(GL_COLOR_BUFFER_BIT); // ������ ������ ��ü�� ĥ�ϱ�
	glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h) { //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ� {
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'c':	//--- ������ û�ϻ����� ����
		red = 0;
		green = 1.0;
		blue = 1.0; 
		break; 
	case 'm':	//--- ������ ��ȫ������ ����
		red = 1.0;
		green = 0.0;
		blue = 1.0;
		break; 
	case 'y':;	//--- ������ ��������� ����
		red = 1.0;
		green = 1.0;
		blue = 0.0;
		break;
	case 'a': {
		red = randcolor(rd);
		green = randcolor(rd);
		blue = randcolor(rd);
		}
		break;
	case 'w':
		red = 1.0;
		green = 1.0;
		blue = 1.0;
		break;
	case 'k':
		red = 0.0;
		green = 0.0;
		blue = 0.0;
		break;
	case 't':
		glutTimerFunc(1000 / 60, timer_randcolor, 1); //--- Ÿ�̸� �ݹ��Լ� ���� (60 FPS)
		istimer = true;
		break;
	case 's':
		istimer = false;
		break;
	case 'q':
		exit(0); //--- 'q'Ű�� ������ ���α׷� ����
		break;
	}
	glutPostRedisplay(); //--- ������ �ٲ� ������ ��� �ݹ� �Լ��� ȣ���Ͽ� ȭ���� refresh �Ѵ�
}

GLvoid timer_randcolor(int value) { //--- �ݹ� �Լ�: Ÿ�̸� �ݹ� �Լ�
	if (istimer) {
		red = randcolor(rd);
		green = randcolor(rd);
		blue = randcolor(rd);
		glutTimerFunc(1000 / 60, timer_randcolor, 1); //--- Ÿ�̸� �ݹ��Լ� ���� (60 FPS)
	}
	glutPostRedisplay(); // ȭ�� ���� ��û
}