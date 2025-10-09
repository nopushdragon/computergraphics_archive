#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>
#include <vector>

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_real_distribution<float> rdcolor(0.0f, 1.0f);
std::uniform_real_distribution<float> rdxy(-1.0f, 1.0f);
std::uniform_real_distribution<float> rdmovexy(-0.01f, 0.1f);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Timer(int value);
GLvoid MyMouseMove(int x, int y);
void InitBuffer();
void Moveshape(float x, float y);

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

std::vector<GLfloat> allVertices;

struct SHAPE {
	int shape; // 1: 점, 2: 선, 3: 삼각형, 4: 사각형 5: 오각형
	std::vector<GLfloat> vertex;
	float x, y;
	float size;

	int move = 0;
	float movex = rdmovexy(mt), movey = rdmovexy(mt);
};
std::vector<SHAPE> shapes;
SHAPE MakeShape(int shape, float x, float y, float r, float g, float b, float size);
void resetShape();

int nowshape = -1;

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
	resetShape();

	glutDisplayFunc(drawScene); //--- 출력 콜백 함수
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(1000 / 120, Timer, 1); //--- 타이머 콜백함수 지정 (60 FPS)
	glutMotionFunc(MyMouseMove); // 마우스 움직임 콜백함수 지정

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

void UpdateBuffer()
{
	allVertices.clear();

	for (int i = 0; i < shapes.size(); i++)	allVertices.insert(allVertices.end(), shapes[i].vertex.begin(), shapes[i].vertex.end());
	
	// 합쳐진 데이터로 VBO 업데이트
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(GLfloat), allVertices.data(), GL_DYNAMIC_DRAW);
}

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgramID);
	glBindVertexArray(vao);

	GLint first = 0;
	for (int i = 0; i < shapes.size(); i++) {
		int vertexCount = shapes[i].vertex.size() / 6;

		// 올바른 시작 위치(first)에서 그리기
		if (shapes[i].shape == 1) glDrawArrays(GL_TRIANGLES, first, vertexCount);
		else if (shapes[i].shape == 2) glDrawArrays(GL_LINES, first, vertexCount);
		else if (shapes[i].shape == 3) glDrawArrays(GL_TRIANGLES, first, vertexCount);
		else if (shapes[i].shape == 4) glDrawArrays(GL_TRIANGLES, first, vertexCount);
		else if (shapes[i].shape == 5) glDrawArrays(GL_TRIANGLES, first, vertexCount);

		// 다음 도형을 위해 시작 위치 업데이트
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
	float mx = (2.0f * x / width) - 1.0f;
	float my = 1.0f - (2.0f * y / height);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

		for (int i = 0; i < shapes.size(); i++) {
			float dx = shapes[i].x - mx;
			float dy = shapes[i].y - my;
			float dist = sqrt(dx * dx + dy * dy);

			if (dist < shapes[i].size/2.0f) {
				nowshape = i;
				break;
			}
		}
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		if (nowshape != -1) {
			for (int i = 0; i < shapes.size(); i++) {
				float dx = shapes[i].x - mx;
				float dy = shapes[i].y - my;
				float dist = sqrt(dx * dx + dy * dy);

				if (dist < shapes[i].size && i != nowshape) {
					int sumVer = shapes[i].shape + shapes[nowshape].shape;
					if (sumVer > 5) sumVer -= 5;

					shapes.push_back(MakeShape(sumVer, shapes[i].x, shapes[i].y, rdcolor(mt), rdcolor(mt), rdcolor(mt), 0.1f));
					shapes.back().move = 1;
					if (i > nowshape) {
						shapes.erase(shapes.begin() + i);
						shapes.erase(shapes.begin() + nowshape);
					}
					else {
						shapes.erase(shapes.begin() + nowshape);
						shapes.erase(shapes.begin() + i);
					}

					break;
				}
			}

			nowshape = -1;
		}
	}

	UpdateBuffer();
	glutPostRedisplay();
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'c':
		resetShape();
		UpdateBuffer();
		break;
	case 's':
		for(int i = 0 ; i < shapes.size() ; i++) {
			if (shapes[i].move == 1) shapes[i].move = 2;
			else if (shapes[i].move == 2) shapes[i].move = 1;
		}
		break;
	case 'q':
		exit(0);
		break;
	}

	glutPostRedisplay(); // 다시 그리기 요청
}

GLvoid MyMouseMove(int x, int y) 
{
	if (nowshape != -1) {
		float mx = (2.0f * x / width) - 1.0f;
		float my = 1.0f - (2.0f * y / height);

		float dx = mx - shapes[nowshape].x;
		float dy = my - shapes[nowshape].y;

		shapes[nowshape].x = mx;
		shapes[nowshape].y = my;

		for (int i = 0; i < shapes[nowshape].vertex.size(); i += 6) {
			shapes[nowshape].vertex[i] += dx;
			shapes[nowshape].vertex[i + 1] += dy;
		}

		UpdateBuffer();
		glutPostRedisplay();
	}
}

