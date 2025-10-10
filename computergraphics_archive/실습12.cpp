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
float rr = rdcolor(mt);
float rg = rdcolor(mt);
float rb = rdcolor(mt);

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
	int shape; // 0:선 1:삼각형, 2:사각형 3:오각형
	std::vector<GLfloat> vertex;
	float r, g, b;
};
std::vector<SHAPE> shapes;
SHAPE make_shape(int shape, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float x5, float y5, float r, float g, float b);
void reset_shape( int shape, float x, float y, float size);

bool preview = false;

struct DOT {
	float x, y;
};
struct DOTS {
	DOT dot[4][5];
};
std::vector<DOTS>dots;
int timercnt = 0;

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
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		// GLEW 초기화 실패!
		fprintf(stderr, "GLEW 초기화 실패: %s\n", glewGetErrorString(err));
		system("pause"); // 에러 메시지 확인을 위해 잠시 멈춤
		exit(1);
	}
	printf("GLEW 초기화 성공!\n"); // 성공했는지 확인하기 위한 메시지
	glewInit();

	//--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
	make_vertexShaders(); //--- 버텍스 세이더 만들기
	make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
	shaderProgramID = make_shaderProgram();	//--- 세이더 프로그램 만들기

	// 버퍼(VAO/VBO/EBO) 초기화 (셰이더 프로그램 생성 후 호출)
	InitBuffer();
	reset_shape(0,-0.5f,0.5f,0.3f);
	reset_shape(1, 0.5f, 0.5f, 0.3f);
	reset_shape(2, -0.5f, -0.5f, 0.3f);
	reset_shape(3, 0.5f, -0.5f, 0.3f);

	glutDisplayFunc(drawScene); //--- 출력 콜백 함수
	glutReshapeFunc(Reshape);
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
	if (vertexSource == NULL) {
		std::cerr << "FATAL ERROR: vertex.glsl 파일을 현재 실행 폴더에서 찾을 수 없습니다!" << std::endl;
		system("pause"); // 결과 확인을 위해 잠시 멈춤
		exit(1);
	}
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	free(vertexSource);

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
	free(fragmentSource);

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
	GLchar errorLog[512];
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
		return 0;
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
		glDrawArrays(GL_TRIANGLES, first, vertexCount);

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

GLvoid Keyboard(unsigned char key, int x, int y)
{
	rr = rdcolor(mt);
	rg = rdcolor(mt);
	rb = rdcolor(mt);

	switch (key) {
	case 'l':
		if (preview == false) {
			preview = true;
			dots.clear();
			shapes.clear();
			reset_shape(1, 0.0f, 0.0f, 0.8f);
			timercnt = 0;
		}
		break;
	case 't':
		if (preview == false) {
			preview = true;
			dots.clear();
			shapes.clear();
			reset_shape(2, 0.0f, 0.0f, 0.8f);
			timercnt = 0;
		}
		break;
	case 'r':
		if (preview == false) {
			preview = true;
			dots.clear();
			shapes.clear();
			reset_shape(3, 0.0f, 0.0f, 0.8f);
			timercnt = 0;
		}
		break;
	case 'p':
		if (preview == false) {
			preview = true;
			dots.clear();
			shapes.clear();
			reset_shape(0, 0.0f, 0.0f, 0.8f);
			timercnt = 0;
		}
		break;
	case 'a':
		dots.clear();
		shapes.clear();
		reset_shape(0, -0.5f, 0.5f, 0.3f);
		reset_shape(1, 0.5f, 0.5f, 0.3f);
		reset_shape(2, -0.5f, -0.5f, 0.3f);
		reset_shape(3, 0.5f, -0.5f, 0.3f);
		break;
	case 'q':
		exit(0);
		break;
	}

	glutPostRedisplay(); // 다시 그리기 요청
}

