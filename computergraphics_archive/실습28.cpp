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
std::uniform_real_distribution<float> rdpos(-8.0f, 8.0f);
std::uniform_real_distribution<float> rdpos_y(10.0f, 12.0f);
std::uniform_real_distribution<float> rdspeed(0.01f, 0.05f);


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

float x_cam = 0.0f;
float y_cam = 10.0f;
float z_cam = 15.0f;
float x_at = 0.0f;
float y_at = 0.0f;
float z_at = 0.0f;

float light_radian_stack = 0.0f;
float light_x = 0.0f;
float light_y = 5.0f;
float light_z = 5.0f;
float light_power = 0.5f;
bool isy = false; 
bool iss = false;

float radian_stack = 0.0f;
//

struct LIGHT {
    glm::vec4 pos;
    glm::mat4 model = glm::mat4(1.0f);
    float r = 1.0f, g = 1.0f, b = 1.0f;
}light;

struct OBB {
    glm::vec3 center = glm::vec3(0.0f);     // OBB의 중심 (월드 좌표계)
    glm::vec3 u[3];                         // OBB의 세 정규직교 축 (u[0]=x축, u[1]=y축, u[2]=z축)
    glm::vec3 half_length = glm::vec3(0.0f); // 각 축을 따른 중심으로부터의 반치수 (Local Space)
};

std::vector<GLfloat> allVertices;
struct SHAPE {
    std::vector<GLfloat> vertex;
    glm::mat4 model = glm::mat4(1.0f);
    int face_count;
    int object_num;
    glm::vec3 reset = glm::vec3(0.0f, 0.0f, 0.0f);
    float speed = rdspeed(mt);

    // OBB 관련 추가 항목
    OBB local_obb;  // 모델링 시점의 로컬 OBB (Model Matrix가 Identity일 때)
    OBB world_obb;  // 현재 프레임의 월드 OBB (Model Matrix 적용 후)
    bool is_colliding = false; // 충돌 상태
};
std::vector<SHAPE> shapes;

GLvoid MouseMove(int x, int y);
void update_world_obb(SHAPE& shape);
bool check_obb_collision(const SHAPE& shapeA, const SHAPE& shapeB);
bool is_separated(const OBB& a, const OBB& b, const glm::vec3& axis);

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
    LoadOBJ("n_plane.obj", 0);
    LoadOBJ("n_tank.obj", 1);
    LoadOBJ("n_cube.obj", 2);

    LoadOBJ("n_cube.obj", 3);
    LoadOBJ("n_cube.obj", 4);
    LoadOBJ("n_cube.obj", 5);

    for (int i = 0; i < 50;i++) {
        LoadOBJ("n_cube.obj", 6+i);
    }

    glutDisplayFunc(drawScene); //--- 출력 콜백 함수
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);
    glutMotionFunc(MouseMove);
    glutTimerFunc(1000 / 60, Timer, 1); //--- 타이머 콜백함수 지정 (60 FPS)

    glutMainLoop();
}

