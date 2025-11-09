#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#define MAX_LINE_LENGTH 256

#define BOX_SIZE  2.5f

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
std::uniform_real_distribution<float> rdlocate(-BOX_SIZE, BOX_SIZE);

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
float z_cam = 10.0f;
float x_at = 0.0f;
float y_at = 0.0f;
float z_at = 0.0f;

bool iso = false;
float open_stack = 0.0f;
float camera_radius = 10.0f; // 초기 카메라 공전 반경 (x_cam과 z_cam의 초기값으로 계산됨)
float camera_angle_deg = 0.0f; // Y축 기준으로 공전하는 각도 (도 단위)
const float ORBIT_SPEED = 15.0f; // 공전 속도 (15도/클릭)11

float robot_x = 0.0f;
float robot_y = 0.0f;
float robot_z = 0.0f;
int robot_dir = 4; // 0앞 1우 2뒤 3좌 4멈
float robot_speed = 0.1f; // 수평 속도

float robot_y_vel = 0.0f; // 수직 속도
const float GRAVITY = -0.01f; // 중력 가속도 (작은 값으로 설정)
const float JUMP_VELOCITY = 0.25f;
const float FLOOR_Y_LIMIT = 1.2f; // 로봇이 서있을 때의 최소 robot_y 값 (바닥 충돌 처리용)
bool ground = false;
bool jump = false;

float animation_time = 0.0f; // 애니메이션 시간 카운터
float arm_angle = 0.0f;     // 팔의 현재 회전 각도
float leg_angle = 0.0f;     // 다리의 현재 회전 각도
float SWING_LIMIT = 20.0f; // 팔/다리 최대 스윙 각도 (도)
float SWING_SPEED = 0.1f; // 스윙 속도 (Timer 함수에서 증분될 값)

