#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#define MAX_LINE_LENGTH 256

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <vector>
#include <string.h>

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_real_distribution<float> rdcolor(0.0f, 1.0f);
std::uniform_int_distribution<int> rdface1(0, 5);
std::uniform_int_distribution<int> rdface2(0, 3);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Timer(int value);
void InitBuffer();
void InitAxisBuffer();
void LoadOBJ(const char* filename, int object_num);
void reset_bool();

//--- 필요한 변수 선언
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체
GLuint vao, vbo;
GLuint axis_vao, axis_vbo; // 좌표축을 위한 VAO, VBO

//
bool move_towards(float* current, float target, float speed);
bool MoveObjectToTarget(float* current_d, float* current_e,
    float target_d, float target_e,
    float b_size_stack, float base_speed);

bool axis_display = true;
bool depth_on = true;

int now_object = -1;
int isy = -1;
int isx = -1;
int isr = -1;
int isa = -1;
int isb = -1;
int isd = -1;
int ise = -1;
bool isc = false;
bool isv = false;
int ist = -1;
int isu = -1;

float x_radian_stack_1 = 0.0f;
float y_radian_stack_1 = 0.0f;
float r_radian_stack_1 = 0.0f;
float a_size_stack_1 = 1.0f;
float b_size_stack_1 = 1.0f;
float d_length_stack_1 = 0.0f;
float e_length_stack_1 = 0.0f;
float v_size_stack_1 = 1.0f;
float v_j_radian_stack_1 = 0.0f;
float v_g_radian_stack_1 = 0.0f;
float t_final_d_1, t_final_e_1;
float u_final_d_1, u_final_e_1;

float x_radian_stack_2 = 0.0f;
float y_radian_stack_2 = 0.0f;
float r_radian_stack_2 = 0.0f;
float a_size_stack_2 = 1.0f;
float b_size_stack_2 = 1.0f;
float d_length_stack_2 = 0.0f;
float e_length_stack_2 = 0.0f;
float v_size_stack_2 = 1.0f;
float v_j_radian_stack_2 = 0.0f;
float v_g_radian_stack_2 = 0.0f;
float t_final_d_2, t_final_e_2;
float u_final_d_2, u_final_e_2;
//

std::vector<GLfloat> allVertices;
struct SHAPE {
    std::vector<GLfloat> vertex;
    glm::mat4 model = glm::mat4(1.0f);
    int face_count;
    int object_num;
};
std::vector<SHAPE> shapes;

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
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
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
    InitAxisBuffer();
    LoadOBJ("15_cube.obj", 0);
    LoadOBJ("sphere.obj", 1);
    LoadOBJ("15_pyramid.obj", 2);
    LoadOBJ("donut.obj", 3);

    glutDisplayFunc(drawScene); //--- 출력 콜백 함수
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);
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

void draw_axis()
{
    // 좌표축은 원점에 고정되므로 모델 행렬은 단위 행렬을 사용합니다.
    // view, projection 행렬은 drawScene에서 이미 셰이더로 전달되었으므로 재전송할 필요가 없습니다.
    glm::mat4 model = glm::mat4(1.0f);
    GLuint modelLoc = glGetUniformLocation(shaderProgramID, "uModel");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

    // 선 굵기를 2.0으로 설정 (기본값은 1.0)
    glLineWidth(2.0f);

    glBindVertexArray(axis_vao);
    // 3개의 선분(총 6개의 정점)을 그립니다.
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);
}

void UpdateBuffer()
{
    allVertices.clear();

    for (int i = 0; i < shapes.size(); i++)	allVertices.insert(allVertices.end(), shapes[i].vertex.begin(), shapes[i].vertex.end());

    // 합쳐진 데이터로 VBO 업데이트
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(GLfloat), allVertices.data(), GL_DYNAMIC_DRAW);
}

