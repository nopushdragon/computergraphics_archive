#include <iostream>
#include <gl/glew.h> //--- 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>
#include <vector>

#define SIZEW 800
#define SIZEH 800
#define TimerSpeed 60 //--- 타이머 속도 설정 (60 FPS)

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
	float x, y; // 위치
	float r, g, b; // 색상
	float size; // 크기
};
std::vector<NEMO> nemo;
std::vector<NEMO> particle;

int mode = 1;

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // 디스플레이 모드 설정
	glutInitWindowPosition(100, 100); // 윈도우의 위치 지정
	glutInitWindowSize(SIZEW, SIZEH); // 윈도우의 크기 지정
	glutCreateWindow("Example1"); // 윈도우 생성

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) { // glew 초기화 
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

	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
	glutTimerFunc(1000 / TimerSpeed, Timer, 1); //--- 타이머 콜백함수 지정 (60 FPS)
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutMainLoop(); // 이벤트 처리 시작
}

GLvoid drawScene() { //--- 콜백 함수: 출력 콜백 함수 {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // 바탕색을 ‘blue’로 지정
	glClear(GL_COLOR_BUFFER_BIT); // 설정된 색으로 전체를 칠하기
	
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


	glutSwapBuffers(); // 화면에 출력하기
}

GLvoid Reshape(int w, int h) { //--- 콜백 함수: 다시 그리기 콜백 함수 {
	glViewport(0, 0, w, h);
}

GLvoid Timer(int value) { //--- 콜백 함수: 타이머 콜백 함수 {
	
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

	glutPostRedisplay(); // 다시 그리기
	glutTimerFunc(1000 / TimerSpeed, Timer, 1); // 타이머 재설정 (60 FPS)
}

GLvoid Mouse(int button, int state, int x, int y) { //--- 콜백 함수: 마우스 콜백 함수 {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

		// 마우스 왼쪽 버튼이 눌렸을 때 처리할 내용
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
		// 마우스 오른쪽 버튼이 눌렸을 때 처리할 내용
	}
}

GLvoid Keyboard(unsigned char key, int x, int y) { //--- 콜백 함수: 키보드 콜백 함수 {
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