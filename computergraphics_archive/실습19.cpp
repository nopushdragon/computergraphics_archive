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
bool axis_display = true;
bool depth_on = true;
bool proj_on = true;
bool solid_wire = false;

float x_stack = 0.0f;
float y_stack = 0.0f;
float x_cam = 3.0f;
float y_cam = 3.0f;
float z_cam = 3.0f;
float x_at = 0.0f;
float y_at = 0.0f;
float z_at = 0.0f;
float earth_move = 1.0f;
float moon_move = 0.3f;
int isz = 0;

float angle = 1.0f;
float global_z_angle = 0.0f;
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
    for(int i = 0 ; i < 7;i++) LoadOBJ("sphere.obj", i);
    for(int i = 0; i<6;i++) LoadOBJ("ring.obj", i+7);

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

    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramID);

    if (axis_display) draw_axis();

    glm::mat4 view = glm::lookAt(
        glm::vec3(x_cam, y_cam, z_cam),
        glm::vec3(x_at, y_at, z_at),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 projection;
    if (proj_on) {
        projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    }
    else {
        float aspect = (float)width / (float)height;
        float ortho_size = 3.0f;
        projection = glm::ortho(-ortho_size * aspect, ortho_size * aspect, -ortho_size, ortho_size, 0.1f, 100.0f);
    }

    glm::mat4 global_z_rotation = glm::rotate(glm::mat4(1.0f), glm::radians(global_z_angle), glm::vec3(0.0f, 0.0f, 1.0f));

    GLuint modelLoc = glGetUniformLocation(shaderProgramID, "uModel");
    GLuint viewLoc = glGetUniformLocation(shaderProgramID, "uView");
    GLuint projLoc = glGetUniformLocation(shaderProgramID, "uProj");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

    if (viewLoc == -1 || projLoc == -1) {
        std::cerr << "ERROR: View or Proj uniform location not found!" << std::endl;
    }

    glBindVertexArray(vao);
    GLint first = 0;
    for (int i = 0; i < shapes.size(); i++) {
        glm::mat4 model = shapes[i].model;

        glm::mat4 final_model = global_z_rotation * model;

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &final_model[0][0]);

        if (modelLoc == -1) {
            std::cerr << "ERROR: Model uniform location not found!" << std::endl;
        }

        int vertexCount = shapes[i].vertex.size() / 6;
        if (!solid_wire) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glLineWidth(1.0f);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(0.5f);
        }
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

GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'p':
        proj_on = false;
        break;
    case 'P':
        proj_on = true;
        break;
    case 'm':
        solid_wire = false;
        break;
    case 'M':
        solid_wire = true;
        break;
    case 'h':
        depth_on = !depth_on;
        break;
    case 'w':
        y_cam -= 0.1f;
        y_at -= 0.1f;
        break;
    case 's':
        y_cam += 0.1f;
        y_at += 0.1f;
        break;
    case 'a':
        x_cam += 0.1f;
        x_at += 0.1f;
        break;
    case 'd':
        x_cam -= 0.1f;
        x_at-= 0.1f;
        break;
    case '=':
        z_cam -= 0.1f;
        z_at -= 0.1f;
        break;
    case '-':
        z_cam += 0.1f;
        z_at += 0.1f;
        break;
    case 'y':
        earth_move += 0.1f;
        moon_move += 0.05f;
        break;
    case 'Y':
        earth_move -= 0.1f;
        moon_move -= 0.05f;
        break;
    case 'z':
        if (isz == 1) isz = 0;
        else isz = 1;
        break;
    case 'Z':
        if (isz == -1) isz = 0;
        else isz = -1;
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

GLvoid Timer(int value) //--- 콜백 함수: 타이머 콜백 함수
{
    angle += 1.0f; // 공통 각도 증가
    if(isz == 1) global_z_angle += 0.5f;
    else if (isz == -1) global_z_angle -= 0.5f;

    for (int i = 0; i < shapes.size(); i++) {
        shapes[i].model = glm::mat4(1.0f); // 매번 모델 행렬 초기화

        if (shapes[i].object_num == 0) {
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.5f));
        }
        else if (shapes[i].object_num == 1) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, 0.0f, earth_move));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.2f));
        }
        else if (shapes[i].object_num == 2) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(angle), glm::vec3(-1.0f, 1.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, 0.0f, earth_move));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.2f));
        }
        else if (shapes[i].object_num == 3) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(angle), glm::vec3(1.0f, 1.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, 0.0f, earth_move));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.2f));
        }
        else if (shapes[i].object_num == 4) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, 0.0f, earth_move));

            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(angle * 2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, 0.0f, moon_move));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.1f));
        }
        else if (shapes[i].object_num == 5) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(angle), glm::vec3(-1.0f, 1.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, 0.0f, earth_move));

            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(angle * 5.0f), glm::vec3(-1.0f, 1.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, 0.0f, moon_move));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.1f));
        }
        else if (shapes[i].object_num == 6) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(angle), glm::vec3(1.0f, 1.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, 0.0f, earth_move));

            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(angle * 10.0f), glm::vec3(1.0f, 1.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, 0.0f, moon_move));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.1f));
        }
        else if (shapes[i].object_num == 7) {
            float ring_scale = earth_move;
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(ring_scale));
        }
        else if (shapes[i].object_num == 8) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, -1.0f));
            float ring_scale = earth_move;
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(ring_scale));
        }
        else if (shapes[i].object_num == 9) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            float ring_scale = earth_move;
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(ring_scale));
        }
        else if (shapes[i].object_num == 10) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, 0.0f, earth_move));
            float ring_scale = moon_move;
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(ring_scale));
        }
        else if (shapes[i].object_num == 11) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(angle), glm::vec3(1.0f, 1.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, 0.0f, earth_move));
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, -1.0f));
            float ring_scale = moon_move;
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(ring_scale));
        }
        else if (shapes[i].object_num == 12) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(angle), glm::vec3(-1.0f, 1.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, 0.0f, earth_move));
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            float ring_scale = moon_move;
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(ring_scale));
        }
    }

    glutPostRedisplay(); // 다시 그리기 요청
    glutTimerFunc(1000 / 60, Timer, 1); //--- 타이머 콜백함수 지정 (60 FPS)
}

GLvoid Mouse(int button, int state, int x, int y) {

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

    if (object_num == 0){
        r = 0.8f; g = 0.3f; b = 0.2f;
    }
    else if (object_num >= 1 && object_num < 4) {
        r = 0.2f; g = 0.8f; b = 0.6f;
    }
    else if (object_num >= 4 && object_num < 7) {
        r = 0.2f; g = 0.25f; b = 0.8f;
    }
    else {
        r = 0.5f; g = 0.5f; b = 0.5f;
    }

    for (auto& m : models) {
        for (size_t i = 0; i < m.face_count; i++) {
            Face f = m.faces[i];
            Vertex v1 = m.vertices[f.v1];
            Vertex v2 = m.vertices[f.v2];
            Vertex v3 = m.vertices[f.v3];

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