GLvoid Timer(int value) //--- 콜백 함수: 타이머 콜백 함수
{
	bool lineEnd = true, triEnd = true, rectEnd = true, pentaEnd=true;
	bool End = true;
	
	if (preview == false) {
		for (int i = 0;i < shapes.size();i++)
		{
			if (shapes[i].shape == 0) { //오각-선
				if (dots[i].dot[0][2].y > dots[i].dot[0][0].y + 0.005f) {
					dots[i].dot[0][2].y -= 0.001f;
					lineEnd = false;
				}
				if (dots[i].dot[0][3].y > dots[i].dot[0][0].y + 0.005f) {
					dots[i].dot[0][3].y -= 0.001f;
					lineEnd = false;
				}
				if (dots[i].dot[0][4].y > dots[i].dot[0][0].y + 0.005f) {
					dots[i].dot[0][4].y -= 0.001f;
					lineEnd = false;
				}
			}
			else if (shapes[i].shape == 1) { // 선-삼각
				if (dots[i].dot[1][2].y < dots[i].dot[1][0].y + 0.6f) {
					dots[i].dot[1][2].y += 0.001f;
					triEnd = true;
				}
			}
			else if (shapes[i].shape == 2) { // 삼각-사각
				if (dots[i].dot[2][3].x > dots[i].dot[2][0].x) {
					dots[i].dot[2][3].x -= 0.001f;
					rectEnd = true;
				}
				if (dots[i].dot[2][2].x < dots[i].dot[2][1].x) {
					dots[i].dot[2][2].x += 0.001f;
					rectEnd = true;
				}
			}
			else if (shapes[i].shape == 3) { // 사각-오각
				if (dots[i].dot[3][0].x < dots[i].dot[3][4].x + 0.15f) {
					dots[i].dot[3][0].x += 0.001f;
					pentaEnd = true;
				}
				if (dots[i].dot[3][1].x > dots[i].dot[3][2].x - 0.15f) {
					dots[i].dot[3][1].x -= 0.001f;
					pentaEnd = true;
				}
				if (dots[i].dot[3][2].y > dots[i].dot[3][0].y + 0.3f) {
					dots[i].dot[3][2].y -= 0.001f;
					pentaEnd = true;
				}
				if (dots[i].dot[3][3].y < dots[i].dot[3][0].y + 0.6f) {
					dots[i].dot[3][3].y += 0.001f;
					pentaEnd = true;
				}
				if (dots[i].dot[3][4].y > dots[i].dot[3][0].y + 0.3f) {
					dots[i].dot[3][4].y -= 0.001f;
					pentaEnd = true;
				}
			}

			SHAPE updatedShape = make_shape(shapes[i].shape,
				dots[i].dot[shapes[i].shape][0].x, dots[i].dot[shapes[i].shape][0].y,
				dots[i].dot[shapes[i].shape][1].x, dots[i].dot[shapes[i].shape][1].y,
				dots[i].dot[shapes[i].shape][2].x, dots[i].dot[shapes[i].shape][2].y,
				dots[i].dot[shapes[i].shape][3].x, dots[i].dot[shapes[i].shape][3].y,
				dots[i].dot[shapes[i].shape][4].x, dots[i].dot[shapes[i].shape][4].y,
				shapes[i].r, shapes[i].g, shapes[i].b);

			shapes[i].vertex = updatedShape.vertex;
		}

		if (lineEnd && triEnd && rectEnd && pentaEnd) {
			int nowshape[4];
			for (int i = 0;i < shapes.size();i++) nowshape[i] = (shapes[i].shape + 1) % 4;
			dots.clear();
			shapes.clear();
			reset_shape(nowshape[0], -0.5f, 0.5f, 0.3f);
			reset_shape(nowshape[1], 0.5f, 0.5f, 0.3f);
			reset_shape(nowshape[2], -0.5f, -0.5f, 0.3f);
			reset_shape(nowshape[3], 0.5f, -0.5f, 0.3f);
		}
	}
	else {
		for (int i = 0;i < shapes.size();i++)
		{
			if (shapes[i].shape == 0) { //오각-선
				if (dots[i].dot[0][2].y > dots[i].dot[0][0].y + 0.005f) {
					dots[i].dot[0][2].y -= 0.01f;
					if (dots[i].dot[0][2].y < dots[i].dot[0][0].y + 0.005f) dots[i].dot[0][2].y = dots[i].dot[0][0].y + 0.005f;
					End = false;
				}
				if (dots[i].dot[0][3].y > dots[i].dot[0][0].y + 0.005f) {
					dots[i].dot[0][3].y -= 0.01f;
					if (dots[i].dot[0][3].y < dots[i].dot[0][0].y + 0.005f) dots[i].dot[0][3].y = dots[i].dot[0][0].y + 0.005f;
					End = false;
				}
				if (dots[i].dot[0][4].y > dots[i].dot[0][0].y + 0.005f) {
					dots[i].dot[0][4].y -= 0.01f;
					if (dots[i].dot[0][4].y < dots[i].dot[0][0].y + 0.005f) dots[i].dot[0][4].y = dots[i].dot[0][0].y + 0.005f;
					End = false;
				}
			}
			else if (shapes[i].shape == 1) { // 선-삼각
				if (dots[i].dot[1][2].y < dots[i].dot[1][0].y + 1.6f) {
					dots[i].dot[1][2].y += 0.01f;
					End = false;
				}
			}
			else if (shapes[i].shape == 2) { // 삼각-사각
				if (dots[i].dot[2][3].x > dots[i].dot[2][0].x) {
					dots[i].dot[2][3].x -= 0.01f;
					End = false;
				}
				if (dots[i].dot[2][2].x < dots[i].dot[2][1].x) {
					dots[i].dot[2][2].x += 0.01f;
					End = false;
				}
			}
			else if (shapes[i].shape == 3) { // 사각-오각
				if (dots[i].dot[3][0].x < dots[i].dot[3][4].x + 0.4f) {
					dots[i].dot[3][0].x += 0.01f;
					End = false;
				}
				if (dots[i].dot[3][1].x > dots[i].dot[3][2].x - 0.4f) {
					dots[i].dot[3][1].x -= 0.01f;
					End = false;
				}
				if (dots[i].dot[3][2].y > dots[i].dot[3][0].y + 0.8f) {
					dots[i].dot[3][2].y -= 0.01f;
					End = false;
				}
				if (dots[i].dot[3][3].y < dots[i].dot[3][0].y + 1.6f) {
					dots[i].dot[3][3].y += 0.01f;
					End = false;
				}
				if (dots[i].dot[3][4].y > dots[i].dot[3][0].y + 0.8f) {
					dots[i].dot[3][4].y -= 0.01f;
					End = false;
				}
			}

			SHAPE updatedShape = make_shape(shapes[i].shape,
				dots[i].dot[shapes[i].shape][0].x, dots[i].dot[shapes[i].shape][0].y,
				dots[i].dot[shapes[i].shape][1].x, dots[i].dot[shapes[i].shape][1].y,
				dots[i].dot[shapes[i].shape][2].x, dots[i].dot[shapes[i].shape][2].y,
				dots[i].dot[shapes[i].shape][3].x, dots[i].dot[shapes[i].shape][3].y,
				dots[i].dot[shapes[i].shape][4].x, dots[i].dot[shapes[i].shape][4].y,
				shapes[i].r, shapes[i].g, shapes[i].b);

			shapes[i].vertex = updatedShape.vertex;
		}

		if (End) {
			timercnt++;
			if (timercnt == 60) {
				preview = false;
				dots.clear();
				shapes.clear();
				reset_shape(0, -0.5f, 0.5f, 0.3f);
				reset_shape(1, 0.5f, 0.5f, 0.3f);
				reset_shape(2, -0.5f, -0.5f, 0.3f);
				reset_shape(3, 0.5f, -0.5f, 0.3f);
			}
		}
	}

	UpdateBuffer();
	glutPostRedisplay(); // 다시 그리기 요청
	glutTimerFunc(1000 / 120, Timer, 1); //--- 타이머 콜백함수 지정 (60 FPS)
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

SHAPE make_shape( int shape,float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float x5, float y5, float r, float g, float b)
{
	SHAPE newShape;
	newShape.shape = shape;
	newShape.r = r;
	newShape.g = g;
	newShape.b = b;
	if (shape == 0) { 
		newShape.vertex.insert(newShape.vertex.end(), { x5, y5, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x1, y1, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x2, y2, 0.0f, r, g, b });

		newShape.vertex.insert(newShape.vertex.end(), { x5, y5, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x2, y2, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x3, y3, 0.0f, r, g, b });

		newShape.vertex.insert(newShape.vertex.end(), { x5, y5, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x3, y3, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x4, y4, 0.0f, r, g, b });
	}
	else if (shape == 1) {
		newShape.vertex.insert(newShape.vertex.end(), { x1, y1, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x2, y2, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x3, y3, 0.0f, r, g, b });
	}
	else if (shape == 2) {
		newShape.vertex.insert(newShape.vertex.end(), { x4, y4, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x1, y1, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x2, y2, 0.0f, r, g, b });

		newShape.vertex.insert(newShape.vertex.end(), { x4, y4, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x2, y2, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x3, y3, 0.0f, r, g, b });
	}
	else if (shape == 3) {
		newShape.vertex.insert(newShape.vertex.end(), { x5, y5, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x1, y1, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x2, y2, 0.0f, r, g, b });

		newShape.vertex.insert(newShape.vertex.end(), { x5, y5, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x2, y2, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x3, y3, 0.0f, r, g, b });

		newShape.vertex.insert(newShape.vertex.end(), { x5, y5, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x3, y3, 0.0f, r, g, b });
		newShape.vertex.insert(newShape.vertex.end(), { x4, y4, 0.0f, r, g, b });
	}
	return newShape;
}


void reset_shape(int shape, float x, float y, float size) //shape 0:오각-선, 1:선-삼각형, 2:삼각-사각형, 3:사각-오각형
{
	DOTS new_dots = {{
		{{x - (size / 2.0f), y - size}, {x + (size / 2.0f), y - size}, {x + size, y}, {x, y + size}, {x - size, y}},
		{{x - size, y - size}, {x + size, y - size}, {x,y-size}, {0.0f, 0.0f}, {0.0f, 0.0f}},
		{{x - size, y - size}, {x + size, y - size}, {x,y + size}, {x,y + size}, {0.0f, 0.0f}},
		{{x - size, y - size}, {x + size, y - size}, {x+size,y + size}, {x,y + size}, {x-size,y + size}},
	} };
	dots.push_back(new_dots);


	shapes.push_back(make_shape(shape, new_dots.dot[shape][0].x, new_dots.dot[shape][0].y,
		new_dots.dot[shape][1].x, new_dots.dot[shape][1].y, 
		new_dots.dot[shape][2].x, new_dots.dot[shape][2].y, 
		new_dots.dot[shape][3].x, new_dots.dot[shape][3].y, 
		new_dots.dot[shape][4].x, new_dots.dot[shape][4].y,
		rr,rg,rb));

	UpdateBuffer();
}
