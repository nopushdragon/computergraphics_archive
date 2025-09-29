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

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Keyboard(unsigned char key, int x, int y);
void InitBuffer();
void Moveshape(float x, float y);

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

	glutDisplayFunc(drawScene); //--- ��� �ݹ� �Լ�
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);

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

std::vector<GLfloat> allVertices;
struct SHAPE {
	int shape; // 0: ��, 1: ��, 2: �ﰢ��, 3: �簢��
	std::vector<GLfloat> vertex;
	float x, y;
};
std::vector<SHAPE> shapes;

void UpdateBuffer()
{
	allVertices.clear();
	for(int i = 0 ; i < shapes.size(); i++) 
		if(shapes[i].shape == 0) allVertices.insert(allVertices.end(), shapes[i].vertex.begin(), shapes[i].vertex.end());
	for (int i = 0; i < shapes.size(); i++) 
		if (shapes[i].shape == 1) allVertices.insert(allVertices.end(), shapes[i].vertex.begin(), shapes[i].vertex.end());
	for (int i = 0; i < shapes.size(); i++) 
		if (shapes[i].shape == 2) allVertices.insert(allVertices.end(), shapes[i].vertex.begin(), shapes[i].vertex.end());
	for (int i = 0; i < shapes.size(); i++) 
		if (shapes[i].shape == 3) allVertices.insert(allVertices.end(), shapes[i].vertex.begin(), shapes[i].vertex.end());

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

		// �ùٸ� ���� ��ġ(first)���� �׸���
		if (shapes[i].shape == 0) glDrawArrays(GL_TRIANGLES, first, vertexCount);
		else if (shapes[i].shape == 1) glDrawArrays(GL_LINES, first, vertexCount);
		else if (shapes[i].shape == 2) glDrawArrays(GL_TRIANGLES, first, vertexCount);
		else if (shapes[i].shape == 3) glDrawArrays(GL_TRIANGLES, first, vertexCount);

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

GLvoid Mouse(int button, int state, int x, int y) 
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		float mx = (2.0f * x / width) - 1.0f;
		float my = 1.0f - (2.0f * y / height);

		static int shapeCount = 0;
		if (shapeCount < 10) {
			SHAPE newShape;
			newShape.shape = mode;
			newShape.x = mx;
			newShape.y = my;

			float size = 0.1f; // ũ��
			if (mode == 0) {
				float r = rdcolor(mt);
				float g = rdcolor(mt);
				float b = rdcolor(mt);

				newShape.vertex.push_back(mx - (size - 0.08f));
				newShape.vertex.push_back(my + (size - 0.08f));
				newShape.vertex.push_back(0.0f);
				newShape.vertex.push_back(r);
				newShape.vertex.push_back(g);
				newShape.vertex.push_back(b);

				newShape.vertex.push_back(mx - (size - 0.08f));
				newShape.vertex.push_back(my - (size - 0.08f));
				newShape.vertex.push_back(0.0f);
				newShape.vertex.push_back(r);
				newShape.vertex.push_back(g);
				newShape.vertex.push_back(b);

				newShape.vertex.push_back(mx + (size - 0.08f));
				newShape.vertex.push_back(my + (size - 0.08f));
				newShape.vertex.push_back(0.0f);
				newShape.vertex.push_back(r);
				newShape.vertex.push_back(g);
				newShape.vertex.push_back(b);
				//2���� �ﰢ��	
				newShape.vertex.push_back(mx - (size - 0.08f));
				newShape.vertex.push_back(my - (size - 0.08f));
				newShape.vertex.push_back(0.0f);
				newShape.vertex.push_back(r);
				newShape.vertex.push_back(g);
				newShape.vertex.push_back(b);

				newShape.vertex.push_back(mx + (size - 0.08f));
				newShape.vertex.push_back(my - (size - 0.08f));
				newShape.vertex.push_back(0.0f);
				newShape.vertex.push_back(r);
				newShape.vertex.push_back(g);
				newShape.vertex.push_back(b);

				newShape.vertex.push_back(mx + (size - 0.08f));
				newShape.vertex.push_back(my + (size - 0.08f));
				newShape.vertex.push_back(0.0f);
				newShape.vertex.push_back(r);
				newShape.vertex.push_back(g);
				newShape.vertex.push_back(b);
			}
			else if (mode == 1) {
				// Ŭ�� ���� �߽����� ���� �߰�
				newShape.vertex.push_back(mx - size);
				newShape.vertex.push_back(my);
				newShape.vertex.push_back(0.0f);
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));

				newShape.vertex.push_back(mx + size);
				newShape.vertex.push_back(my);
				newShape.vertex.push_back(0.0f);
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));
			}
			else if (mode == 2) {
				// Ŭ�� ���� �߽����� �ﰢ�� 3�� ���� �߰�
				newShape.vertex.push_back(mx);
				newShape.vertex.push_back(my + size);
				newShape.vertex.push_back(0.0f);
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));

				newShape.vertex.push_back(mx - size);
				newShape.vertex.push_back(my - size);
				newShape.vertex.push_back(0.0f);
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));

				newShape.vertex.push_back(mx + size);
				newShape.vertex.push_back(my - size);
				newShape.vertex.push_back(0.0f);
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));
			}
			else if (mode == 3) {
				newShape.vertex.push_back(mx - size);
				newShape.vertex.push_back(my + size);
				newShape.vertex.push_back(0.0f);
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));

				newShape.vertex.push_back(mx - size);
				newShape.vertex.push_back(my - size);
				newShape.vertex.push_back(0.0f);
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));

				newShape.vertex.push_back(mx + size);
				newShape.vertex.push_back(my + size);
				newShape.vertex.push_back(0.0f);
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));
				//2���� �ﰢ��	
				newShape.vertex.push_back(mx - size);
				newShape.vertex.push_back(my - size);
				newShape.vertex.push_back(0.0f);
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));

				newShape.vertex.push_back(mx + size);
				newShape.vertex.push_back(my - size);
				newShape.vertex.push_back(0.0f);
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));

				newShape.vertex.push_back(mx + size);
				newShape.vertex.push_back(my + size);
				newShape.vertex.push_back(0.0f);
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));
				newShape.vertex.push_back(rdcolor(mt));
			}
			shapes.push_back(newShape);
			shapeCount++;
		}
		else {
			for(int i = 0 ; i < shapes.size() ; i++) {
				float dx = shapes[i].x - mx;
				float dy = shapes[i].y - my;
				float dist = sqrt(dx * dx + dy * dy);
				if(dist < 0.1f) {
					nowshape = i;
					break;
				}
			}
		}
		UpdateBuffer();

		glutPostRedisplay();
	}
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case '1':
		mode = 0;
		break;
	case '2':
		mode = 1;
		break;
	case '3':
		mode = 2;
		break;
	case '4':
		mode = 3;
		break;
	case 'c':
		shapes.clear();
		UpdateBuffer();
		break;
	case 'w':
		Moveshape(0.0f, 0.1f);
		break;
	case 'a':
		Moveshape(-0.1f, 0.0f);
		break;
	case 's':
		Moveshape(0.0f, -0.1f);
		break;
	case 'd':
		Moveshape(0.1f, 0.0f);
		break;
	case 'i':
		Moveshape(-0.1f, 0.1f);
		break;
	case 'j':
		Moveshape(0.1f, 0.1f);
		break;
	case 'k':
		Moveshape(-0.1f, -0.1f);
		break;
	case 'l':
		Moveshape(0.1f, -0.1f);
		break;
	case 'q':
		exit(0);
		break;
	}

	glutPostRedisplay(); // �ٽ� �׸��� ��û
}

void Moveshape(float x, float y)
{
	shapes[nowshape].x += x;
	shapes[nowshape].y += y;

	if (shapes[nowshape].shape == 0) { // ��
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
	else if (shapes[nowshape].shape == 1) { // ��
		shapes[nowshape].vertex[0] += x;
		shapes[nowshape].vertex[1] += y;
		shapes[nowshape].vertex[6] += x;
		shapes[nowshape].vertex[7] += y;
	}
	else if (shapes[nowshape].shape == 2) { // �ﰢ��
		shapes[nowshape].vertex[0] += x;
		shapes[nowshape].vertex[1] += y;
		shapes[nowshape].vertex[6] += x;
		shapes[nowshape].vertex[7] += y;
		shapes[nowshape].vertex[12] += x;
		shapes[nowshape].vertex[13] += y;
	}
	else if (shapes[nowshape].shape == 3) { // �簢��
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