float box1_x = rdlocate(mt), box1_z = rdlocate(mt);
float box2_x = rdlocate(mt), box2_z = rdlocate(mt);
float box3_x = rdlocate(mt), box3_z = rdlocate(mt);
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

    for (int i = 0; i < 6;i++) LoadOBJ("plane.obj", i);
    for (int i = 0; i < 11;i++) LoadOBJ("15_cube.obj", i + 6);

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
    glEnable(GL_CULL_FACE);

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
        glm::mat4 model = shapes[i].model;

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

        if (modelLoc == -1) {
            std::cerr << "ERROR: Model uniform location not found!" << std::endl;
        }

        int vertexCount = shapes[i].vertex.size() / 6;
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
    float angle_rad;

    switch (key) {
    case 'o':
        iso = !iso;
        break;
    case 'w':
        robot_dir = 2;
        break;
    case 'a':
        robot_dir = 3;
        break;
    case 's':
        robot_dir = 0;
        break;
    case 'd':
        robot_dir = 1;
        break;
    case '-':
        if (robot_speed > 0.05f){
            robot_speed -= 0.05f;
            SWING_LIMIT -= 5.0f;
            SWING_SPEED -= 0.02f;
        }
        if (robot_speed <= 0.05f) robot_speed = 0.05f;
        break;
    case '=':
        if (robot_speed < 0.2f ) {
            robot_speed += 0.05f;
            SWING_LIMIT += 5.0f;
            SWING_SPEED += 0.02f;
        }
        if (robot_speed >= 0.3f) robot_speed = 0.3f;

        break;
    case 'j':
        if (ground) {
            jump = true;
            robot_y_vel = JUMP_VELOCITY;
        }
        break;
    case 'i':
        x_cam = 0.0f;
        y_cam = 0.0f;
        z_cam = 10.0f;
        x_at = 0.0f;
        y_at = 0.0f;
        z_at = 0.0f;
        iso = false;
        open_stack = 0.0f;
        camera_radius = 10.0f;
        camera_angle_deg = 0.0f;
        robot_x = 0.0f;
        robot_y = 0.0f;
        robot_z = 0.0f;
        robot_dir = 4; // 0앞 1우 2뒤 3좌
        robot_speed = 0.02f; // 수평 속도
        robot_y_vel = 0.0f; // 수직 속도
        ground = false;
        jump = false;
        animation_time = 0.0f;
        arm_angle = 0.0f;
        leg_angle = 0.0f;
        box1_x = rdlocate(mt), box1_z = rdlocate(mt);
        box2_x = rdlocate(mt), box2_z = rdlocate(mt);
        box3_x = rdlocate(mt), box3_z = rdlocate(mt);
        break;
    case 'z':
        z_cam += 1.0f;
        z_at += 1.0f;
        break;
    case 'Z':
        z_cam -= 1.0f;
        z_at -= 1.0f;
        break;
    case 'x':
        x_cam += 1.0f;
        x_at += 1.0f;
        break;
    case 'X':
        x_cam -= 1.0f;
        x_at -= 1.0f;
        break;
    case 'y':
        camera_angle_deg += ORBIT_SPEED;
        if (camera_angle_deg >= 360.0f) camera_angle_deg -= 360.0f;

        angle_rad = glm::radians(camera_angle_deg);

        x_cam = x_at + camera_radius * glm::sin(angle_rad);
        z_cam = z_at + camera_radius * glm::cos(angle_rad);
        break;
    case 'Y':
        camera_angle_deg -= ORBIT_SPEED;
        if (camera_angle_deg < 0.0f) camera_angle_deg += 360.0f;

        angle_rad = glm::radians(camera_angle_deg);

        x_cam = x_at + camera_radius * glm::sin(angle_rad);
        z_cam = z_at + camera_radius * glm::cos(angle_rad);
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

    //====================================================================
    // 충돌처리 및 변수 업데이트
    if(iso && open_stack < BOX_SIZE*2) open_stack += 0.05f;
    else if(!iso && open_stack > 0) open_stack -= 0.05f;

    if(robot_dir != 4) animation_time += SWING_SPEED;
    arm_angle = SWING_LIMIT * glm::sin(animation_time);
    leg_angle = SWING_LIMIT * glm::sin(animation_time);

    //for (int i = 6;i <= 12;i++) {
    //    if (check_obb_collision(shapes[i], shapes[0])) {
    //        if (check_obb_collision(shapes[i], shapes[13]) || check_obb_collision(shapes[i], shapes[14]) || check_obb_collision(shapes[i], shapes[15])) {
    //            if (robot_dir == 1) robot_dir = 3;
    //            else if (robot_dir == 3) robot_dir = 1;
    //            else if (robot_dir == 0) robot_dir = 2;
    //            else if (robot_dir == 2) robot_dir = 0;
    //        }
    //    }
    //
    //    if (check_obb_collision(shapes[i], shapes[3]) && robot_dir == 1) robot_dir = 3;
    //    if (check_obb_collision(shapes[i], shapes[5]) && robot_dir == 3) robot_dir = 1;
    //    if (robot_z >= BOX_SIZE && robot_dir == 0) robot_dir = 2;
    //    if (robot_z <= -BOX_SIZE && robot_dir == 2) robot_dir = 0;
    //}
    //
    //robot_y_vel += GRAVITY; // 가속도 적용 (속도 업데이트)
    //ground = false;
    //if (!jump) {
    //    for (int i = 6;i <= 12;i++) {
    //        if (check_obb_collision(shapes[i], shapes[0]) || check_obb_collision(shapes[i], shapes[13]) || check_obb_collision(shapes[i], shapes[14]) || check_obb_collision(shapes[i], shapes[15])) {
    //            robot_y_vel = 0.0f;
    //            ground = true;
    //        }
    //    }
    //}
    //if (robot_y_vel > 0.0f) jump = false;
    //robot_y += robot_y_vel;

    

    robot_y_vel += GRAVITY; // 가속도 적용 (속도 업데이트)
    ground = false;
    if (!jump) {
        if (check_obb_collision(shapes[16], shapes[0]) || check_obb_collision(shapes[16], shapes[13]) || check_obb_collision(shapes[16], shapes[14]) || check_obb_collision(shapes[16], shapes[15])) {
            robot_y_vel = 0.0f;
            ground = true;
        }
    }
    if (robot_y_vel > 0.0f) jump = false;
    robot_y += robot_y_vel;

    if (check_obb_collision(shapes[16], shapes[0])) {
        if (check_obb_collision(shapes[16], shapes[13]) || check_obb_collision(shapes[16], shapes[14]) || check_obb_collision(shapes[16], shapes[15])) {
            if (robot_dir == 1) robot_dir = 3;
            else if (robot_dir == 3) robot_dir = 1;
            else if (robot_dir == 0) robot_dir = 2;
            else if (robot_dir == 2) robot_dir = 0;
        }
    }

    if (check_obb_collision(shapes[16], shapes[3]) && robot_dir == 1) robot_dir = 3;
    if (check_obb_collision(shapes[16], shapes[5]) && robot_dir == 3) robot_dir = 1;
    if (robot_z >= BOX_SIZE && robot_dir == 0) robot_dir = 2;
    if (robot_z <= -BOX_SIZE && robot_dir == 2) robot_dir = 0;
    
    if (robot_dir == 0) robot_z += robot_speed;
    else if (robot_dir == 1) robot_x += robot_speed;
    else if (robot_dir == 2) robot_z -= robot_speed;
    else if (robot_dir == 3) robot_x -= robot_speed;

    //====================================================================
    //모델 행렬 업데이트
    for (int i = 0; i < shapes.size(); i++) {
        shapes[i].model = glm::mat4(1.0f);

        if (shapes[i].object_num == 0) { //밑
            shapes[i].model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -BOX_SIZE, 0.0f));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(5.0f));
        }
        else if (shapes[i].object_num == 1) { //위
            shapes[i].model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, BOX_SIZE, 0.0f));
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(5.0f));
        }
        else if (shapes[i].object_num == 2) { //앞
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, open_stack, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, 0.0f, BOX_SIZE));
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(5.0f));
        }
        else if (shapes[i].object_num == 3) { //오
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(BOX_SIZE, 0.0f, 0.0f));
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(5.0f));
        }
        else if (shapes[i].object_num == 4) { //뒤
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(0.0f, 0.0f, -BOX_SIZE));
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(5.0f));
        }
        else if (shapes[i].object_num == 5) { //왼
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(-BOX_SIZE, -0.0f, 0.0f));
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(270.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(5.0f));
        }
        //================================================
        else if (shapes[i].object_num > 5 && shapes[i].object_num < 13 || shapes[i].object_num == 16) { // 로봇 전체
            float robot_rotate = 0.0f;
            if (robot_dir == 0) robot_rotate = 0.0f;
            else if (robot_dir == 1) robot_rotate = 90.0f;
            else if (robot_dir == 2) robot_rotate = 180.0f;
            else if (robot_dir == 3) robot_rotate = 270.0f;

            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(robot_x, robot_y, robot_z));
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(robot_rotate), glm::vec3(0.0f, 1.0f, 0.0f));
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(-robot_x, -robot_y, -robot_z));

            if (shapes[i].object_num == 6) { //머리
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(robot_x, robot_y + 0.6f, robot_z));
                shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.4f, 0.2f, 0.2f));
            }
            else if (shapes[i].object_num == 7) { //코
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(robot_x, robot_y + 0.5f, robot_z + 0.2f));
                shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.2f, 0.1f, 0.4f));
            }
            else if (shapes[i].object_num == 8) { //몸
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(robot_x, robot_y, robot_z));
                shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.6f, 0.4f, 0.4f));
            }
            else if (shapes[i].object_num == 9) { //오팔
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(robot_x - 0.7f, robot_y, robot_z));
                shapes[i].model = glm::rotate(shapes[i].model, glm::radians(-arm_angle), glm::vec3(1.0f, 0.0f, 0.0f));
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(-(robot_x - 0.7f), -robot_y, -robot_z));

                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(robot_x - 0.7f, robot_y, robot_z));
                shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.1f,0.4f,0.1f));
            }
            else if (shapes[i].object_num == 10) { //왼팔
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(robot_x + 0.7f, robot_y, robot_z));
                shapes[i].model = glm::rotate(shapes[i].model, glm::radians(arm_angle), glm::vec3(1.0f, 0.0f, 0.0f));
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(-(robot_x + 0.7f), -robot_y, -robot_z));

                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(robot_x + 0.7f, robot_y, robot_z));
                shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.1f, 0.4f, 0.1f));
            }
            else if (shapes[i].object_num == 11) { //오다
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(robot_x - 0.2f, robot_y - 0.4f, robot_z));
                shapes[i].model = glm::rotate(shapes[i].model, glm::radians(leg_angle), glm::vec3(1.0f, 0.0f, 0.0f));
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(-(robot_x - 0.2f), -(robot_y - 0.4f), -robot_z));

                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(robot_x - 0.2f, robot_y - 0.8f, robot_z));
                shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.1f, 0.4f, 0.1f));
            }
            else if (shapes[i].object_num == 12) { //왼다
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(robot_x + 0.2f, robot_y - 0.4f, robot_z));
                shapes[i].model = glm::rotate(shapes[i].model, glm::radians(-leg_angle), glm::vec3(1.0f, 0.0f, 0.0f));
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(-(robot_x + 0.2f), -(robot_y - 0.4f), -robot_z));

                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(robot_x + 0.2f, robot_y - 0.8f, robot_z));
                shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.1f, 0.4f, 0.1f));
            }
            else if (shapes[i].object_num == 16) { //히트박스
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(robot_x, robot_y - 0.8f, robot_z));
                shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.6f, 0.4f, 0.4f));
            }
            //================================================
        }
        else if (shapes[i].object_num >= 13){
            if (shapes[i].object_num == 13) { //장애물1
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(box1_x, -BOX_SIZE + 0.4f, box1_z));
                shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.8f, 0.4f, 0.8f));
            }
            else if (shapes[i].object_num == 14) { //장애물2
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(box2_x, -BOX_SIZE + 0.4f, box2_z));
                shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.8f, 0.4f, 0.2f));
            }
            else if (shapes[i].object_num == 15) { //장애물3
                shapes[i].model = glm::translate(shapes[i].model, glm::vec3(box3_x, -BOX_SIZE + 0.4f, box3_z));
                shapes[i].model = glm::scale(shapes[i].model, glm::vec3(0.2f, 0.4f, 0.8f));
            }
        }

        update_world_obb(shapes[i]);
    }

    glutPostRedisplay(); // 다시 그리기 요청
    glutTimerFunc(1000 / 60, Timer, 1); //--- 타이머 콜백함수 지정 (60 FPS)
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
    r = rdcolor(mt); g = rdcolor(mt); b = rdcolor(mt);

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
            //r = rdcolor(mt); g = rdcolor(mt); b = rdcolor(mt);

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