void make_vertexShaders()
{
    GLchar* vertexSource;

    //--- 버텍스 세이더 읽어 저장하고 컴파일 하기
    //--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수

    vertexSource = filetobuf("vertex_light.glsl");
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
    fragmentSource = filetobuf("fragment_light_28.glsl"); // 프래그세이더 읽어오기
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

GLvoid drawScene() {
    if (depth_on)
        glEnable(GL_DEPTH_TEST); // 은면제거
    else
        glDisable(GL_DEPTH_TEST);

    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramID);

    unsigned int lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos"); //--- lightPos 값 전달
    glUniform3f(lightPosLocation, light.pos.x, light.pos.y, light.pos.z);
    unsigned int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor"); //--- lightColor 값 전달
    glUniform3f(lightColorLocation, light.r, light.g, light.b);
    unsigned int ambientLightLocation = glGetUniformLocation(shaderProgramID, "ambientLight");
    glUniform1f(ambientLightLocation, light_power);
    unsigned int viewPosLocation = glGetUniformLocation(shaderProgramID, "viewPos"); //--- viewPos 값 전달
    glUniform3f(viewPosLocation, x_cam, y_cam, z_cam);


    if (axis_display) draw_axis();  //좌표축 그리기

    glm::mat4 view = glm::lookAt(
        glm::vec3(x_cam, y_cam, z_cam),
        glm::vec3(x_at, y_at, z_at),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

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

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
        if (modelLoc == -1) {
            std::cerr << "ERROR: Model uniform location not found!" << std::endl;
        }

        // 노멀 행렬 계산: Model 행렬의 역전치 행렬 (3x3)
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
        // 노멀 행렬 유니폼 위치 찾기 및 전달
        GLuint normalMatLoc = glGetUniformLocation(shaderProgramID, "uNormalMatrix");
        if (normalMatLoc != -1) {
            glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, &normalMatrix[0][0]);
        }

        int vertexCount = shapes[i].vertex.size() / 9;
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
    case 's':
        iss = !iss;
        for (int i = 6;i < shapes.size();i++) {
            shapes[i].reset = glm::vec3(rdpos(mt), rdpos_y(mt), rdpos(mt));
        }
        break;
    case'y':
        isy =!isy;
        break;
    case 'n':
        light_y -= 0.5f;
        light_z -= 0.5f;
        break;
    case 'f':
        light_y += 0.5f;
        light_z += 0.5f;
        break;
    case '=':
        light_power += 0.1f;
        if (light_power >= 1.0f) light_power = 1.0f;
        break;
    case '-':
        light_power -= 0.1f;
        if (light_power <= 0.0f)light_power = 0.0f;
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
    //====================================================================
    //선언    

    //====================================================================
    // 충돌처리

    //====================================================================
    // 변수 업데이트
    if (iss) {
        for (int i = 6;i < shapes.size();i++) {
            shapes[i].reset.y -= shapes[i].speed;
            //if (check_obb_collision(shapes[i], shapes[0])) {
            //    shapes[i].reset.y = rdpos_y(mt);
            //}
            if (shapes[i].reset.y <= -0.5f) {
                shapes[i].reset.y = rdpos_y(mt);
            }
        }
    }
    if (isy) light_radian_stack += 2.0f;
    radian_stack += 2.0f;
    //====================================================================
    //모델 행렬 업데이트
    for (int i = 0; i < shapes.size(); i++) {
        shapes[i].model = glm::mat4(1.0f);

        if (shapes[i].object_num == 0) {
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, -0.5f, 0.0f));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(15.0f, 0.1f, 15.0f));
        }
        else if (shapes[i].object_num == 1) {
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.5f));
        }
        else if (shapes[i].object_num == 2) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(light_radian_stack), glm::vec3(0.0f, 1.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(light_x, light_y, light_z));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.2f));
        }
        else if (shapes[i].object_num == 3) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(radian_stack), glm::vec3(0.0f, 1.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, 0.0f, 4.0f));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.7f));
        }
        else if (shapes[i].object_num == 4) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(radian_stack), glm::vec3(-1.0f, 1.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, 0.0f, 4.0f));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.5f));
        }
        else if (shapes[i].object_num == 5) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(radian_stack), glm::vec3(1.0f, 1.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, 0.0f, 4.0f));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.3f));
        }
        else {
            shapes[i].model = glm::translate(shapes[i].model, shapes[i].reset);
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.05f));
        }

        update_world_obb(shapes[i]);
    }
    //====================================================================
    // 조명 업데이트
    light.pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::mat4 light_model = glm::mat4(1.0f);
    light_model = glm::rotate(light_model, glm::radians(light_radian_stack), glm::vec3(0.0f, 1.0f, 0.0f));
    light_model = glm::translate(light_model, glm::vec3(light_x, light_y, light_z));
    light.pos = light_model * light.pos;

    glutPostRedisplay(); // 다시 그리기 요청
    glutTimerFunc(1000 / 60, Timer, 1); //--- 타이머 콜백함수 지정 (60 FPS)
    std::cout << light_power << std::endl;
}

GLvoid Mouse(int button, int state, int x, int y) {
}

GLvoid MouseMove(int x, int y) {
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // --- 노멀벡터 속성 (location = 1, vec3)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // --- 색상 속성 (location = 1, vec3)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    glUseProgram(shaderProgramID);
}

void UpdateBuffer()
{
    allVertices.clear();

    for (int i = 0; i < shapes.size(); i++)	allVertices.insert(allVertices.end(), shapes[i].vertex.begin(), shapes[i].vertex.end());

    // 합쳐진 데이터로 VBO 업데이트
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(GLfloat), allVertices.data(), GL_DYNAMIC_DRAW);
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

struct Normal {
    float x, y, z;
};

struct Face {
    // OBJ 파일은 v/vt/vn 순서로 인덱스를 가집니다.
    // 여기서는 텍스처 좌표(vt)는 무시하고, 위치(v)와 노멀(vn) 인덱스만 사용합니다.
    unsigned int v_idx[3];  // 정점 위치 인덱스 (v1, v2, v3)
    unsigned int vn_idx[3]; // 정점 노멀 인덱스 (vn1, vn2, vn3)
};

