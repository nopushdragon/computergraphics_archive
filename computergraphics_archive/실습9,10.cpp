#define _CRT_SECURE_NO_WARNINGS //--- ���α׷� �� �տ� ������ ��
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

//--- �ʿ��� ���� ����
GLint width, height;
GLuint shaderProgramID; //--- ���̴� ���α׷� �̸�
GLuint vertexShader; //--- ���ؽ� ���̴� ��ü
GLuint fragmentShader; //--- �����׸�Ʈ ���̴� ��ü

std::vector<GLfloat> dotver;
std::vector<GLfloat> linever;
std::vector<GLfloat> triver;
std::vector<GLfloat> rectver;
GLuint vao, vbo;

int mode = 0; // 0: ��, 1: ��, 2: �ﰢ��, 3: �簢��
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

void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	width = WIDTH; height = HEIGHT;

	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Example1");

	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	glewInit();

	//--- ���̴� �о�ͼ� ���̴� ���α׷� �����: ����� �����Լ� ȣ��
	make_vertexShaders(); //--- ���ؽ� ���̴� �����
	make_fragmentShaders(); //--- �����׸�Ʈ ���̴� �����
	shaderProgramID = make_shaderProgram();	//--- ���̴� ���α׷� �����

	// ����(VAO/VBO/EBO) �ʱ�ȭ (���̴� ���α׷� ���� �� ȣ��)
	InitBuffer();
	InitLines();

	glutDisplayFunc(drawScene); //--- ��� �ݹ� �Լ�
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(1000 / 60, Timer, 1); //--- Ÿ�̸� �ݹ��Լ� ���� (60 FPS)

	glutMainLoop();
}

void make_vertexShaders()
{
	GLchar* vertexSource;

	//--- ���ؽ� ���̴� �о� �����ϰ� ������ �ϱ�
	//--- filetobuf: ��������� �Լ��� �ؽ�Ʈ�� �о ���ڿ��� �����ϴ� �Լ�

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
		std::cerr << "ERROR: vertex shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}

//--- �����׸�Ʈ ���̴� ��ü �����
void make_fragmentShaders()
{
	GLchar* fragmentSource;

	//--- �����׸�Ʈ ���̴� �о� �����ϰ� �������ϱ�
	fragmentSource = filetobuf("fragment.glsl"); // �����׼��̴� �о����
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}

GLuint make_shaderProgram()
{
	GLint result;
	GLchar* errorLog = NULL;
	GLuint shaderID;
	shaderID = glCreateProgram(); //--- ���̴� ���α׷� �����
	glAttachShader(shaderID, vertexShader); //--- ���̴� ���α׷��� ���ؽ� ���̴� ���̱�
	glAttachShader(shaderID, fragmentShader); //--- ���̴� ���α׷��� �����׸�Ʈ ���̴� ���̱�
	glLinkProgram(shaderID); //--- ���̴� ���α׷� ��ũ�ϱ�
	glDeleteShader(vertexShader); //--- ���̴� ��ü�� ���̴� ���α׷��� ��ũ��������, ���̴� ��ü ��ü�� ���� ����
	glDeleteShader(fragmentShader);
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // ---���̴��� �� ����Ǿ����� üũ�ϱ�
	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program ���� ����\n" << errorLog << std::endl;
		return false;
	}
	glUseProgram(shaderID); //--- ������� ���̴� ���α׷� ����ϱ�
	//--- ���� ���� ���̴����α׷� ���� �� �ְ�, �� �� �Ѱ��� ���α׷��� ����Ϸ���
	//--- glUseProgram �Լ��� ȣ���Ͽ� ��� �� Ư�� ���α׷��� �����Ѵ�.
	//--- ����ϱ� ������ ȣ���� �� �ִ�.
	return shaderID;
}

std::uniform_real_distribution<float> rdmovexy(-0.01f, 0.1f);
struct SHAPE {
	int shape; // 0: ��, 1: ��
	std::vector<GLfloat> vertex;
	int locate; // 0:1��и� 1:2��и� 2:3��и� 3:4��и�
	float x, y;
	float r, g, b;
	float size;

	int move = 0;

	float movex = rdmovexy(mt), movey = rdmovexy(mt);

	bool UpDown = true; // true: ���� false: �Ʒ���
	bool UpDownAnime = false;
	int UpDownTimer = 0;

	float speed;
	int spiral = 0; // 0: ������, 1:�Ʒ� 2:���� 3:��
	float spiralbox[4]; // �� �Ʒ� ���� ������ ��� ( ���� �������� �ؼ� �����̷��Ұ���)

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

	// ������ �����ͷ� VBO ������Ʈ
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

GLvoid drawScene() //--- �ݹ� �Լ�: �׸��� �ݹ� �Լ�
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

		// �ùٸ� ���� ��ġ(first)���� �׸���
		if (shapes[i].shape == 0) glDrawArrays(GL_TRIANGLES, first, vertexCount);
		else if (shapes[i].shape == 1) glDrawArrays(GL_LINE_LOOP, first, vertexCount);

		// ���� ������ ���� ���� ��ġ ������Ʈ
		first += vertexCount;
	}

	glBindVertexArray(0);
	glutSwapBuffers();
}

GLvoid Reshape(int w, int h) //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
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

	if(mx >= 0.0f && my >= 0.0f) nowlocate = 0; // 1��и�
	else if (mx < 0.0f && my >= 0.0f) nowlocate = 1; // 2��и�
	else if (mx < 0.0f && my < 0.0f) nowlocate = 2; // 3��и�
	else if (mx >= 0.0f && my < 0.0f) nowlocate = 3; // 4��и�

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

	glutPostRedisplay(); // �ٽ� �׸��� ��û
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
	glutPostRedisplay(); // ȭ�� ���� ��û

	glutTimerFunc(1000 / 60, Timer, 1); // Ÿ�̸� �缳�� (60 FPS)
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// �ʱ⿡�� �� ����
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	// --- ��ġ �Ӽ� (location = 0, vec3)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// --- ���� �Ӽ� (location = 1, vec3)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}