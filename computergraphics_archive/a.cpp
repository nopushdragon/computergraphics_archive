#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#define MAX_LINE_LENGTH 256

#define BOX_SIZE  1.0f
#define CUBE_SIZE_1 0.1f
#define CUBE_SIZE_2 0.2f
#define CUBE_SIZE_3 0.3f
#define SPHERE_SIZE 0.2f


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
float y_cam = 0.0f;
float z_cam = 4.0f;
float x_at = 0.0f;
float y_at = 0.0f;
float z_at = 0.0f;

float y_rotate_stack = 0.0f;
bool open = false;

bool is_rotating = true;
float last_mouse_x;
float rotation_angle = 0.0f;
float cube_rotate_angle = 0.0f;
int sphere_cnt = 1;
//

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

    for (int i = 0; i < 5;i++) LoadOBJ("plane.obj", i);
    for (int i = 0; i < 3;i++) LoadOBJ("15_cube.obj", i + 5);
    for (int i = 0; i < 5;i++) LoadOBJ("middle_size_sphere.obj", i + 8);

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
        if (shapes[i].object_num <= 7 + sphere_cnt) {
            glm::mat4 model = shapes[i].model;

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

            if (modelLoc == -1) {
                std::cerr << "ERROR: Model uniform location not found!" << std::endl;
            }

            int vertexCount = shapes[i].vertex.size() / 6;
            glDrawArrays(GL_TRIANGLES, first, vertexCount);
            first += vertexCount;
        }
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
    case 'z':
        z_cam += 0.1f;
        break;
    case 'Z':
        z_cam -= 0.1f;
        break;
    case 'y':
        y_rotate_stack += 5.0f;
        break;
    case 'Y':
        y_rotate_stack -= 5.0f;
        break;
    case 'o':
        open = !open;
        break;
    case 'b':
        sphere_cnt++;
        break;
    case 'h':
        depth_on = !depth_on;
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
    static float open_ratoate = 0.0f;
    if (open) {
        if (open_ratoate < 90.0f) open_ratoate += 5.0f;
    }
    else {
        if (open_ratoate > 0) open_ratoate -= 5.0f;
    }

    // --- 큐브들의 하강 상태를 저장하는 변수 (static으로 한 번만 초기화됨)
    // object_num 1, 2, 3 순서대로 cube_y_offset[0], [1], [2]에 대응
    static float cube_y_offset[3] = { 0.0f, 0.0f, 0.0f };
    static float cube_fall_speed = 0.005f; // 떨어지는 속도 조절
    static float cube_x_offset[3] = { 0.0f, 0.0f, 0.0f };

    //if (cube_rotate_angle >= 90.0f) {
    //    cube_rotate_angle -= 90.0f;
    //    cube_x_offset[0] = BOX_SIZE - CUBE_SIZE_1;
    //    cube_x_offset[1] = BOX_SIZE - CUBE_SIZE_2;
    //    cube_x_offset[2] = BOX_SIZE - CUBE_SIZE_3;
    //}
    //else if (cube_rotate_angle <= -90.0f) {
    //    cube_rotate_angle += 90.0f;
    //    cube_x_offset[0] = -(BOX_SIZE - CUBE_SIZE_1);
    //    cube_x_offset[1] = -(BOX_SIZE - CUBE_SIZE_2);
    //    cube_x_offset[2] = -(BOX_SIZE - CUBE_SIZE_3);
    //}

    static float sphere_y_offset[5] = { 0.1f,0.0f,0.0f,0.0f,0.3f };
    static float sphere_x_offset[5] = { 0.0f,0.3f,0.2f,0.0f,0.0f };
    static bool sphere_falling[5] = { true, false, true, false, true };
    static bool sphere_moving_right[5] = { false, true, false, false, true };

    // 1. 충돌 상태 초기화
    for (size_t i = 0; i < shapes.size(); i++) {
        shapes[i].is_colliding = false;
    }

    // 박스 벽 인덱스: 0~4
    const int WALL_START_INDEX = 0;
    const int WALL_END_INDEX = 4;
    // 큐브 인덱스: 5, 6, 7
    const int CUBE_START_INDEX = 5;
    const int CUBE_END_INDEX = 7;
    // 구체 인덱스: 8부터
    const int SPHERE_START_INDEX = 8;

    const float GRAVITY_FALL = 0.05f;
    const float GRAVITY_TILT = 0.05f;
    float tilt_angle_rad = glm::radians(rotation_angle);
    float gravity_x_accel = GRAVITY_TILT * glm::sin(tilt_angle_rad);
    //====================================================================
    // 충돌처리
    for (int i = CUBE_START_INDEX; i <= CUBE_END_INDEX; i++) { // 큐브
        for (int wall_idx = WALL_START_INDEX + 1; wall_idx <= WALL_END_INDEX; wall_idx++) { // 5개 벽

            if (check_obb_collision(shapes[i], shapes[wall_idx])) {
                shapes[i].is_colliding = true;
                shapes[wall_idx].is_colliding = true;

                std::cout << "Cube " << shapes[i].object_num << " collided with Wall " << wall_idx << std::endl;
                break;
            }
        }
    }
    //====================================================================
    //모델 행렬 업데이트
    for (int i = 0; i < shapes.size(); i++) {
        shapes[i].model = glm::mat4(1.0f);
        shapes[i].model = glm::rotate(shapes[i].model, glm::radians(y_rotate_stack), glm::vec3(0.0f, 1.0f, 0.0f));

        if (i >= WALL_START_INDEX && i <= WALL_END_INDEX) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(rotation_angle), glm::vec3(0.0f, 0.0f, 1.0f));
            if (i == 0) {
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, -BOX_SIZE, -BOX_SIZE));
                shapes[i].model = glm::rotate(shapes[i].model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, 0.0f, -BOX_SIZE));
            }
            else if (i == 1) {
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, -BOX_SIZE, 0.0f));
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(BOX_SIZE, 0.0f, 0.0f));
                shapes[i].model = glm::rotate(shapes[i].model, glm::radians(open_ratoate), glm::vec3(0.0f, 0.0f, 1.0f));
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(-BOX_SIZE, 0.0f, 0.0f));
            }
            else if (i == 2) {
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(-BOX_SIZE, -BOX_SIZE, 0.0f));
                shapes[i].model = glm::rotate(shapes[i].model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(BOX_SIZE, 0.0f, 0.0f));
            }
            else if (i == 3) {
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, BOX_SIZE, 0.0f));
            }
            else if (i == 4) {
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(BOX_SIZE, -BOX_SIZE, 0.0f));
                shapes[i].model = glm::rotate(shapes[i].model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(-BOX_SIZE, 0.0f, 0.0f));
            }

            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(BOX_SIZE * 2));
            update_world_obb(shapes[i]);
        }
        else if (i >= CUBE_START_INDEX && i <= CUBE_END_INDEX) {
            int cube_idx = i - 5;

            if (!shapes[i].is_colliding) {
                cube_y_offset[cube_idx] -= GRAVITY_FALL;
            }
            else if (shapes[i].is_colliding) {
                if(cube_x_offset[cube_idx] > -BOX_SIZE && cube_x_offset[cube_idx] < BOX_SIZE)
                    cube_x_offset[cube_idx] -= gravity_x_accel;

                if(i == 5 && cube_x_offset[cube_idx] < -(BOX_SIZE - CUBE_SIZE_1)) cube_x_offset[cube_idx] = -(BOX_SIZE - CUBE_SIZE_1);
                else if(i == 6 && cube_x_offset[cube_idx] < -(BOX_SIZE - CUBE_SIZE_2)) cube_x_offset[cube_idx] = -(BOX_SIZE - CUBE_SIZE_2);
                else if (i == 7 && cube_x_offset[cube_idx] < -(BOX_SIZE - CUBE_SIZE_3)) cube_x_offset[cube_idx] = -(BOX_SIZE - CUBE_SIZE_3);

                if (i == 5 && cube_x_offset[cube_idx] > BOX_SIZE - CUBE_SIZE_1) cube_x_offset[cube_idx] = BOX_SIZE - CUBE_SIZE_1;
                else if (i == 6 && cube_x_offset[cube_idx] > BOX_SIZE - CUBE_SIZE_2) cube_x_offset[cube_idx] = BOX_SIZE - CUBE_SIZE_2;
                else if (i == 7 && cube_x_offset[cube_idx] > BOX_SIZE - CUBE_SIZE_3) cube_x_offset[cube_idx] = BOX_SIZE - CUBE_SIZE_3;
            }

            if (shapes[i].is_colliding) shapes[i].model = glm::rotate(shapes[i].model, glm::radians(rotation_angle), glm::vec3(0.0f, 0.0f, 1.0f));
            if (shapes[i].object_num == 5) {
                shapes[i].model = glm::translate(shapes[i].model,glm::vec3(cube_x_offset[cube_idx], cube_y_offset[cube_idx], BOX_SIZE - 0.1f - CUBE_SIZE_1));
                shapes[i].model = glm::scale(shapes[i].model, glm::vec3(CUBE_SIZE_1));
            }
            else if (shapes[i].object_num == 6) {
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(cube_x_offset[cube_idx], cube_y_offset[cube_idx], BOX_SIZE - 0.1f - CUBE_SIZE_1 * 2 - CUBE_SIZE_2));
                shapes[i].model = glm::scale(shapes[i].model, glm::vec3(CUBE_SIZE_2));
            }
            else if(shapes[i].object_num == 7){
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(cube_x_offset[cube_idx], cube_y_offset[cube_idx], BOX_SIZE - 0.1f - CUBE_SIZE_1 * 2 - CUBE_SIZE_2 * 2 - CUBE_SIZE_3));
                shapes[i].model = glm::scale(shapes[i].model, glm::vec3(CUBE_SIZE_3));
            }

            update_world_obb(shapes[i]);
        }
        else if (i >= SPHERE_START_INDEX) { // 구체들 (인덱스 8부터)
            int sphere_idx = shapes[i].object_num - 8; // 구체 인덱스는 여전히 0부터 4

            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(rotation_angle), glm::vec3(0.0f, 0.0f, 1.0f));

            float sphere_base_speed = 0.003f;
            float speed_factor = (float)(sphere_idx * 0.2 + 1);

            if (sphere_falling[sphere_idx]) {
                sphere_y_offset[sphere_idx] -= sphere_base_speed * speed_factor;
            }
            else {
                sphere_y_offset[sphere_idx] += sphere_base_speed * speed_factor;
            }

            if (sphere_moving_right[sphere_idx]) {
                sphere_x_offset[sphere_idx] += sphere_base_speed * speed_factor;
            }
            else {
                sphere_x_offset[sphere_idx] -= sphere_base_speed * speed_factor;
            }

            if (sphere_y_offset[sphere_idx] >= BOX_SIZE - SPHERE_SIZE) sphere_falling[sphere_idx] = true;
            else if (sphere_y_offset[sphere_idx] <= -(BOX_SIZE - SPHERE_SIZE)) sphere_falling[sphere_idx] = false;
            if (sphere_x_offset[sphere_idx] >= BOX_SIZE - SPHERE_SIZE) sphere_moving_right[sphere_idx] = false;
            else if (sphere_x_offset[sphere_idx] <= -(BOX_SIZE - SPHERE_SIZE)) sphere_moving_right[sphere_idx] = true;

            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(sphere_x_offset[sphere_idx], sphere_y_offset[sphere_idx], 0.0f));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(SPHERE_SIZE));

            update_world_obb(shapes[i]);
        }
    }

    glutPostRedisplay(); // 다시 그리기 요청
    glutTimerFunc(1000 / 60, Timer, 1); //--- 타이머 콜백함수 지정 (60 FPS)
}

