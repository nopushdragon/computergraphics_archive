#include <iostream>
#include <gl/glew.h> //--- �ʿ��� ������� include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>
#include <vector>

#define SIZEW 800
#define SIZEH 800
#define TimerSpeed 60 //--- Ÿ�̸� �ӵ� ���� (60 FPS)

std::random_device rd; 
std::mt19937 mt(rd());
std::uniform_real_distribution<float> rdxy(-0.9f, 0.9f);
std::uniform_real_distribution<float> rdcolor(0.0f, 1.0f);
std::uniform_int_distribution<int> rdcount(5, 10);
std::uniform_real_distribution<float> rdsize(0.05f, 0.2f);

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Timer(int value);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Keyboard(unsigned char key, int x, int y);

struct NEMO
{
	float x, y; // ��ġ
	float r, g, b; // ����
	float size; // ũ��
};
std::vector<NEMO> nemo;
std::vector<NEMO> particle;

int mode = 1;

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

	for(int i=0; i<rdcount(mt); i++)
	{
		NEMO n;
		n.x = rdxy(mt);
		n.y = rdxy(mt);
		n.r = rdcolor(mt);
		n.b = rdcolor(mt);
		n.g = rdcolor(mt);
		n.size = rdsize(mt);
		nemo.push_back(n);
	}

	glutDisplayFunc(drawScene); // ��� �Լ��� ����
	glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
	glutTimerFunc(1000 / TimerSpeed, Timer, 1); //--- Ÿ�̸� �ݹ��Լ� ���� (60 FPS)
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutMainLoop(); // �̺�Ʈ ó�� ����
}

GLvoid drawScene() { //--- �ݹ� �Լ�: ��� �ݹ� �Լ� {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // �������� ��blue���� ����
	glClear(GL_COLOR_BUFFER_BIT); // ������ ������ ��ü�� ĥ�ϱ�
	
	for(int i =0; i<nemo.size(); i++)
	{
		glColor3f(nemo[i].r, nemo[i].g, nemo[i].b);
		glRectf(nemo[i].x - nemo[i].size, nemo[i].y - nemo[i].size, nemo[i].x + nemo[i].size, nemo[i].y + nemo[i].size);
	}

	if(particle.size() > 0){
		for(int i =0; i<particle.size(); i++)
		{
			glColor3f(particle[i].r, particle[i].g, particle[i].b);
			glRectf(particle[i].x - particle[i].size, particle[i].y - particle[i].size, particle[i].x + particle[i].size, particle[i].y + particle[i].size);
			
		}
	}


	glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h) { //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ� {
	glViewport(0, 0, w, h);
}

GLvoid Timer(int value) { //--- �ݹ� �Լ�: Ÿ�̸� �ݹ� �Լ� {
	
	if(particle.size() > 0){
		for (int i = 0; i < particle.size(); i++) {
			if (mode == 1) {
				if ((i + 1) % 4 == 1) {
					particle[i].x -= 0.01f;
					particle[i].size -= 0.002f;
				}
				else if ((i + 1) % 4 == 2) {
					particle[i].y += 0.01f;
					particle[i].size -= 0.002f;
				}
				else if ((i + 1) % 4 == 3) {
					particle[i].y -= 0.01f;
					particle[i].size -= 0.002f;
				}
				else if ((i + 1) % 4 == 0) {
					particle[i].x += 0.01f;
					particle[i].size -= 0.002f;
				}
			}
			else if (mode == 2) {
				if ((i + 1) % 4 == 1) {
					particle[i].x -= 0.01f;
					particle[i].y += 0.01f;
					particle[i].size -= 0.002f;
				}
				else if ((i + 1) % 4 == 2) {
					particle[i].x += 0.01f;
					particle[i].y += 0.01f;
					particle[i].size -= 0.002f;
				}
				else if ((i + 1) % 4 == 3) {
					particle[i].x -= 0.01f;
					particle[i].y -= 0.01f;
					particle[i].size -= 0.002f;
				}
				else if ((i + 1) % 4 == 0) {
					particle[i].x += 0.01f;
					particle[i].y -= 0.01f;
					particle[i].size -= 0.002f;
				}
			}
			else if (mode == 3) {
				if ((i + 1) % 4 == 1) {
					particle[i].x -= 0.01f;
					particle[i].size -= 0.002f;
				}
				else if ((i + 1) % 4 == 2) {
					particle[i].x -= 0.01f;
					particle[i].size -= 0.002f;
				}
				else if ((i + 1) % 4 == 3) {
					particle[i].x -= 0.01f;
					particle[i].size -= 0.002f;
				}
				else if ((i + 1) % 4 == 0) {
					particle[i].x -= 0.01f;
					particle[i].size -= 0.002f;
				}
			}
			else if (mode == 4) {
				if ((i % 8) == 0) {
					particle[i].x -= 0.01f;
					particle[i].y += 0.01f;
				}
				else if ((i % 8) == 1) {
					particle[i].y += 0.01f;
				}
				else if ((i % 8) == 2) {
					particle[i].x += 0.01f;
					particle[i].y += 0.01f;
				}
				else if ((i % 8) == 3) {
					particle[i].x -= 0.01f;
				}
				else if ((i % 8) == 4) {
					particle[i].x += 0.01f;
				}
				else if ((i % 8) == 5) {
					particle[i].x -= 0.01f;
					particle[i].y -= 0.01f;
				}
				else if ((i % 8) == 6) {
					particle[i].y -= 0.01f;
				}
				else if ((i % 8) == 7) {
					particle[i].x += 0.01f;
					particle[i].y -= 0.01f;
				}
				particle[i].size -= 0.002f;
			}

			for (int i = 0; i < particle.size(); i++) {
				if (particle[i].size <= 0.0f) {
					particle.erase(particle.begin() + i);
				}
			}
		}
	}

	glutPostRedisplay(); // �ٽ� �׸���
	glutTimerFunc(1000 / TimerSpeed, Timer, 1); // Ÿ�̸� �缳�� (60 FPS)
}

