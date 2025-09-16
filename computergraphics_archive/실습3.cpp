#include <iostream>
#include <gl/glew.h> //--- 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>
#include <vector>

#define SIZEW 800
#define SIZEH 800
#define TimerSpeed 60

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_real_distribution<float> randcolor(0.0f, 1.0f);
std::uniform_real_distribution<float> randf(-1.0f, 1.0f);

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Timer(int value);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid MouseMove(int x, int y);
GLvoid StopMove(int x, int y);
GLvoid Keyboard(unsigned char key, int x, int y);

void init();

struct NEMO{
	float x1, y1, x2, y2;
	float r, g, b;
};
std::vector <NEMO> nemo;

bool isMouse = false;
bool isAdd = false;
int MoveNemo = -1;
int AddNemo = -1;
float startX, startY;
float rr = randcolor(rd);
float rg = randcolor(rd);
float rb = randcolor(rd);

float leftx,rightx,topy,bottomy;

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // 디스플레이 모드 설정
	glutInitWindowPosition(100, 100); // 윈도우의 위치 지정
	glutInitWindowSize(SIZEW, SIZEH); // 윈도우의 크기 지정
	glutCreateWindow("Example1"); // 윈도우 생성

	init();

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) { // glew 초기화 
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
	glutTimerFunc(1000 / TimerSpeed, Timer, 1); //--- 타이머 콜백함수 지정 (60 FPS)
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(MouseMove);
	glutPassiveMotionFunc(StopMove);
	glutMainLoop(); // 이벤트 처리 시작
}

GLvoid drawScene() { //--- 콜백 함수: 출력 콜백 함수 {
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f); // 바탕색을 ‘blue’로 지정
	glClear(GL_COLOR_BUFFER_BIT); // 설정된 색으로 전체를 칠하기
	// 그리기 부분 구현: 그리기 관련 부분이 여기에 포함된다
	
	for (int i = 0; i < nemo.size(); i++)
	{
		glColor3f(nemo[i].r, nemo[i].g, nemo[i].b);
		glRectf(nemo[i].x1, nemo[i].y1, nemo[i].x2, nemo[i].y2);
	}

	if (isMouse) {
		for (int i = 0; i < nemo.size(); i++)
		{
			if (i == AddNemo) {
				if (nemo[AddNemo].x1 < nemo[MoveNemo].x1) leftx = nemo[AddNemo].x1;
				else leftx = nemo[MoveNemo].x1;
				if (nemo[AddNemo].x2 > nemo[MoveNemo].x2) rightx = nemo[AddNemo].x2;
				else rightx = nemo[MoveNemo].x2;
				if (nemo[AddNemo].y1 > nemo[MoveNemo].y1) topy = nemo[AddNemo].y1;
				else topy = nemo[MoveNemo].y1;
				if (nemo[AddNemo].y2 < nemo[MoveNemo].y2) bottomy = nemo[AddNemo].y2;
				else bottomy = nemo[MoveNemo].y2;
				

				glColor3f(rr, rg, rb);
				glRectf(leftx, topy, rightx, bottomy);
			}
		}
	}

	glutSwapBuffers(); // 화면에 출력하기
}

GLvoid Reshape(int w, int h) { //--- 콜백 함수: 다시 그리기 콜백 함수 {
	glViewport(0, 0, w, h);
}

GLvoid Timer(int value) {
	while (nemo.size() > 30) {
		nemo.pop_back();
	}

	glutTimerFunc(1000 / TimerSpeed, Timer, 1); //--- 타이머 콜백함수 지정 (60 FPS)
	glutPostRedisplay();
}

GLvoid Mouse(int button, int state, int x, int y) {
	float mx = (float)x / (SIZEW / 2) - 1.0f;
	float my = (SIZEH - (float)y) / (SIZEH / 2) - 1.0f;

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		for (int i = 0; i < nemo.size(); i++)
		{
			if (mx > nemo[i].x1 && mx < nemo[i].x2 && my < nemo[i].y1 && my > nemo[i].y2)
			{
				isMouse = true;
				MoveNemo = i;
				startX = mx;
				startY = my;
			}
		}
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		for (int i = 0; i < nemo.size(); i++)
		{
			if (mx > nemo[i].x1 && mx < nemo[i].x2 && my < nemo[i].y1 && my > nemo[i].y2)
			{
				std::uniform_real_distribution<float> distroyrd(0.05f, 0.1f);

				nemo.erase(nemo.begin() + i);
				for (int j = 0; j < 2;j++) {
					float distroysize = distroyrd(rd);
					float rx = randf(rd);
					float ry = randf(rd);
					nemo.push_back({ rx, ry, rx + distroysize, ry - distroysize, randcolor(rd), randcolor(rd), randcolor(rd) });
				}
			}
		}
	}
}

GLvoid MouseMove(int x, int y) {
	if (isMouse) {

		float mx = (float)x / (SIZEW / 2) - 1.0f;
		float my = (SIZEH - (float)y) / (SIZEH / 2) - 1.0f;
		isAdd = false;

		for (int i = 0; i < nemo.size(); i++)
		{
			if (i == MoveNemo) {
				nemo[i].x1 += mx - startX;
				nemo[i].x2 += mx - startX;
				nemo[i].y1 += my - startY;
				nemo[i].y2 += my - startY;
				startX = mx;
				startY = my;

			}
			if (mx > nemo[i].x1 && mx < nemo[i].x2 && my < nemo[i].y1 && my > nemo[i].y2 && i != MoveNemo)
			{
				AddNemo = i;
				isAdd = true;
			}
		}
		if (!isAdd) {
			rr = randcolor(rd);
			rg = randcolor(rd);
			rb = randcolor(rd);
			AddNemo = -1;
		}
		
	}
}

GLvoid StopMove(int x, int y) {
	if (isAdd) {

		nemo.push_back({ leftx, topy, rightx, bottomy, rr, rg, rb });

		for(int i = nemo.size()-1;i >= 0;i--){
			if (i == MoveNemo || i == AddNemo)
				nemo.erase(nemo.begin() + i);
		}
	}
	isAdd = false;
	isMouse = false;
	MoveNemo = -1;
	AddNemo = -1;
}

GLvoid Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'a':
		float rx = randf(rd);
		float ry = randf(rd);
		nemo.push_back({ rx,ry, rx + 0.1f, ry - 0.1f, randcolor(rd), randcolor(rd), randcolor(rd) });
		break;
	}

}

void init() {
}
