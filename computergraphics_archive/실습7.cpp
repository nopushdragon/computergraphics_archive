#include <iostream>
#include <gl/glew.h> //--- 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_real_distribution<float> rdx(0.5f, 1.0f);
std::uniform_real_distribution<float> rdy(-1.0f, 1.0f);
std::uniform_real_distribution<float> rdcolor(0.0f, 1.0f);

#define SIZEW 900
#define SIZEH 900
#define TimerSpeed 60 //--- 타이머 속도 설정 (60 FPS)

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Timer(int value);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid MouseMove(int x, int y);
GLvoid Keyboard(unsigned char key, int x, int y);

void init();

struct NEMO {
	float x, y;
	float r, g, b;
	float sizex, sizey;
};
NEMO board[10];
NEMO nemo[10];

int NowNemo = -1;
bool cheat = false;

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // 디스플레이 모드 설정
	glutInitWindowPosition(50, 50); // 윈도우의 위치 지정
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

	init();

	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
	glutTimerFunc(1000 / TimerSpeed, Timer, 1); //--- 타이머 콜백함수 지정 (60 FPS)
	glutMouseFunc(Mouse);
	glutMotionFunc(MouseMove);
	glutKeyboardFunc(Keyboard);
	glutMainLoop(); // 이벤트 처리 시작
}

GLvoid drawScene() { //--- 콜백 함수: 출력 콜백 함수 {
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); // 설정된 색으로 전체를 칠하기

	glColor3f(0.0f, 0.0f, 0.0f);
	glRectf(0.5f, -1.0f, 0.51f, 1.0f);

	for (int i = 0; i < 10;i++) {
		glColor3f(board[i].r, board[i].g, board[i].b);
		glRectf(board[i].x - board[i].sizex, board[i].y - board[i].sizey, board[i].x + board[i].sizex, board[i].y + board[i].sizey);
	}
	for (int i = 0;i < 10;i++) {
		glColor3f(nemo[i].r, nemo[i].g, nemo[i].b);
		glRectf(nemo[i].x - nemo[i].sizex, nemo[i].y - nemo[i].sizey, nemo[i].x + nemo[i].sizex, nemo[i].y + nemo[i].sizey);
	}
	glutSwapBuffers(); // 화면에 출력하기
}

GLvoid Reshape(int w, int h) { //--- 콜백 함수: 다시 그리기 콜백 함수 {
	glViewport(0, 0, w, h);
}

GLvoid Timer(int value) { //--- 콜백 함수: 타이머 콜백 함수 {
	if(cheat) {
		for (int i = 0;i < 10;i++) {
			bool arrive = false;
			while (!arrive) {
				if (nemo[i].x > board[i].x - 0.01f && nemo[i].x < board[i].x + 0.01f &&
					nemo[i].y > board[i].y - 0.01f && nemo[i].y < board[i].y + 0.01f) {
					arrive = true;
				}
				else {
					if (nemo[i].x < board[i].x) {
						nemo[i].x += 0.007f;
					}
					else if (nemo[i].x > board[i].x) {
						nemo[i].x -= 0.007f;
					}
					if (nemo[i].y < board[i].y) {
						nemo[i].y += 0.007f;
					}
					else if (nemo[i].y > board[i].y) {
						nemo[i].y -= 0.007f;
					}
					break;
				}
			}
			if (arrive)continue;
			else break;
		}
	}

	glutPostRedisplay(); // 다시 그리기
	glutTimerFunc(1000 / TimerSpeed, Timer, 1); // 타이머 재설정 (60 FPS)
}

