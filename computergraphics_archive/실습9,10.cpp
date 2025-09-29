#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>
#include <vector>

#define WIDTH 600
#define HEIGHT 600

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
void InitLines();
GLvoid Timer(int value);

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

int mode = 0; // 0: 점, 1: 선, 2: 삼각형, 3: 사각형
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
	width = WIDTH; height = HEIGHT;

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
	InitLines();

	glutDisplayFunc(drawScene); //--- 출력 콜백 함수
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(1000 / 60, Timer, 1); //--- 타이머 콜백함수 지정 (60 FPS)

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

std::uniform_real_distribution<float> rdmovexy(-0.01f, 0.1f);
struct SHAPE {
	int shape; // 0: 면, 1: 선
	std::vector<GLfloat> vertex;
	int locate; // 0:1사분면 1:2사분면 2:3사분면 3:4사분면
	float x, y;
	float r, g, b;
	float size;

	int move = 0;

	float movex = rdmovexy(mt), movey = rdmovexy(mt);

	bool UpDown = true; // true: 위로 false: 아래로
	bool UpDownAnime = false;
	int UpDownTimer = 0;

	float speed;
	int spiral = 0; // 0: 오른쪽, 1:아래 2:왼쪽 3:위
	float spiralbox[4]; // 위 아래 왼쪽 오른쪽 경계 ( 점점 좁아지게 해서 스파이럴할거임)

	float angle = 0.0f;
	float radius = 0.0f;

};
std::vector<GLfloat> allVertices;
std::vector<SHAPE> shapes;
GLuint lineVAO, lineVBO;

void UpdateBuffer()
{
	allVertices.clear();
	for (int i = 0; i < shapes.size(); i++)
		allVertices.insert(allVertices.end(), shapes[i].vertex.begin(), shapes[i].vertex.end());

	// 합쳐진 데이터로 VBO 업데이트
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(GLfloat), allVertices.data(), GL_DYNAMIC_DRAW);
}