struct Model {
    std::vector<Vertex> vertices;       // v
    std::vector<Normal> normals;        // vn
    std::vector<Face> faces;            // f v/vt/vn v/vt/vn v/vt/vn
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
    // fopen_s는 Visual Studio 전용이므로, 이식성을 위해 fopen을 사용하고
    // 안전한 코드를 위해 에러 처리를 추가합니다.
    file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    char line[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), file)) {
        read_newline(line);

        if (strncmp(line, "v ", 2) == 0) {
            float x, y, z;
            if (sscanf(line + 2, "%f %f %f", &x, &y, &z) == 3) {
                model.vertices.push_back({ x, y, z });
            }
        }
        else if (strncmp(line, "vn ", 3) == 0) {
            float x, y, z;
            if (sscanf(line + 3, "%f %f %f", &x, &y, &z) == 3) {
                model.normals.push_back({ x, y, z });
            }
        }
        else if (strncmp(line, "f ", 2) == 0) {
            Face face;
            // OBJ 면 정의 파싱: v/vt/vn v/vt/vn v/vt/vn
            // 텍스처 좌표(vt)는 %*u 로 무시하고, 위치(v)와 노멀(vn) 인덱스만 추출
            // %u/%*u/%u 형태의 파싱을 수행하며, 인덱스는 1부터 시작하므로 -1을 해줘야 합니다.
            int result = sscanf(line + 2, "%u/%*u/%u %u/%*u/%u %u/%*u/%u",
                &face.v_idx[0], &face.vn_idx[0],
                &face.v_idx[1], &face.vn_idx[1],
                &face.v_idx[2], &face.vn_idx[2]);

            // 정점 인덱스는 1부터 시작하므로 0-base 인덱스로 변환
            if (result == 6) {
                for (int i = 0; i < 3; i++) {
                    face.v_idx[i] -= 1;
                    face.vn_idx[i] -= 1;
                }
                model.faces.push_back(face);
            }
        }
    }
    fclose(file);
    return model;
}

void LoadOBJ(const char* filename, int object_num)
{
    models.clear();
    Model m = read_obj_file(filename); // OBJ 파일을 읽어서 Model 구조체에 저장
    models.push_back(m); // 전역 models vector에 저장

    // --- OBB 계산을 위한 로컬 좌표계의 최소/최대값 초기화
    glm::vec3 min_v = glm::vec3(FLT_MAX);
    glm::vec3 max_v = glm::vec3(-FLT_MAX);

    static float r, g, b;
    r = rdcolor(mt); g = rdcolor(mt); b = rdcolor(mt);
    if (object_num == 0) {
        r = 0.2f;
        g = 1.0f;
        b = 0.2f;
    }
    else if (object_num == 2) {
        r = 1.0f;
        g = 1.0f;
        b = 1.0f;
    }
    else if (object_num >= 6) {
        r = 1.0f;
        g = 1.0f;
        b = 1.0f;
    }

    SHAPE object_shape;
    object_shape.object_num = object_num;

    // --- 정점 데이터 합치기: 위치(3), 노멀(3), 색상(3) -> 총 9 float ---
    for (auto& m : models) { // models vector를 순회하지만 실제로는 방금 로드한 1개의 모델만 존재
        for (const auto& f : m.faces) {
            // 각 면(삼각형)의 3개 정점을 순회
            for (int i = 0; i < 3; i++) {
                const Vertex& v = m.vertices[f.v_idx[i]];
                const Normal& n = m.normals[f.vn_idx[i]];

                // AABB 계산 (모든 정점을 대상으로) - 기존 코드 유지
                // OBJ 파일의 정점 인덱스 f.v_idx[i]를 사용하여 위치 데이터 가져옴
                // AABB 계산 코드가 빠져 있으므로, 다시 추가합니다.
                glm::vec3 current_v = glm::vec3(v.x, v.y, v.z);
                min_v.x = glm::min(min_v.x, current_v.x);
                min_v.y = glm::min(min_v.y, current_v.y);
                min_v.z = glm::min(min_v.z, current_v.z);
                max_v.x = glm::max(max_v.x, current_v.x);
                max_v.y = glm::max(max_v.y, current_v.y);
                max_v.z = glm::max(max_v.z, current_v.z);

                // VBO에 들어갈 데이터 (위치(3), 노멀(3), 색상(3) 순서)
                object_shape.vertex.insert(object_shape.vertex.end(), {
                    v.x, v.y, v.z,      // 위치 (vPos, location=0)
                    n.x, n.y, n.z,      // 노멀 (vNormal, location=1)
                    r, g, b             // 색상 (aColor, location=2)
                    });
            }
            object_shape.face_count++; // 면 개수 증가
        }
    }

    // 2. **[OBB 설정]**
    glm::vec3 local_center = (min_v + max_v) * 0.5f;
    glm::vec3 local_half_length = (max_v - min_v) * 0.5f;

    object_shape.local_obb.center = local_center;
    object_shape.local_obb.half_length = local_half_length; // OBJ 파일의 로컬 Half-Length (큐브/구체는 약 1.0)
    object_shape.local_obb.u[0] = glm::vec3(1.0f, 0.0f, 0.0f);
    object_shape.local_obb.u[1] = glm::vec3(0.0f, 1.0f, 0.0f);
    object_shape.local_obb.u[2] = glm::vec3(0.0f, 0.0f, 1.0f);

    shapes.push_back(object_shape);


    UpdateBuffer();
}