GLvoid drawScene() {
    if (depth_on)
        glEnable(GL_DEPTH_TEST); // 은면제거
    else
        glDisable(GL_DEPTH_TEST);

    //glEnable(GL_CULL_FACE); 뒷면제거
    //glDisable(GL_CULL_FACE);

    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (axis_display) draw_axis();

    glBindVertexArray(vao);
    glUseProgram(shaderProgramID);
    GLint first = 0;
    for (int i = 0; i < shapes.size(); i++) {
        glm::mat4 model = shapes[i].model;

        //model = glm::translate(model, glm::vec3(1.5f, 0.5f, 0.0f)); x축 1.5 y축 0.5만큼 이동
        //model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // y축 기준 45도 회전
        //model = glm::scale(model, glm::vec3(0.5f)); 스케일 조정

        glm::mat4 view = glm::lookAt(
            glm::vec3(3.0f, 3.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

        GLuint modelLoc = glGetUniformLocation(shaderProgramID, "uModel");
        GLuint viewLoc = glGetUniformLocation(shaderProgramID, "uView");
        GLuint projLoc = glGetUniformLocation(shaderProgramID, "uProj");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        if (modelLoc == -1 || viewLoc == -1 || projLoc == -1) {
            std::cerr << "ERROR: One or more uniform locations not found!" << std::endl;
        }

        int vertexCount = shapes[i].vertex.size() / 6;
        if ((shapes[i].object_num == 0 || shapes[i].object_num == 1) && !isc) {
            glDrawArrays(GL_TRIANGLES, first, vertexCount);
        }
        else if ((shapes[i].object_num == 2 || shapes[i].object_num == 3) && isc) {
            glDrawArrays(GL_TRIANGLES, first, vertexCount);
        }
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
    switch (key) {
    case '1':
        now_object = 1; // 좌측
        break;
    case '2':
        now_object = 2; // 우측
        break;
    case '3':
        now_object = 3; // 둘 다
        break;
    case 'h':
        depth_on = !depth_on;
        break;
    case 'x':
        reset_bool();
        isx = 0;
        break;
    case 'X':
        reset_bool();
        isx = 1;
        break;
    case 'y':
        reset_bool();
        isy = 0;
        break;
    case 'Y':
        reset_bool();
        isy = 1;
        break;
    case 'r':
        reset_bool();
        isr = 0;
        break;
    case 'R':
        reset_bool();
        isr = 1;
        break;
    case 'a':
        reset_bool();
        isa = 0;
        break;
    case 'A':
        reset_bool();
        isa = 1;
        break;
    case 'b':
        reset_bool();
        isb = 0;
        break;
    case 'B':
        reset_bool();
        isb = 1;
        break;
    case 'd':
        reset_bool();
        isd = 0;
        break;
    case 'D':
        reset_bool();
        isd = 1;
        break;
    case 'e':
        reset_bool();
        ise = 0;
        break;
    case 'E':
        reset_bool();
        ise = 1;
        break;
    case 't': {
        reset_bool();
        float obj1_current_x = (-1.0f + d_length_stack_1) * b_size_stack_1;
        float obj1_current_y = e_length_stack_1 * b_size_stack_1; // y축도 스케일 적용

        float obj2_current_x = (1.0f + d_length_stack_2) * b_size_stack_2;
        float obj2_current_y = e_length_stack_2 * b_size_stack_2; // y축도 스케일 적용

        t_final_d_1 = (obj2_current_x / b_size_stack_1) - (-1.0f);
        t_final_e_1 = (obj2_current_y / b_size_stack_1);

        t_final_d_2 = (obj1_current_x / b_size_stack_2) - (1.0f);
        t_final_e_2 = (obj1_current_y / b_size_stack_2);

        ist = 0;
        break;
    }
    case 'u': {
        reset_bool();
        float obj1_current_x = (-1.0f + d_length_stack_1) * b_size_stack_1;
        float obj1_current_y = e_length_stack_1 * b_size_stack_1; // y축도 스케일 적용

        float obj2_current_x = (1.0f + d_length_stack_2) * b_size_stack_2;
        float obj2_current_y = e_length_stack_2 * b_size_stack_2; // y축도 스케일 적용

        u_final_d_1 = (obj2_current_x / b_size_stack_1) - (-1.0f);
        u_final_e_1 = (obj2_current_y / b_size_stack_1);

        u_final_d_2 = (obj1_current_x / b_size_stack_2) - (1.0f);
        u_final_e_2 = (obj1_current_y / b_size_stack_2);

        isu = 0;
        break;
    }
    case 'v':
        reset_bool();
        isv = 0;
        break;
    case 'c':
        isc = !isc;
        break;
    case 's':
        x_radian_stack_1 = 0.0f;
        y_radian_stack_1 = 0.0f;
        r_radian_stack_1 = 0.0f;
        a_size_stack_1 = 1.0f;
        b_size_stack_1 = 1.0f;
        d_length_stack_1 = 0.0f;
        e_length_stack_1 = 0.0f;
        v_size_stack_1 = 1.0f;
        v_j_radian_stack_1 = 0.0f;
        v_g_radian_stack_1 = 0.0f;

        x_radian_stack_2 = 0.0f;
        y_radian_stack_2 = 0.0f;
        r_radian_stack_2 = 0.0f;
        a_size_stack_2 = 1.0f;
        b_size_stack_2 = 1.0f;
        d_length_stack_2 = 0.0f;
        e_length_stack_2 = 0.0f;
        v_size_stack_2 = 1.0f;
        v_j_radian_stack_2 = 0.0f;
        v_g_radian_stack_2 = 0.0f;
        break;
    case VK_TAB:
        axis_display = !axis_display;
        break;
    case 'q':
        exit(0);
        break;
    }

    glutPostRedisplay(); // 다시 그리기 요청
}

void reset_bool() {
    isx = -1;
    isy = -1;
    isr = -1;
    isa = -1;
    isb = -1;
    isd = -1;
    ise = -1;
    isv = false;
    ist = -1;
    isu = -1;
}

GLvoid Timer(int value) //--- 콜백 함수: 타이머 콜백 함수
{
    for (int i = 0;i < shapes.size();i++) {
        shapes[i].model = glm::mat4(1.0f);

        //스케일(원점)
        if (shapes[i].object_num == 0 || shapes[i].object_num == 2) {
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(b_size_stack_1));

        }
        else if (shapes[i].object_num == 1 || shapes[i].object_num == 3) {
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(b_size_stack_2));

        }

        //공전
        if (shapes[i].object_num == 0 || shapes[i].object_num == 2) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(r_radian_stack_1), glm::vec3(0.0f, 1.0f, 0.0f));
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(v_g_radian_stack_1), glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else if (shapes[i].object_num == 1 || shapes[i].object_num == 3) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(r_radian_stack_2), glm::vec3(0.0f, 1.0f, 0.0f));
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(v_g_radian_stack_2), glm::vec3(0.0f, 1.0f, 0.0f));
        }

        //이동
        if (shapes[i].object_num == 0 || shapes[i].object_num == 2) {
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, e_length_stack_1, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(d_length_stack_1, 0.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(-1.0f, 0.0f, 0.0f));
        }
        else if (shapes[i].object_num == 1 || shapes[i].object_num == 3) {
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, e_length_stack_2, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(d_length_stack_2, 0.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(1.0f, 0.0f, 0.0f));
        }

        //자전
        if (shapes[i].object_num == 0 || shapes[i].object_num == 2) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(y_radian_stack_1), glm::vec3(0.0f, 1.0f, 0.0f));
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(x_radian_stack_1), glm::vec3(1.0f, 0.0f, 0.0f));
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(v_j_radian_stack_1), glm::vec3(1.0f, 0.0f, 0.0f));
        }
        else if (shapes[i].object_num == 1 || shapes[i].object_num == 3) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(y_radian_stack_2), glm::vec3(0.0f, 1.0f, 0.0f));
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(x_radian_stack_2), glm::vec3(1.0f, 0.0f, 0.0f));
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(v_j_radian_stack_2), glm::vec3(1.0f, 0.0f, 0.0f));
        }

        //스케일(객체)
        if (shapes[i].object_num == 0 || shapes[i].object_num == 2) {
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(a_size_stack_1));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(v_size_stack_1));
        }
        else if (shapes[i].object_num == 1 || shapes[i].object_num == 3) {
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(a_size_stack_2));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(v_size_stack_2));
        }
        shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.3f));
    }

    if (isx == 0) {
        if (now_object == 1 || now_object == 3) x_radian_stack_1 += 1.0f;
        if (now_object == 2 || now_object == 3) x_radian_stack_2 += 1.0f;
    }
    else if (isx == 1) {
        if (now_object == 1 || now_object == 3) x_radian_stack_1 -= 1.0f;
        if (now_object == 2 || now_object == 3) x_radian_stack_2 -= 1.0f;
    }
    else if (isy == 0) {
        if (now_object == 1 || now_object == 3) y_radian_stack_1 += 1.0f;
        if (now_object == 2 || now_object == 3) y_radian_stack_2 += 1.0f;
    }
    else if (isy == 1) {
        if (now_object == 1 || now_object == 3) y_radian_stack_1 -= 1.0f;
        if (now_object == 2 || now_object == 3) y_radian_stack_2 -= 1.0f;
    }
    else if (isr == 0) {
        if (now_object == 1 || now_object == 3) r_radian_stack_1 += 1.0f;
        if (now_object == 2 || now_object == 3) r_radian_stack_2 += 1.0f;
    }
    else if (isr == 1) {
        if (now_object == 1 || now_object == 3) r_radian_stack_1 -= 1.0f;
        if (now_object == 2 || now_object == 3) r_radian_stack_2 -= 1.0f;
    }
    else if (isa == 0) {
        if (now_object == 1 || now_object == 3) a_size_stack_1 += 0.005f;
        if (now_object == 2 || now_object == 3) a_size_stack_2 += 0.005f;
    }
    else if (isa == 1) {
        if (now_object == 1 || now_object == 3) a_size_stack_1 -= 0.005f;
        if (now_object == 2 || now_object == 3) a_size_stack_2 -= 0.005f;
    }
    else if (isb == 0) {
        if (now_object == 1 || now_object == 3) b_size_stack_1 += 0.005f;
        if (now_object == 2 || now_object == 3) b_size_stack_2 += 0.005f;
    }
    else if (isb == 1) {
        if (now_object == 1 || now_object == 3) b_size_stack_1 -= 0.005f;
        if (now_object == 2 || now_object == 3) b_size_stack_2 -= 0.005f;
    }
    else if (isd == 0) {
        if (now_object == 1 || now_object == 3) d_length_stack_1 -= 0.005f;
        if (now_object == 2 || now_object == 3) d_length_stack_2 -= 0.005f;
    }
    else if (isd == 1) {
        if (now_object == 1 || now_object == 3) d_length_stack_1 += 0.005f;
        if (now_object == 2 || now_object == 3) d_length_stack_2 += 0.005f;
    }
    else if (ise == 0) {
        if (now_object == 1 || now_object == 3) e_length_stack_1 += 0.005f;
        if (now_object == 2 || now_object == 3) e_length_stack_2 += 0.005f;
    }
    else if (ise == 1) {
        if (now_object == 1 || now_object == 3) e_length_stack_1 -= 0.005f;
        if (now_object == 2 || now_object == 3) e_length_stack_2 -= 0.005f;
    }
    else if (isv == true) {
        v_size_stack_1 += 0.002f;
        v_j_radian_stack_1 += 1.0f;
        v_g_radian_stack_1 += 1.0f;
        v_size_stack_2 -= 0.002f;
        v_j_radian_stack_2 += 1.0f;
        v_g_radian_stack_2 += 1.0f;
    }
    else if (ist == 0) {
        const float base_speed = 0.05f;
        static int t_cnt = 0;
        t_cnt = (t_cnt + 1) % 2;

        bool obj1_done = MoveObjectToTarget(&d_length_stack_1, &e_length_stack_1, 1.0f, 0.0f, b_size_stack_1, base_speed);
        bool obj2_done = MoveObjectToTarget(&d_length_stack_2, &e_length_stack_2, -1.0f, 0.0f, b_size_stack_2, base_speed);

        if (obj1_done && obj2_done) {
            ist = 1;
        }
    }
    else if (ist == 1) {
        const float base_speed = 0.05f;

        bool obj1_done = MoveObjectToTarget(&d_length_stack_1, &e_length_stack_1, t_final_d_1, t_final_e_1, b_size_stack_1, base_speed);
        bool obj2_done = MoveObjectToTarget(&d_length_stack_2, &e_length_stack_2, t_final_d_2, t_final_e_2, b_size_stack_2, base_speed);

        if (obj1_done && obj2_done) {
            ist = -1;
        }
    }
    else if (isu == 0) {
        const float base_speed = 0.05f;

        bool obj1_done = MoveObjectToTarget(&d_length_stack_1, &e_length_stack_1, 1.0f, 1.0f, b_size_stack_1, base_speed);
        bool obj2_done = MoveObjectToTarget(&d_length_stack_2, &e_length_stack_2, -1.0f, -1.0f, b_size_stack_2, base_speed);

        if (obj1_done && obj2_done) {
            isu = 1;
        }
    }
    else if (isu == 1) {
        const float base_speed = 0.05f;

        bool obj1_done = MoveObjectToTarget(&d_length_stack_1, &e_length_stack_1, u_final_d_1, u_final_e_1, b_size_stack_1, base_speed);
        bool obj2_done = MoveObjectToTarget(&d_length_stack_2, &e_length_stack_2, u_final_d_2, u_final_e_2, b_size_stack_2, base_speed);

        if (obj1_done && obj2_done) {
            isu = -1;
        }
    }

    glutPostRedisplay(); // 다시 그리기 요청
    glutTimerFunc(1000 / 60, Timer, 1); //--- 타이머 콜백함수 지정 (60 FPS)
}