void InitLines()
{
	GLfloat lineVertices[] = {
		-1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 0.0f,
		 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 0.0f,
		  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		  0.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f
	};

	glGenVertexArrays(1, &lineVAO);
	glGenBuffers(1, &lineVBO);

	glBindVertexArray(lineVAO);

	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	Mouse(GLUT_RIGHT_BUTTON, GLUT_DOWN, 450, 450);
	Mouse(GLUT_RIGHT_BUTTON, GLUT_DOWN, 150, 450);
	Mouse(GLUT_RIGHT_BUTTON, GLUT_DOWN, 150, 150);
	Mouse(GLUT_RIGHT_BUTTON, GLUT_DOWN, 450, 150);
}

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgramID);

	glBindVertexArray(lineVAO);
	glDrawArrays(GL_LINES, 0, 4);

	glBindVertexArray(vao);
	GLint first = 0;
	for (int i = 0; i < shapes.size(); i++) {
		int vertexCount = shapes[i].vertex.size() / 6;

		// 올바른 시작 위치(first)에서 그리기
		if (shapes[i].shape == 0) glDrawArrays(GL_TRIANGLES, first, vertexCount);
		else if (shapes[i].shape == 1) glDrawArrays(GL_LINE_LOOP, first, vertexCount);

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

int shapeCount[4] = {0,0,0,0};
GLvoid Mouse(int button, int state, int x, int y)
{
	std::uniform_real_distribution<float> rdcolor(0.0f, 1.0f);
	std::uniform_real_distribution<float> rdsize(0.1f, 0.2f);

	float mx = (2.0f * x / width) - 1.0f;
	float my = 1.0f - (2.0f * y / height);

	int nowlocate = -1;

	if(mx >= 0.0f && my >= 0.0f) nowlocate = 0; // 1사분면
	else if (mx < 0.0f && my >= 0.0f) nowlocate = 1; // 2사분면
	else if (mx < 0.0f && my < 0.0f) nowlocate = 2; // 3사분면
	else if (mx >= 0.0f && my < 0.0f) nowlocate = 3; // 4사분면

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		SHAPE newShape;
		newShape.shape = mode;
		newShape.x = mx;
		newShape.y = my;
		newShape.locate = nowlocate;
		newShape.movex = rdmovexy(mt);
		newShape.movey = rdmovexy(mt);
		newShape.size = rdsize(mt);
		newShape.r = rdcolor(mt);
		newShape.g = rdcolor(mt);
		newShape.b = rdcolor(mt);
		
		newShape.vertex.push_back(mx);
		newShape.vertex.push_back(my + (newShape.size * 2));
		newShape.vertex.push_back(0.0f);
		newShape.vertex.push_back(newShape.r);
		newShape.vertex.push_back(newShape.g);
		newShape.vertex.push_back(newShape.b);

		newShape.vertex.push_back(mx - newShape.size);
		newShape.vertex.push_back(my - newShape.size);
		newShape.vertex.push_back(0.0f);
		newShape.vertex.push_back(newShape.r);
		newShape.vertex.push_back(newShape.g);
		newShape.vertex.push_back(newShape.b);

		newShape.vertex.push_back(mx + newShape.size);
		newShape.vertex.push_back(my - newShape.size);
		newShape.vertex.push_back(0.0f);
		newShape.vertex.push_back(newShape.r);
		newShape.vertex.push_back(newShape.g);
		newShape.vertex.push_back(newShape.b);


		shapes.push_back(newShape);
		shapeCount[nowlocate]++;
		for(int i = 0 ; i < shapes.size() ; i++) {
			if(shapes[i].locate == nowlocate) {
				shapes.erase(shapes.begin() + i);
				shapeCount[nowlocate]--;
				break;
			}
		}

	}
	else if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		if (shapeCount[nowlocate] < 4) {
			SHAPE newShape;
			newShape.shape = mode;
			newShape.x = mx;
			newShape.y = my;
			newShape.locate = nowlocate;
			newShape.movex = rdmovexy(mt);
			newShape.movey = rdmovexy(mt);
			newShape.size = rdsize(mt);
			newShape.r = rdcolor(mt);
			newShape.g = rdcolor(mt);
			newShape.b = rdcolor(mt);

			newShape.vertex.push_back(mx);
			newShape.vertex.push_back(my + (newShape.size * 2));
			newShape.vertex.push_back(0.0f);
			newShape.vertex.push_back(newShape.r);
			newShape.vertex.push_back(newShape.g);
			newShape.vertex.push_back(newShape.b);

			newShape.vertex.push_back(mx - newShape.size);
			newShape.vertex.push_back(my - newShape.size);
			newShape.vertex.push_back(0.0f);
			newShape.vertex.push_back(newShape.r);
			newShape.vertex.push_back(newShape.g);
			newShape.vertex.push_back(newShape.b);

			newShape.vertex.push_back(mx + newShape.size);
			newShape.vertex.push_back(my - newShape.size);
			newShape.vertex.push_back(0.0f);
			newShape.vertex.push_back(newShape.r);
			newShape.vertex.push_back(newShape.g);
			newShape.vertex.push_back(newShape.b);

			shapes.push_back(newShape);

			shapeCount[nowlocate]++;
		}
	}

	UpdateBuffer();

	glutPostRedisplay();
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'a':
		mode = 0;
		break;
	case 'b':
		mode = 1;
		break;
	case 'c':
		shapes.clear();
		allVertices.clear();
		UpdateBuffer();
		shapeCount[0] = 0; shapeCount[1] = 0; shapeCount[2] = 0; shapeCount[3] = 0;
		break;
	case 'q':
		exit(0);
		break;
	case '1':
		for (int i = 0; i < shapes.size(); i++) shapes[i].move = 1;
		break;
	case '2':
		for (int i = 0; i < shapes.size(); i++) shapes[i].move = 2;
		break;
	case '3':
		for (int i = 0; i < shapes.size(); i++) {
			std::uniform_real_distribution<float> rdspeed(0.01f, 0.05f);
			shapes[i].move = 3;
			float speed = rdspeed(mt);
			float oldx = shapes[i].x;
			float oldy = shapes[i].y;

			shapes[i].x = 0.0f;
			shapes[i].y = 0.9f;

			float dx = shapes[i].x - oldx;
			float dy = shapes[i].y - oldy;
			for (int j = 0; j < shapes[i].vertex.size(); j += 6) {
				shapes[i].vertex[j] += dx;
				shapes[i].vertex[j + 1] += dy;
			}

			shapes[i].speed = speed;
			shapes[i].spiral = 2; 
			shapes[i].spiralbox[0] = 0.9f;  
			shapes[i].spiralbox[1] = -0.9f; 
			shapes[i].spiralbox[2] = -0.9f; 
			shapes[i].spiralbox[3] = 0.9f;  
		}
		break;
	case '4':
		for (int i = 0; i < shapes.size(); i++) {
			std::uniform_real_distribution<float> rdspeed(0.01f, 0.05f);
			shapes[i].move = 4;
			float speed = rdspeed(mt);
			float oldx = shapes[i].x;
			float oldy = shapes[i].y;

			shapes[i].x = 0.0f;
			shapes[i].y = 0.0f;

			float dx = shapes[i].x - oldx;
			float dy = shapes[i].y - oldy;
			for (int j = 0; j < shapes[i].vertex.size(); j += 6) {
				shapes[i].vertex[j] += dx;
				shapes[i].vertex[j + 1] += dy;
			}

			shapes[i].speed = speed;
			shapes[i].angle = 0.0f;
			shapes[i].radius = 0.0f;
		}
		break;
	}

	glutPostRedisplay(); // 다시 그리기 요청
}

