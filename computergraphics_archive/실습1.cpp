#include <iostream>
#include <gl/glew.h> //--- 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid timer_randcolor(int value);

float red = 1.0f, green = 1.0f, blue = 1.0f; //--- 배경색을 지정하는 전역변수
bool istimer = false;
std::random_device rd;
std::mt19937 mt(rd());
std::uniform_real_distribution<float> randcolor(0.0f, 1.0f);

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // 디스플레이 모드 설정
	glutInitWindowPosition(100, 100); // 윈도우의 위치 지정
	glutInitWindowSize(800, 600); // 윈도우의 크기 지정
	glutCreateWindow("Example1"); // 윈도우 생성

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) // glew 초기화 
	{ 
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
	glutKeyboardFunc(Keyboard); //--- 키보드 입력 콜백함수 지정
	glutMainLoop(); // 이벤트 처리 시작
}

GLvoid drawScene() { //--- 콜백 함수: 출력 콜백 함수 {
	glClearColor(red, green, blue, 1.0f); // 바탕색을 ‘blue’로 지정
	glClear(GL_COLOR_BUFFER_BIT); // 설정된 색으로 전체를 칠하기
	glutSwapBuffers(); // 화면에 출력하기
}

GLvoid Reshape(int w, int h) { //--- 콜백 함수: 다시 그리기 콜백 함수 {
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'c':	//--- 배경색을 청록색으로 설정
		red = 0;
		green = 1.0;
		blue = 1.0; 
		break; 
	case 'm':	//--- 배경색을 자홍색으로 설정
		red = 1.0;
		green = 0.0;
		blue = 1.0;
		break; 
	case 'y':;	//--- 배경색을 노랑색으로 설정
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
		glutTimerFunc(1000 / 60, timer_randcolor, 1); //--- 타이머 콜백함수 지정 (60 FPS)
		istimer = true;
		break;
	case 's':
		istimer = false;
		break;
	case 'q':
		exit(0); //--- 'q'키를 누르면 프로그램 종료
		break;
	}
	glutPostRedisplay(); //--- 배경색이 바뀔 때마다 출력 콜백 함수를 호출하여 화면을 refresh 한다
}

GLvoid timer_randcolor(int value) { //--- 콜백 함수: 타이머 콜백 함수
	if (istimer) {
		red = randcolor(rd);
		green = randcolor(rd);
		blue = randcolor(rd);
		glutTimerFunc(1000 / 60, timer_randcolor, 1); //--- 타이머 콜백함수 지정 (60 FPS)
	}
	glutPostRedisplay(); // 화면 갱신 요청
}