void update_world_obb(SHAPE& shape) {
    // 모델 행렬의 회전/스케일 부분 (3x3) 추출
    glm::mat3 rotation_scale_mat = glm::mat3(shape.model);

    // 1. 월드 중심 변환
    glm::vec4 local_center_h = glm::vec4(shape.local_obb.center, 1.0f);
    shape.world_obb.center = glm::vec3(shape.model * local_center_h);

    // 2. 월드 축 변환
    for (int i = 0; i < 3; i++) {
        shape.world_obb.u[i] = glm::normalize(rotation_scale_mat * shape.local_obb.u[i]);
    }

    // 3. **[핵심 수정]** 반치수 설정: 스케일 벡터를 추출하여 local_obb에 적용
    // (모델 행렬의 각 축 길이가 스케일 팩터를 포함함)
    glm::vec3 scale_factors = glm::vec3(
        glm::length(rotation_scale_mat[0]), // X축 스케일
        glm::length(rotation_scale_mat[1]), // Y축 스케일
        glm::length(rotation_scale_mat[2])  // Z축 스케일
    );

    // 로컬 반치수에 스케일 팩터를 곱하여 월드 반치수 설정
    // (주의: 여기서는 비균일 스케일을 처리하기 위해 각 축의 스케일을 개별적으로 곱합니다.)
    shape.world_obb.half_length = shape.local_obb.half_length * scale_factors;
}

bool is_separated(const OBB& a, const OBB& b, const glm::vec3& axis) {
    // 축의 길이가 너무 작으면 무시 (부동소수점 오차 처리)
    if (glm::length(axis) < 1e-6) return false;

    // 두 OBB 중심 사이의 벡터
    glm::vec3 T = b.center - a.center;

    // 1. 중심 간 거리 투영
    float distance_proj = glm::abs(glm::dot(T, axis));

    // 2. OBB A의 '반지름' 투영 (각 로컬 축의 투영 길이의 합)
    float radius_a =
        glm::abs(glm::dot(a.half_length.x * a.u[0], axis)) +
        glm::abs(glm::dot(a.half_length.y * a.u[1], axis)) +
        glm::abs(glm::dot(a.half_length.z * a.u[2], axis));

    // 3. OBB B의 '반지름' 투영
    float radius_b =
        glm::abs(glm::dot(b.half_length.x * b.u[0], axis)) +
        glm::abs(glm::dot(b.half_length.y * b.u[1], axis)) +
        glm::abs(glm::dot(b.half_length.z * b.u[2], axis));

    // 중심 간 거리가 두 반지름의 합보다 크면 분리 (충돌 아님)
    return distance_proj > (radius_a + radius_b);
}

bool check_obb_collision(const SHAPE& shapeA, const SHAPE& shapeB) {
    const OBB& a = shapeA.world_obb;
    const OBB& b = shapeB.world_obb;

    // 1. OBB A의 축 3개 검사
    for (int i = 0; i < 3; i++) {
        if (is_separated(a, b, a.u[i])) return false;
    }

    // 2. OBB B의 축 3개 검사
    for (int i = 0; i < 3; i++) {
        if (is_separated(a, b, b.u[i])) return false;
    }

    // 3. 교차 축 (A_i x B_j) 9개 검사
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            glm::vec3 cross_axis = glm::cross(a.u[i], b.u[j]);
            if (is_separated(a, b, cross_axis)) return false;
        }
    }

    // 15개 축 모두에서 분리되지 않았다면 충돌!
    return true;
}