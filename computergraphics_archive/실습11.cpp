#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>
#include <vector>
#include <cmath>
#define M_PI 3.1415926535

std::random_device rd;
std::mt19937 mt(rd());

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Keyboard(unsigned char key, int x, int y);
void InitBuffer();
GLvoid Timer(int value);
void makedots(float x, float y);

//--- 필요한 변수 선언
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

std::vector<GLfloat> dotver;
std::vector<GLfloat> linever;
std::vector<GLfloat> triver;
std::vector<GLfloat> rectver;
GLuint vao, vbo;

int maxspiral = 1;
int spiralState = 0; // 0:정지, 1:퍼져나가는 중, 2:돌아오는 중
float angle = 0.0f;
float radius = 0.0f;
float maxAngle = 0.0f;
float g_direction = 1.0f;
bool spotLine = false;
float r = 0.0f, g = 0.0f, b = 0.0f;

char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;

	fptr = fopen(file, "rb"); // Open file for reading
	if (!fptr) // Return NULL on failure
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file
	length = ftell(fptr); // Find out how many bytes into the file we are
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer
	fclose(fptr); // Close the file
	buf[length] = 0; // Null terminator
	return buf; // Return the buffer
}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	width = 500;
	height = 500;

	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Example1");

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();

	//--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
	make_vertexShaders(); //--- 버텍스 세이더 만들기
	make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
	shaderProgramID = make_shaderProgram();	//--- 세이더 프로그램 만들기

	// 버퍼(VAO/VBO/EBO) 초기화 (셰이더 프로그램 생성 후 호출)
	InitBuffer();

	glutDisplayFunc(drawScene); //--- 출력 콜백 함수
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(1000 / 120, Timer, 1); //--- 타이머 콜백함수 지정 (60 FPS)

	glutMainLoop();
}

void make_vertexShaders()
{
	GLchar* vertexSource;

	//--- 버텍스 세이더 읽어 저장하고 컴파일 하기
	//--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수

	vertexSource = filetobuf("vertex.glsl");
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

//--- 프래그먼트 세이더 객체 만들기
void make_fragmentShaders()
{
	GLchar* fragmentSource;

	//--- 프래그먼트 세이더 읽어 저장하고 컴파일하기
	fragmentSource = filetobuf("fragment.glsl"); // 프래그세이더 읽어오기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

GLuint make_shaderProgram()
{
	GLint result;
	GLchar* errorLog = NULL;
	GLuint shaderID;
	shaderID = glCreateProgram(); //--- 세이더 프로그램 만들기
	glAttachShader(shaderID, vertexShader); //--- 세이더 프로그램에 버텍스 세이더 붙이기
	glAttachShader(shaderID, fragmentShader); //--- 세이더 프로그램에 프래그먼트 세이더 붙이기
	glLinkProgram(shaderID); //--- 세이더 프로그램 링크하기
	glDeleteShader(vertexShader); //--- 세이더 객체를 세이더 프로그램에 링크했음으로, 세이더 객체 자체는 삭제 가능
	glDeleteShader(fragmentShader);
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // ---세이더가 잘 연결되었는지 체크하기
	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
		return false;
	}
	glUseProgram(shaderID); //--- 만들어진 세이더 프로그램 사용하기
	//--- 여러 개의 세이더프로그램 만들 수 있고, 그 중 한개의 프로그램을 사용하려면
	//--- glUseProgram 함수를 호출하여 사용 할 특정 프로그램을 지정한다.
	//--- 사용하기 직전에 호출할 수 있다.
	return shaderID;
}

std::vector<GLfloat> allVertices;
struct SHAPE {
	std::vector<GLfloat> vertex;
	float x, y;
	float centerX, centerY;
};
std::vector<SHAPE> shapes;

void UpdateBuffer()
{
	allVertices.clear();
	for (int i = 0; i < shapes.size(); i++)
		allVertices.insert(allVertices.end(), shapes[i].vertex.begin(), shapes[i].vertex.end());

	// 합쳐진 데이터로 VBO 업데이트
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(GLfloat), allVertices.data(), GL_DYNAMIC_DRAW);
}

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	glClearColor(r, g, b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgramID);
	glBindVertexArray(vao);

	GLint first = 0;
	for (int i = 0; i < shapes.size(); i++) {
		int vertexCount = shapes[i].vertex.size() / 6;
		
		glDrawArrays(GL_TRIANGLES, first, vertexCount);
		first += vertexCount;
	}

	glBindVertexArray(0);
	glutSwapBuffers();
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}

