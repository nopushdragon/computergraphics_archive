#define _CRT_SECURE_NO_WARNINGS //--- ���α׷� �� �տ� ������ ��
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
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Timer(int value);
void InitBuffer();

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

std::vector<GLfloat> allVertices;

struct SHAPE {
	std::vector<GLfloat> vertex;
};
std::vector<SHAPE> shapes;
SHAPE make_triangle(float x1, float y1, float x2, float y2, float x3, float y3, float r, float g, float b);
void resetShape();

int moveState = 0; // 0: ����, 1: �ð� 2: �ݽð�
bool changeSize = false;
float radius = 1.0f;
float centerX = 0.0f, centerY = 0.0f;

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
	width = 500;
	height = 500;

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
	resetShape();

	glutDisplayFunc(drawScene); //--- ��� �ݹ� �Լ�
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(1000 / 120, Timer, 1); //--- Ÿ�̸� �ݹ��Լ� ���� (60 FPS)

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

void UpdateBuffer()
{
	allVertices.clear();

	for (int i = 0; i < shapes.size(); i++)	allVertices.insert(allVertices.end(), shapes[i].vertex.begin(), shapes[i].vertex.end());

	// ������ �����ͷ� VBO ������Ʈ
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(GLfloat), allVertices.data(), GL_DYNAMIC_DRAW);
}

GLvoid drawScene() //--- �ݹ� �Լ�: �׸��� �ݹ� �Լ�
{
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgramID);
	glBindVertexArray(vao);

	GLint first = 0;
	for (int i = 0; i < shapes.size(); i++) {
		int vertexCount = shapes[i].vertex.size() / 6;
		glDrawArrays(GL_TRIANGLES, first, vertexCount);

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

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case '1':
		shapes.clear();
		shapes.push_back(make_triangle(0.0f, 0.4f, -0.2f, 0.2f, 0.2f, 0.2f, rdcolor(mt), rdcolor(mt), rdcolor(mt)));
		shapes.push_back(make_triangle(0.4f, 0.0f, 0.2f, 0.2f, 0.2f, -0.2f, rdcolor(mt), rdcolor(mt), rdcolor(mt)));
		shapes.push_back(make_triangle(0.0f, -0.4f, 0.2f, -0.2f, -0.2f, -0.2f, rdcolor(mt), rdcolor(mt), rdcolor(mt)));
		shapes.push_back(make_triangle(-0.4f, 0.0f, -0.2f, -0.2f, -0.2f, 0.2f, rdcolor(mt), rdcolor(mt), rdcolor(mt)));
		UpdateBuffer();
		break;
	case '2':
		shapes.clear();
		shapes.push_back(make_triangle(0.0f, 0.05f, -0.2f, 0.2f, 0.2f, 0.2f, rdcolor(mt), rdcolor(mt), rdcolor(mt)));
		shapes.push_back(make_triangle(0.05f, 0.0f, 0.2f, 0.2f, 0.2f, -0.2f, rdcolor(mt), rdcolor(mt), rdcolor(mt)));
		shapes.push_back(make_triangle(0.0f, -0.05f, 0.2f, -0.2f, -0.2f, -0.2f, rdcolor(mt), rdcolor(mt), rdcolor(mt)));
		shapes.push_back(make_triangle(-0.05f, 0.0f, -0.2f, -0.2f, -0.2f, 0.2f, rdcolor(mt), rdcolor(mt), rdcolor(mt)));
		UpdateBuffer();
		break;
	case '3':
		changeSize = !changeSize;
		radius = 1.0f;
		break;
	case '4':
		centerX = rdxy(mt);
		centerY = rdxy(mt);
		break;
	case 'c':
		moveState = 1; // �ð���� ȸ��
		break;
	case 't':
		moveState = 2; // �ݽð���� ȸ��
		break;
	case 's':
		moveState = 0; // ����
		break;
	case 'q':
		exit(0);
		break;
	}

	glutPostRedisplay(); // �ٽ� �׸��� ��û
}

bool bigsmall = false;
GLvoid Timer(int value) //--- �ݹ� �Լ�: Ÿ�̸� �ݹ� �Լ�
{
	if(moveState != 0)
	{
		if (changeSize) {
			if (!bigsmall) {
				radius += 0.005f;
				if (radius >= 1.1f) bigsmall = true;
			}
			else {
				radius -= 0.005f;
				if (radius <= 0.9f) bigsmall = false;
			}
		}

		float angle = (moveState == 1) ? -0.02f : 0.02f; // �ð���� �Ǵ� �ݽð����
		for (int i = 0; i < shapes.size(); i++) {
			for (int j = 0; j < shapes[i].vertex.size(); j += 6) {
				float x = shapes[i].vertex[j];
				float y = shapes[i].vertex[j + 1];
				// ȸ�� ��ȯ ���� ����
				float newX =  (x - centerX) * cos(angle) - (y - centerY) * sin(angle);
				float newY =  (x - centerX) * sin(angle) + (y - centerY) * cos(angle);

				if (changeSize)
				{
					newX *= radius;
					newY *= radius;
				}

				shapes[i].vertex[j] = newX + centerX;
				shapes[i].vertex[j + 1] = newY + centerY;
			}
		}
	}
	UpdateBuffer();
	glutPostRedisplay(); // �ٽ� �׸��� ��û
	glutTimerFunc(1000 / 120, Timer, 1); //--- Ÿ�̸� �ݹ��Լ� ���� (60 FPS)
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

void resetShape()
{
	shapes.clear();

	shapes.push_back(make_triangle(0.0f, 0.4f, -0.2f, 0.2f, 0.2f, 0.2f, rdcolor(mt), rdcolor(mt), rdcolor(mt)));
	shapes.push_back(make_triangle(0.4f, 0.0f, 0.2f, 0.2f, 0.2f, -0.2f, rdcolor(mt), rdcolor(mt), rdcolor(mt)));
	shapes.push_back(make_triangle(0.0f, -0.4f, 0.2f, -0.2f, -0.2f, -0.2f, rdcolor(mt), rdcolor(mt), rdcolor(mt)));
	shapes.push_back(make_triangle(-0.4f, 0.0f, -0.2f, -0.2f, -0.2f, 0.2f, rdcolor(mt), rdcolor(mt), rdcolor(mt)));

	UpdateBuffer();
}

SHAPE make_triangle( float x1, float y1, float x2, float y2, float x3, float y3, float r, float g, float b)
{
	SHAPE newShape;
	newShape.vertex.insert(newShape.vertex.end(), { x1, y1, 0.0f, r, g, b });
	newShape.vertex.insert(newShape.vertex.end(), { x2, y2, 0.0f, r, g, b });
	newShape.vertex.insert(newShape.vertex.end(), { x3, y3, 0.0f, r, g, b });
	return newShape;
}
