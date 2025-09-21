#include <iostream>
#include <gl/glew.h> //--- 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <vector>
#include <random>

std::random_device rd;  //--- 랜덤 엔진
std::mt19937 mt(rd());
std::uniform_real_distribution<float> rdxy(-1.0f, 1.0f); // 0.0 ~ 1.0 사이의 균등 분포
std::uniform_real_distribution<float> rdcolor(0.0f, 1.0f);

#define SIZEW 800
#define SIZEH 800
#define TimerSpeed 60 //--- 타이머 속도 설정 (60fps)

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Timer(int value);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid MouseMove(int x, int y);
GLvoid Keyboard(unsigned char key, int x, int y);

struct NEMO{
	float x, y; // 위치
	float r, g, b; // 색상
};
std::vector<NEMO> nemo(40);

struct ERASER{
	float x, y; // 위치
	float r=0.0f, g=0.0f, b=0.0f;	// 색상
	float size = 0.04f;
}eraser;

float size = 0.02f;
bool mouseDown = false;

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

	for(int i = 0 ; i < nemo.size() ; i++){
		nemo[i].x = rdxy(mt);
		nemo[i].y = rdxy(mt);
		nemo[i].r = rdcolor(mt);
		nemo[i].g = rdcolor(mt);
		nemo[i].b = rdcolor(mt);
	}

	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
	glutTimerFunc(1000 / TimerSpeed, Timer, 1); //--- 타이머 콜백함수 지정 (60 FPS)
	glutMouseFunc(Mouse);
	glutMotionFunc(MouseMove);
	glutKeyboardFunc(Keyboard);
	glutMainLoop(); // 이벤트 처리 시작
}

GLvoid drawScene() { //--- 콜백 함수: 출력 콜백 함수 {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); // 설정된 색으로 전체를 칠하기
	
	for(int i = 0 ; i < nemo.size() ; i++){
		glColor3f(nemo[i].r, nemo[i].g, nemo[i].b);
		glRectf(nemo[i].x - size,nemo[i].y -size, nemo[i].x + size, nemo[i].y +size);
	}

	if(mouseDown){
		glColor3f(eraser.r, eraser.g, eraser.b);
		glRectf(eraser.x - eraser.size,eraser.y - eraser.size, eraser.x + eraser.size, eraser.y + eraser.size);;
	}


	glutSwapBuffers(); // 화면에 출력하기
}

GLvoid Reshape(int w, int h) { //--- 콜백 함수: 다시 그리기 콜백 함수 {
	glViewport(0, 0, w, h);
}

GLvoid Timer(int value) { //--- 콜백 함수: 타이머 콜백 함수

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
	glutPostRedisplay(); // 화면 갱신 요청
	glutTimerFunc(1000 / TimerSpeed, Timer, 1); // 타이머 재설정 (60 FPS)
}

GLvoid Mouse(int button, int state, int x, int y) { //--- 콜백 함수: 마우스 콜백 함수
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

GLvoid Keyboard(unsigned char key, int x, int y) { //--- 콜백 함수: 키보드 콜백 함수
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