void Timer(int value)
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
		else if (shapes[i].move == 2) {
			if (!shapes[i].UpDownAnime) {
				shapes[i].x += shapes[i].movex;
				if (shapes[i].x > 1.0f || shapes[i].x < -1.0f) {
					shapes[i].movex *= -1.0f;
					shapes[i].UpDownAnime = true;
				}
			}
			else {
				if(shapes[i].UpDown) {
					shapes[i].y += 0.01f;
					shapes[i].UpDownTimer++;
					if (shapes[i].UpDownTimer >= 20) {
						shapes[i].UpDownTimer = 0;
						shapes[i].UpDownAnime = false;
					}
					if (shapes[i].y > 1.0f) {
						shapes[i].UpDownTimer = 0;
						shapes[i].UpDownAnime = false;
						shapes[i].UpDown = false;
					}
				}
				else {
					shapes[i].y -= 0.01f;
					shapes[i].UpDownTimer++;
					if (shapes[i].UpDownTimer >= 20) {
						shapes[i].UpDownTimer = 0;
						shapes[i].UpDownAnime = false;
					}
					if (shapes[i].y < -1.0f) {
						shapes[i].UpDownTimer = 0;
						shapes[i].UpDownAnime = false;
						shapes[i].UpDown = true;
					}
				}
			}
		}
		else if (shapes[i].move == 3) {
			
			float centerDistance = sqrt((shapes[i].x * shapes[i].x) + (shapes[i].y * shapes[i].y));
			if (centerDistance <= 0.02f) {
				shapes[i].move = 0;
				continue;
			}

			if (shapes[i].spiral == 0) {
				shapes[i].x += shapes[i].speed;
				if (shapes[i].x >= shapes[i].spiralbox[3]) {
					shapes[i].x = shapes[i].spiralbox[3];
					shapes[i].spiral = 3;
					shapes[i].spiralbox[0] -= 0.1f;
				}
			}
			else if( shapes[i].spiral == 1) {
				shapes[i].y -= shapes[i].speed;
				if (shapes[i].y <= shapes[i].spiralbox[1]) {
					shapes[i].y = shapes[i].spiralbox[1];
					shapes[i].spiral = 0;
					shapes[i].spiralbox[3] -= 0.1f;
				}
			}
			else if (shapes[i].spiral == 2) {
				shapes[i].x -= shapes[i].speed;
				if (shapes[i].x <= shapes[i].spiralbox[2]) {
					shapes[i].x = shapes[i].spiralbox[2];
					shapes[i].spiral = 1;
					shapes[i].spiralbox[1] += 0.1f;
				}
			}
			else if (shapes[i].spiral == 3) {
				shapes[i].y += shapes[i].speed;
				if (shapes[i].y >= shapes[i].spiralbox[0]) {
					shapes[i].y = shapes[i].spiralbox[0];
					shapes[i].spiral = 2;
					shapes[i].spiralbox[2] += 0.1f;
				}
			}
		}
		else if(shapes[i].move == 4) {

			shapes[i].angle += 0.05f;

			if (shapes[i].radius <= 0.8f) {
				shapes[i].radius += shapes[i].speed / 100.0f;
			}

			shapes[i].x = cos(shapes[i].angle) * shapes[i].radius;
			shapes[i].y = sin(shapes[i].angle) * shapes[i].radius;
		}

		if (shapes[i].move == 2 || shapes[i].move == 3 || shapes[i].move == 4) {

			float wayx = shapes[i].x - oldx;
			float wayy = shapes[i].y - oldy;
			
			float dist = sqrt(wayx * wayx + wayy * wayy);
			if (dist > 0.0001f) {
				wayx /= dist;
				wayy /= dist;

				float size = shapes[i].size;

				float v1x = shapes[i].x + wayx * size*2;
				float v1y = shapes[i].y + wayy * size*2;

				float perp_x = -wayy;
				float perp_y = wayx;

				float v2x = shapes[i].x - wayx * size + perp_x * size;
				float v2y = shapes[i].y - wayy * size + perp_y * size;

				float v3x = shapes[i].x - wayx * size - perp_x * size;
				float v3y = shapes[i].y - wayy * size - perp_y * size;

				shapes[i].vertex.clear();
				shapes[i].vertex.insert(shapes[i].vertex.end(), { v1x, v1y, 0.0f, shapes[i].r, shapes[i].g, shapes[i].b });
				shapes[i].vertex.insert(shapes[i].vertex.end(), { v2x, v2y, 0.0f, shapes[i].r, shapes[i].g, shapes[i].b });
				shapes[i].vertex.insert(shapes[i].vertex.end(), { v3x, v3y, 0.0f, shapes[i].r, shapes[i].g, shapes[i].b });
			}
		}
		else {
			float dx = shapes[i].x - oldx;
			float dy = shapes[i].y - oldy;
			for (int j = 0; j < shapes[i].vertex.size(); j += 6) {
				shapes[i].vertex[j] += dx;
				shapes[i].vertex[j + 1] += dy;
			}
		}
	}



	UpdateBuffer();
	glutPostRedisplay(); // 화면 갱신 요청

	glutTimerFunc(1000 / 60, Timer, 1); // 타이머 재설정 (60 FPS)
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