GLvoid Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		std::uniform_int_distribution<int> rdspiral(0,1);
		int rdstate = rdspiral(mt);
		if (rdstate == 0) spiralState = 1;
		else spiralState = 3;

		std::cout << spiralState << std::endl;

		std::uniform_real_distribution<float> rdcolor(0.0f, 1.0f);
		r = rdcolor(mt);
		g = rdcolor(mt);
		b = rdcolor(mt);

		float mx = (2.0f * x / width) - 1.0f;
		float my = 1.0f - (2.0f * y / height);
		
		shapes.clear();
		angle = 0.0f;
		radius = 0.0f;
		
		maxAngle = 1800.0f * (M_PI / 180.0f); // 720도
	
		makedots(mx, my);
		for (int i = 0;i < maxspiral-1;i++) {
			std::uniform_real_distribution<float> rdxy(-1.0f, 1.0f);
			float rx = rdxy(mt);
			float ry = rdxy(mt);
			makedots(rx, ry);
		}
		UpdateBuffer();
		glutPostRedisplay();
	}
}

void makedots(float x, float y)
{
	SHAPE newShape;
	newShape.x = x;
	newShape.y = y;
	newShape.centerX = x; 
	newShape.centerY = y;

	float size = 0.008f;
	newShape.vertex = {
		x - size, y + size, 0.0f, 1.0f, 1.0f, 1.0f,
		x - size, y - size, 0.0f, 1.0f, 1.0f, 1.0f,
		x + size, y + size, 0.0f, 1.0f, 1.0f, 1.0f,
		x - size, y - size, 0.0f, 1.0f, 1.0f, 1.0f,
		x + size, y - size, 0.0f, 1.0f, 1.0f, 1.0f,
		x + size, y + size, 0.0f, 1.0f, 1.0f, 1.0f
	};
	shapes.push_back(newShape);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'p':
		if(spiralState == 0) spotLine = false;
		break;
	case 'l':
		if (spiralState == 0) spotLine = true;
		break;
	case '1':
		maxspiral = 1;
		break;
	case '2':
		maxspiral = 2;
		break;
	case '3':
		maxspiral = 3;
		break;
	case '4':
		maxspiral = 4;
		break;
	case'5':
		maxspiral = 5;
		break;
	case 'c':
		shapes.clear();
		UpdateBuffer();
		spiralState = 0;
		break;
	case 'q':
		exit(0);
		break;
	}

	glutPostRedisplay(); // 다시 그리기 요청
}


GLvoid Timer(int value)
{
	static int dotTimer = 0;
	std::uniform_real_distribution<float> rdangle(-0.003f, 0.005f);

	if (spiralState == 1) {

		angle += 0.05f;
		radius += rdangle(mt);

		if (abs(angle) >= maxAngle) {
			spiralState = 2;
			angle = -900.0f * (M_PI / 180.0f);
			for (int i = 0; i < shapes.size(); i++) {
				shapes[i].centerX += radius*2;
			}
		}
	}
	else if (spiralState == 2) { 
		angle -= 0.05f;
		radius -= rdangle(mt);

		if (radius <= 0.0f) {
			radius = 0.0f;
			spiralState = 0; 
		}
	}
	else if (spiralState == 3) {
		angle -= 0.05f;
		radius += rdangle(mt);

		if (abs(angle) >= maxAngle) {
			spiralState = 4;
			angle = -900.0f * (M_PI / 180.0f);
			for (int i = 0; i < shapes.size(); i++) {
				shapes[i].centerX += radius * 2;
			}
		}
	}
	else if (spiralState == 4) {
		angle += 0.05f;
		radius -= rdangle(mt);

		if (radius <= 0.0f) {
			radius = 0.0f;
			spiralState = 0;
		}
	}
	
	if (spotLine == 0) {
		if (shapes.size() > 0) {
			dotTimer++;
			if (dotTimer % 8 == 0) {
				for (int i = 0; i < maxspiral; i++) {
					float px = shapes[i].centerX + radius * cos(angle);
					float py = shapes[i].centerY + radius * sin(angle);
					makedots(px, py);
				}
			}
		}
	}
	else {
		if (shapes.size() > 0) {
			for (int i = 0; i < maxspiral; i++) {
				float px = shapes[i].centerX + radius * cos(angle);
				float py = shapes[i].centerY + radius * sin(angle);
				makedots(px, py);
			}
		}
	}

	UpdateBuffer();
	glutPostRedisplay();
	glutTimerFunc(1000 / 120, Timer, 1);
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// 초기에는 빈 버퍼
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	// --- 위치 속성 (location = 0, vec3)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// --- 색상 속성 (location = 1, vec3)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}