GLvoid Mouse(int button, int state, int x, int y) { //--- 콜백 함수: 마우스 콜백 함수 {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

		float mx = (float)(x - SIZEW / 2) / (SIZEW / 2);
		float my = (float)(SIZEH / 2 - y) / (SIZEH / 2);

		for (int i = 0;i < 10;i++) {
			if (mx >= nemo[i].x - nemo[i].sizex && mx <= nemo[i].x + nemo[i].sizex &&
				my >= nemo[i].y - nemo[i].sizey && my <= nemo[i].y + nemo[i].sizey) {
				NowNemo = i;
			}
		}
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		if (NowNemo != -1) {
			if (nemo[NowNemo].x > board[NowNemo].x - 0.02f && nemo[NowNemo].x < board[NowNemo].x + 0.02f &&
				nemo[NowNemo].y > board[NowNemo].y - 0.02f && nemo[NowNemo].y < board[NowNemo].y + 0.02f) {
				std::cout << "Success: " << NowNemo + 1 << "번" << std::endl;
			}
			NowNemo = -1;
		}
	}
}

GLvoid MouseMove(int x, int y) { //--- 콜백 함수: 마우스 이동 콜백 함수 {

	for (int i = 0;i < 10;i++) {
		if (NowNemo == i) {
			float mx = (float)(x - SIZEW / 2) / (SIZEW / 2);
			float my = (float)(SIZEH / 2 - y) / (SIZEH / 2);
			nemo[i].x = mx;
			nemo[i].y = my;
		}
	}
}

GLvoid Keyboard(unsigned char key, int x, int y) { //--- 콜백 함수: 키보드 콜백 함수 {
	if (key == 'r') {
		for (int i = 0; i < 10;i++) {
			nemo[i].r = rdcolor(mt);
			nemo[i].g = rdcolor(mt);
			nemo[i].b = rdcolor(mt);
			nemo[i].x = rdx(mt);
			nemo[i].y = rdy(mt);
		}
	}
	else if( key == 'q') {
		exit(0);
	}
	else if( key == 'a') {
		cheat = true;
	}
}

void init() {
	for (int i = 0;i < 10;i++) {
		if (i == 0) {
			board[i].x = -0.9f;
			board[i].y = 0.9f;
			board[i].sizex = 0.05f;
			board[i].sizey = 0.08f;
		}
		else if (i == 1) {
			board[i].x = -0.7f;
			board[i].y = 0.9f;
			board[i].sizex = 0.08f;
			board[i].sizey = 0.08f;
		}
		else if (i == 2) {
			board[i].x = -0.9f;
			board[i].y = 0.7f;
			board[i].sizex = 0.08f;
			board[i].sizey = 0.12f;
		}
		else if (i == 3) {
			board[i].x = -0.5f;
			board[i].y = 0.7f;
			board[i].sizex = 0.2f;
			board[i].sizey = 0.12f;
		}
		else if (i == 4) {
			board[i].x = -0.5f;
			board[i].y = 0.4f;
			board[i].sizex = 0.4f;
			board[i].sizey = 0.18f;
		}
		else if (i == 5) {
			board[i].x = -0.5f;
			board[i].y = 0.0f;
			board[i].sizex = 0.3f;
			board[i].sizey = 0.22f;
		}
		else if (i == 6) {
			board[i].x = -0.5f;
			board[i].y = -0.4f;
			board[i].sizex = 0.2f;
			board[i].sizey = 0.18f;
		}
		else if (i == 7) {
			board[i].x = -0.5f;
			board[i].y = -0.75f;
			board[i].sizex = 0.1f;
			board[i].sizey = 0.17f;
		}
		else if (i == 8) {
			board[i].x = 0.0f;
			board[i].y = -0.84f;
			board[i].sizex = 0.4f;
			board[i].sizey = 0.08f;
		}
		else if (i == 9) {
			board[i].x = 0.3f;
			board[i].y = -0.5f;
			board[i].sizex = 0.1f;
			board[i].sizey = 0.26f;
		}

		board[i].r = 0.0f;
		board[i].g = 0.0f;
		board[i].b = 0.0f;

		nemo[i] = board[i];
		nemo[i].r = rdcolor(mt);
		nemo[i].g = rdcolor(mt);
		nemo[i].b = rdcolor(mt);
		nemo[i].x = rdx(mt);
		nemo[i].y = rdy(mt);
	}
}