GLvoid Mouse(int button, int state, int x, int y) {

}

bool move_towards(float* current, float target, float speed) {
    if (*current < target) {
        *current += speed;
        if (*current >= target) {
            *current = target;
            return true;
        }
    }
    else if (*current > target) {
        *current -= speed;
        if (*current <= target) {
            *current = target;
            return true;
        }
    }

    return (*current == target);
}

bool MoveObjectToTarget(float* current_d, float* current_e, float target_d, float target_e, float b_size_stack, float base_speed)
{
    float speed = base_speed / (b_size_stack + 0.0001f);
    bool d_done = move_towards(current_d, target_d, speed);
    bool e_done = move_towards(current_e, target_e, speed);
    return d_done && e_done;
}

glm::vec3 GetWorldPos(int object_id) {
    if (object_id == 1) { // 왼쪽 객체 (ID: 1)
        float r = glm::radians(r_radian_stack_1);
        float d = d_length_stack_1;
        float e = e_length_stack_1;
        float b = b_size_stack_1;
        // Timer 함수에서 변환하는 순서 그대로 계산
        float x = ((-1.0f + d) * cos(r)) * b;
        float y = e * b;
        float z = ((-1.0f + d) * sin(r)) * b;
        return glm::vec3(x, y, z);
    }
    else { // 오른쪽 객체 (ID: 2)
        float r = glm::radians(r_radian_stack_2);
        float d = d_length_stack_2;
        float e = e_length_stack_2;
        float b = b_size_stack_2;
        // Timer 함수에서 변환하는 순서 그대로 계산
        float x = ((1.0f + d) * cos(r)) * b;
        float y = e * b;
        float z = ((1.0f + d) * sin(r)) * b;
        return glm::vec3(x, y, z);
    }
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

void InitAxisBuffer()
{
    // X, Y, Z 축을 나타내는 6개의 정점 데이터. 각 정점은 (x, y, z, r, g, b) 형식입니다.
    GLfloat axis_vertices[] = {
        // X-axis (Red)
        -1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         // Y-axis (Green)
          0.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
          0.0f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
          // Z-axis (Blue)
           0.0f, 0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
           0.0f, 0.0f,  1.0f,  0.0f, 0.0f, 1.0f
    };

    glGenVertexArrays(1, &axis_vao);
    glGenBuffers(1, &axis_vbo);

    glBindVertexArray(axis_vao);
    glBindBuffer(GL_ARRAY_BUFFER, axis_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis_vertices), axis_vertices, GL_STATIC_DRAW);

    // 위치 속성 (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 색상 속성 (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

struct Vertex {
    float x, y, z;
};

struct Face {
    unsigned int v1, v2, v3;
};

struct Model {
    Vertex* vertices;
    size_t vertex_count;
    Face* faces;
    size_t face_count;
};
std::vector<Model> models;
Model read_obj_file(const char* filename);

void read_newline(char* str) {
    char* pos;
    if ((pos = strchr(str, '\n')) != NULL)
        *pos = '\0';
}

Model read_obj_file(const char* filename) {
    Model model;
    FILE* file;
    fopen_s(&file, filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    char line[MAX_LINE_LENGTH];
    model.vertex_count = 0;
    model.face_count = 0;

    while (fgets(line, sizeof(line), file)) {
        read_newline(line);
        if (line[0] == 'v' && line[1] == ' ')
            model.vertex_count++;
        else if (line[0] == 'f' && line[1] == ' ')
            model.face_count++;
    }

    fseek(file, 0, SEEK_SET);
    model.vertices = (Vertex*)malloc(model.vertex_count * sizeof(Vertex));
    model.faces = (Face*)malloc(model.face_count * sizeof(Face));
    size_t vertex_index = 0; size_t face_index = 0;
    while (fgets(line, sizeof(line), file)) {
        read_newline(line);
        if (line[0] == 'v' && line[1] == ' ') {
            int result = sscanf_s(line + 2, "%f %f %f",
                &model.vertices[vertex_index].x,
                &model.vertices[vertex_index].y,
                &model.vertices[vertex_index].z);
            vertex_index++;
        }
        else if (line[0] == 'f' && line[1] == ' ') {
            unsigned int v1, v2, v3;
            int result = sscanf_s(line + 2, "%u %u %u", &v1, &v2, &v3);
            model.faces[face_index].v1 = v1 - 1; // OBJ indices start at 1
            model.faces[face_index].v2 = v2 - 1;
            model.faces[face_index].v3 = v3 - 1;
            face_index++;
        }
    }
    fclose(file);
    return model;
}

void LoadOBJ(const char* filename, int object_num)
{
    models.clear();
    models.push_back(read_obj_file(filename));

    float r, g, b;
    static int rdcnt = 0;

    for (auto& m : models) {
        for (size_t i = 0; i < m.face_count; i++) {
            Face f = m.faces[i];
            Vertex v1 = m.vertices[f.v1];
            Vertex v2 = m.vertices[f.v2];
            Vertex v3 = m.vertices[f.v3];

            // 각 삼각형의 세 정점 추가 (RGB 랜덤)
            if (rdcnt % 2 == 0) {
                r = rdcolor(mt);
                g = rdcolor(mt);
                b = rdcolor(mt);
            }
            rdcnt++;

            SHAPE shape;
            shape.object_num = object_num;
            shape.face_count = i;
            shape.vertex.insert(shape.vertex.end(), {
                v1.x, v1.y, v1.z, r, g, b,  // v1 데이터
                v2.x, v2.y, v2.z, r, g, b,  // v2 데이터
                v3.x, v3.y, v3.z, r, g, b   // v3 데이터
                });
            shapes.push_back(shape);
        }
    }
    UpdateBuffer();
}