GLvoid Mouse(int button, int state, int x, int y) { //--- �ݹ� �Լ�: ���콺 �ݹ� �Լ� {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

		// ���콺 ���� ��ư�� ������ �� ó���� ����
		float mx = (float(x) / SIZEW) * 2.0f - 1.0f;
		float my = 1.0f - (float(y) / SIZEH) * 2.0f;

		for (int i = 0; i < nemo.size();i++) {
			if(mx >= nemo[i].x - nemo[i].size && mx <= nemo[i].x + nemo[i].size &&
			   my >= nemo[i].y - nemo[i].size && my <= nemo[i].y + nemo[i].size) {

				if (mode != 4) {
					NEMO p[4];
					p[0].x = nemo[i].x - (nemo[i].size / 2.0f);
					p[0].y = nemo[i].y + (nemo[i].size / 2.0f);
					p[1].x = nemo[i].x + (nemo[i].size / 2.0f);
					p[1].y = nemo[i].y + (nemo[i].size / 2.0f);
					p[2].x = nemo[i].x - (nemo[i].size / 2.0f);
					p[2].y = nemo[i].y - (nemo[i].size / 2.0f);
					p[3].x = nemo[i].x + (nemo[i].size / 2.0f);
					p[3].y = nemo[i].y - (nemo[i].size / 2.0f);
					for(int j = 0 ; j <4;j++){
						p[j].r = nemo[i].r;
						p[j].g = nemo[i].g;
						p[j].b = nemo[i].b;
						p[j].size = nemo[i].size / 2.0f;
						particle.push_back(p[j]);
					}
				}
				else {
					NEMO p[8];
					p[0].x = nemo[i].x - (nemo[i].size / 2.0f);
					p[0].y = nemo[i].y + (nemo[i].size / 2.0f);
					p[1].x = nemo[i].x;
					p[1].y = nemo[i].y + (nemo[i].size / 2.0f);
					p[2].x = nemo[i].x + (nemo[i].size / 2.0f);
					p[2].y = nemo[i].y + (nemo[i].size / 2.0f);
					p[3].x = nemo[i].x - (nemo[i].size / 2.0f);
					p[3].y = nemo[i].y;
					p[4].x = nemo[i].x + (nemo[i].size / 2.0f);
					p[4].y = nemo[i].y;
					p[5].x = nemo[i].x - (nemo[i].size / 2.0f);
					p[5].y = nemo[i].y - (nemo[i].size / 2.0f);
					p[6].x = nemo[i].x;
					p[6].y = nemo[i].y - (nemo[i].size / 2.0f);
					p[7].x = nemo[i].x + (nemo[i].size / 2.0f);
					p[7].y = nemo[i].y - (nemo[i].size / 2.0f);
					
					for (int j = 0; j < 8;j++) {
						p[j].r = nemo[i].r;
						p[j].g = nemo[i].g;
						p[j].b = nemo[i].b;
						p[j].size = nemo[i].size / 2.0f;
						particle.push_back(p[j]);
					}
				}

				nemo.erase(nemo.begin() + i);
				break;
			}
		}
		
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		// ���콺 ������ ��ư�� ������ �� ó���� ����
	}
}

GLvoid Keyboard(unsigned char key, int x, int y) { //--- �ݹ� �Լ�: Ű���� �ݹ� �Լ� {
	if (key == '1') {
		mode = 1;
	}
	else if( key == '2') {
		mode = 2;
	}
	else if( key == '3') {
		mode = 3;
	}
	else if( key == '4') {
		mode = 4;
	}
}