GLvoid Timer(int value) //--- 콜백 함수: 타이머 콜백 함수
{
	for (int i = 0; i < shapes.size();i++) {
		if (shapes[i].move == 0) continue;

		float oldx = shapes[i].x;
		float oldy = shapes[i].y;

		if (shapes[i].move == 1) {
			shapes[i].x += shapes[i].movex;
			shapes[i].y += shapes[i].movey;
			if (shapes[i].x > 1.0f || shapes[i].x < -1.0f) shapes[i].movex *= -1.0f;
			if (shapes[i].y > 1.0f || shapes[i].y < -1.0f) shapes[i].movey *= -1.0f;
		}

		float dx = shapes[i].x - oldx;
		float dy = shapes[i].y - oldy;
		for (int j = 0; j < shapes[i].vertex.size(); j += 6) {
			shapes[i].vertex[j] += dx;
			shapes[i].vertex[j + 1] += dy;
		}
	}
	UpdateBuffer();
	glutPostRedisplay(); // 다시 그리기 요청
	glutTimerFunc(1000 / 120, Timer, 1); //--- 타이머 콜백함수 지정 (60 FPS)
}

void Moveshape(float x, float y)
{
	shapes[nowshape].x += x;
	shapes[nowshape].y += y;

	if (shapes[nowshape].shape == 0) { // 점
		shapes[nowshape].vertex[0] += x;
		shapes[nowshape].vertex[1] += y;
		shapes[nowshape].vertex[6] += x;
		shapes[nowshape].vertex[7] += y;
		shapes[nowshape].vertex[12] += x;
		shapes[nowshape].vertex[13] += y;
		shapes[nowshape].vertex[18] += x;
		shapes[nowshape].vertex[19] += y;
		shapes[nowshape].vertex[24] += x;
		shapes[nowshape].vertex[25] += y;
		shapes[nowshape].vertex[30] += x;
		shapes[nowshape].vertex[31] += y;
	}
	else if (shapes[nowshape].shape == 1) { // 선
		shapes[nowshape].vertex[0] += x;
		shapes[nowshape].vertex[1] += y;
		shapes[nowshape].vertex[6] += x;
		shapes[nowshape].vertex[7] += y;
	}
	else if (shapes[nowshape].shape == 2) { // 삼각형
		shapes[nowshape].vertex[0] += x;
		shapes[nowshape].vertex[1] += y;
		shapes[nowshape].vertex[6] += x;
		shapes[nowshape].vertex[7] += y;
		shapes[nowshape].vertex[12] += x;
		shapes[nowshape].vertex[13] += y;
	}
	else if (shapes[nowshape].shape == 3) { // 사각형
		shapes[nowshape].vertex[0] += x;
		shapes[nowshape].vertex[1] += y;
		shapes[nowshape].vertex[6] += x;
		shapes[nowshape].vertex[7] += y;
		shapes[nowshape].vertex[12] += x;
		shapes[nowshape].vertex[13] += y;
		shapes[nowshape].vertex[18] += x;
		shapes[nowshape].vertex[19] += y;
		shapes[nowshape].vertex[24] += x;
		shapes[nowshape].vertex[25] += y;
		shapes[nowshape].vertex[30] += x;
		shapes[nowshape].vertex[31] += y;
	}

	UpdateBuffer();
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

SHAPE MakeShape(int shape, float x, float y,float r,float g, float b, float size)
{
	SHAPE newShape;
	newShape.shape = shape;
	newShape.x = x;
	newShape.y = y;
	newShape.size = size;

	if (shape == 1) {	//점
		float minus_size = (size / 10.0f) * 8.0f;

		newShape.vertex.insert(newShape.vertex.end(), { x - (size - minus_size), y + (size - minus_size), 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x - (size - minus_size), y - (size - minus_size), 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x + (size - minus_size), y + (size - minus_size), 0.0f, r, g, b });
		//2번쨰 삼각형	
		newShape.vertex.insert(newShape.vertex.end(), { x - (size - minus_size), y - (size - minus_size), 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x + (size - minus_size), y - (size - minus_size), 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x + (size - minus_size), y + (size - minus_size), 0.0f, r, g, b });
	}
	else if (shape == 2) {
		// 클릭 지점 중심으로 수평선 추가
		newShape.vertex.insert(newShape.vertex.end(), { x - size, y, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x + size, y, 0.0f, r, g, b });
	}
	else if (shape == 3) {
		// 클릭 지점 중심으로 삼각형 3개 정점 추가
		newShape.vertex.insert(newShape.vertex.end(), { x, y + size, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x - size, y - size, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x + size, y - size, 0.0f, r, g, b });
	}
	else if (shape == 4) {
		newShape.vertex.insert(newShape.vertex.end(), { x - size, y + size, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x - size, y - size, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x + size, y + size, 0.0f, r, g, b });
		//2번쨰 삼각형	
		newShape.vertex.insert(newShape.vertex.end(), { x - size, y - size, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x + size, y - size, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x + size, y + size, 0.0f, r, g, b });
	}
	else if (shape == 5) {
		newShape.vertex.insert(newShape.vertex.end(), { x - (size), y, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x - (size / 2.0f), y - (size), 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x + (size / 2.0f), y - (size), 0.0f, r, g, b });
		//2번쨰 삼각형
		newShape.vertex.insert(newShape.vertex.end(), { x - (size), y, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x + (size / 2.0f), y - (size), 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x + (size), y, 0.0f, r, g, b });
		//3번쨰 삼각형
		newShape.vertex.insert(newShape.vertex.end(), { x - (size), y, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x + (size), y, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x, y + (size), 0.0f, r, g, b });

	}
	return newShape;
}

void resetShape()
{
	shapes.clear();

	for (int i = 1; i < 6;i++) {
		for (int j = 0; j < 3;j++){
			float rx = rdxy(mt);
			float rt = rdxy(mt);
			float r = rdcolor(mt);
			float g = rdcolor(mt);
			float b = rdcolor(mt);

			shapes.push_back(MakeShape(i, rx, rt, r, g, b, 0.1f));
		}
	}

	UpdateBuffer();
}