GLvoid Mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            // 드래그 시작
            is_rotating = true;
            last_mouse_x = x;
        }
        else if (state == GLUT_UP) {
            // 드래그 종료
            is_rotating = false;
        }
    }
}

GLvoid MouseMove(int x, int y) {
    if (is_rotating) {
        // x좌표 변화량 계산
        int dx = x - last_mouse_x;

        rotation_angle += (float)dx * 0.1f;
        cube_rotate_angle += (float)dx * 0.1f;

        last_mouse_x = x;

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

    // --- OBB 계산을 위한 로컬 좌표계의 최소/최대값 초기화
    glm::vec3 min_v = glm::vec3(FLT_MAX);
    glm::vec3 max_v = glm::vec3(-FLT_MAX);

    static float r, g, b;

     // 큐브, 구체 등 object_num이 1 이상인 도형 (단일 객체 로딩)

    SHAPE object_shape;
    object_shape.object_num = object_num;

    // --- AABB 계산 및 정점 데이터 합치기 ---
    for (auto& m : models) {
        for (size_t i = 0; i < m.face_count; i++) {
            Face f = m.faces[i];
            Vertex v1 = m.vertices[f.v1];
            Vertex v2 = m.vertices[f.v2];
            Vertex v3 = m.vertices[f.v3];

            // AABB 계산 (모든 정점을 대상으로)
            for (const Vertex* v : { &v1, &v2, &v3 }) {
                min_v.x = glm::min(min_v.x, v->x);
                min_v.y = glm::min(min_v.y, v->y);
                min_v.z = glm::min(min_v.z, v->z);
                max_v.x = glm::max(max_v.x, v->x);
                max_v.y = glm::max(max_v.y, v->y);
                max_v.z = glm::max(max_v.z, v->z);
            }

            // 색상 설정 (object_num 기준)
            if (object_num == 0) { r = 0.1f; g = 0.1f; b = 0.1f; }
            else if (object_num == 1) { r = rdcolor(mt); g = rdcolor(mt); b = rdcolor(mt); }
            else if (object_num == 2) { r = rdcolor(mt); g = rdcolor(mt); b = rdcolor(mt); }
            else if (object_num == 3) { r = rdcolor(mt); g = rdcolor(mt); b = rdcolor(mt); }
            else if (object_num == 4) { r = rdcolor(mt); g = rdcolor(mt); b = rdcolor(mt); }
            else if (object_num == 5) { r = 0.8f; g = 0.5f; b = 0.2f; }
            else if (object_num == 6) { r = 0.2f; g = 0.8f; b = 0.5f; }
            else if (object_num == 7) { r = 0.5f; g = 0.2f; b = 0.8f; }
            else if (object_num >= 8) { r = 0.9f; g = 0.9f; b = 0.2f; }

            object_shape.vertex.insert(object_shape.vertex.end(), {
                v1.x, v1.y, v1.z, r, g, b,
                v2.x, v2.y, v2.z, r, g, b,
                v3.x, v3.y, v3.z, r, g, b
                });
            object_shape.face_count++;
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