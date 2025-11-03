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

float x_cam = 1.0f;
float y_cam = 1.0f;
float z_cam = 1.0f;
float x_at = 0.0f;
float y_at = 0.0f;
float z_at = 0.0f;
//
// 
// SM_Bank 관련 상수 (Sphere가 8부터 12까지 5개라고 가정)

// SM_Bank 위치/크기 조정을 위한 변수 (static float 변수들 사이에 추가)
static float sm_bank_y_offset = -0.5f; // Y축 초기 위치 (바닥 중앙에 가깝게)
static float sm_bank_scale = 0.5f;    // OBJ 파일 크기에 맞춰 적절히 축소

struct Material {   //재질
    std::string name;
    glm::vec3 ambient = glm::vec3(0.8f); // Ka
    glm::vec3 diffuse = glm::vec3(0.8f); // Kd
    // 필요한 경우 다른 속성도 추가 가능
};
std::vector<Material> materials;

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
void read_mtl_file(const char* filename);

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
    LoadOBJ("SM_Bank.obj", 0);

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

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
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
    switch (key) {
    case 'z':
        z_cam += 0.1f;
        z_at += 0.1f;
        break;
    case 'Z':
        z_cam -= 0.1f;
        z_at -= 0.1f;
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
    // 
    //====================================================================
    // 충돌처리
    //====================================================================
    //모델 행렬 업데이트
    for (int i = 0; i < shapes.size(); i++) {
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
    unsigned int v1, v2, v3; // 0-based vertex indices
    int material_index;      // 재질 인덱스
};

struct Model {
    Vertex* vertices;
    size_t vertex_count;
    Face* faces;
    size_t face_count;
    std::string mtl_filename;
};
std::vector<Model> models;
Model read_obj_file(const char* filename);

void read_newline(char* str) {
    char* pos;
    if ((pos = strchr(str, '\n')) != NULL)
        *pos = '\0';
}

Model read_obj_file(const char* filename) {
    Model model = { 0 };
    FILE* file;

    // 파일 열기 (오류 처리)
    if (fopen_s(&file, filename, "r") != 0 || file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    char line[MAX_LINE_LENGTH];

    // 임시 저장소
    std::string mtl_filename = "";
    std::vector<Vertex> v_positions_temp; // v 데이터를 임시로 저장
    std::vector<Face> faces_temp;         // Face 데이터를 임시로 저장

    // 재질 관련 상태
    int current_material_index = 0; // 기본값

    // 1단계: 모든 데이터 읽기 및 Face 데이터 분할
    while (fgets(line, sizeof(line), file)) {
        read_newline(line);

        if (line[0] == 'v' && line[1] == ' ') {
            // v 라인: 정점 위치 저장
            Vertex v;
            sscanf_s(line + 2, "%f %f %f", &v.x, &v.y, &v.z);
            v_positions_temp.push_back(v);

        }
        else if (strncmp(line, "mtllib ", 7) == 0) {
            // mtllib 라인: MTL 파일 이름 저장
            model.mtl_filename = std::string(line + 7);

        }
        else if (strncmp(line, "usemtl ", 7) == 0) {
            // usemtl 라인: 사용할 재질 인덱스 찾기
            std::string mat_name = std::string(line + 7);
            current_material_index = 0; // 못 찾으면 0번 (또는 기본값)
            for (size_t i = 0; i < materials.size(); ++i) {
                if (materials[i].name == mat_name) {
                    current_material_index = (int)i;
                    break;
                }
            }

        }
        else if (line[0] == 'f' && line[1] == ' ') {
            // f 라인: 정점 인덱스 파싱 및 삼각형 분할 (핵심 수정)

            // f 라인 전체를 복사하여 파싱에 사용 (strtok_s는 원본 문자열을 수정함)
            char line_copy[MAX_LINE_LENGTH];
            strcpy(line_copy, line + 2);

            // 토큰 분리: 정점 그룹 (v/vt/vn)을 얻기 위해 공백으로 분리
            std::vector<unsigned int> v_indices_in_face;
            char* token_context = NULL;
            char* token = strtok_s(line_copy, " ", &token_context);

            while (token != NULL) {
                unsigned int v = 0, vt = 0, vn = 0;

                // sscanf_s를 사용하여 v 인덱스 추출 (다양한 형식 대응)
                // v/vt/vn, v//vn, v/vt, v 모두 대응 가능하도록 %u%*s 패턴 사용
                if (sscanf_s(token, "%u", &v) == 1) { // v 인덱스가 항상 첫 번째에 오므로 이것만 추출
                    v_indices_in_face.push_back(v);
                }
                // v/vt/vn, v//vn, v/vt 형태에서는 / 뒤의 인덱스는 무시됩니다.

                token = strtok_s(NULL, " ", &token_context);
            }

            // 삼각형 분할: N각형(N > 3)을 N-2개의 삼각형으로 분할
            // f v1 v2 v3 v4 -> (v1, v2, v3), (v1, v3, v4)
            if (v_indices_in_face.size() >= 3) {
                unsigned int v_idx_start = v_indices_in_face[0];

                for (size_t j = 0; j < v_indices_in_face.size() - 2; ++j) {
                    Face f;
                    f.v1 = v_idx_start - 1;
                    f.v2 = v_indices_in_face[j + 1] - 1;
                    f.v3 = v_indices_in_face[j + 2] - 1;
                    f.material_index = current_material_index;

                    faces_temp.push_back(f);
                }
            }
        }
    }

    fclose(file);

    // 2단계: Model 구조체에 최종 데이터 할당 및 복사
    model.vertex_count = v_positions_temp.size();
    model.face_count = faces_temp.size();

    model.vertices = (Vertex*)malloc(model.vertex_count * sizeof(Vertex));
    model.faces = (Face*)malloc(model.face_count * sizeof(Face));

    if (!model.vertices || !model.faces) {
        fprintf(stderr, "Final memory allocation failed.\n");
        if (model.vertices) free(model.vertices);
        if (model.faces) free(model.faces);
        exit(EXIT_FAILURE);
    }

    // 데이터 복사
    for (size_t i = 0; i < model.vertex_count; ++i) {
        model.vertices[i] = v_positions_temp[i];
    }
    for (size_t i = 0; i < model.face_count; ++i) {
        model.faces[i] = faces_temp[i];
    }

    // MTL 파일 로딩은 read_obj_file이 아니라 LoadOBJ에서 호출되어야 하므로, 
    // 파일 이름만 저장하고 LoadOBJ에서 처리하도록 합니다. (이전 구현 유지)
    // 참고: MTL 로딩 로직은 read_obj_file 호출 전에 실행되어야 합니다.
    // 이 함수의 로직이 바뀌면서 MTL 로딩이 2단계로 분리되므로, 
    // LoadOBJ 함수에서 이 read_obj_file을 호출하기 전에 read_mtl_file을 먼저 호출하도록 해야 합니다.

    return model;
}

void LoadOBJ(const char* filename, int object_num)
{
    // --- 1단계: MTL 파일 이름 추출 (첫 번째 파싱) ---
    // read_obj_file을 호출하여 OBJ 파일 내부의 "mtllib" 지시자를 찾습니다.
    Model m_mtl_info = read_obj_file(filename);

    // --- 2단계: MTL 파일 로드 ---
    if (!m_mtl_info.mtl_filename.empty()) {
        // 🚨 중요: 이전에 로드된 재질이 있다면 모두 제거합니다.
        // 이렇게 해야 materials[0]에 SM_Bank의 "StreetBank" 재질이 올바르게 저장됩니다.
        materials.clear();
        read_mtl_file(m_mtl_info.mtl_filename.c_str());
    }

    // 1단계에서 사용한 임시 모델 데이터의 메모리를 해제합니다.
    free(m_mtl_info.vertices);
    free(m_mtl_info.faces);

    // --- 3단계: 최종 모델 로드 (두 번째 파싱) ---
    // 이제 materials 벡터가 MTL 파일의 재질로 채워져 있으므로,
    // read_obj_file이 "usemtl" 지시자를 만나면 올바른 재질 인덱스를 Face에 설정합니다.
    Model m_final = read_obj_file(filename);

    // --- OBB 계산을 위한 로컬 좌표계의 최소/최대값 초기화 ---
    glm::vec3 min_v = glm::vec3(FLT_MAX);
    glm::vec3 max_v = glm::vec3(-FLT_MAX);

    SHAPE object_shape;
    object_shape.object_num = object_num;
    object_shape.face_count = m_final.face_count;

    // --- AABB 계산 및 정점 데이터 합치기 (m_final 사용) ---
    for (size_t i = 0; i < m_final.face_count; i++) {
        Face f = m_final.faces[i];

        Vertex v1 = m_final.vertices[f.v1];
        Vertex v2 = m_final.vertices[f.v2];
        Vertex v3 = m_final.vertices[f.v3];

        // AABB 계산 (min_v, max_v 업데이트)
        min_v = glm::min(min_v, glm::vec3(v1.x, v1.y, v1.z));
        max_v = glm::max(max_v, glm::vec3(v1.x, v1.y, v1.z));
        min_v = glm::min(min_v, glm::vec3(v2.x, v2.y, v2.z));
        max_v = glm::max(max_v, glm::vec3(v2.x, v2.y, v2.z));
        min_v = glm::min(min_v, glm::vec3(v3.x, v3.y, v3.z));
        max_v = glm::max(max_v, glm::vec3(v3.x, v3.y, v3.z));

        // 정점 데이터 (위치 + 색상) 추가
        glm::vec3 color;

        // f.material_index가 0이고, materials[0]이 "빨간색" 재질이므로 정상 적용됨
        if (!materials.empty() && f.material_index >= 0 && (size_t)f.material_index < materials.size()) {
            color = materials[f.material_index].diffuse;
        }
        else {
            // "응"은 출력되지 않아야 함
            std::cout << "응" << std::endl;
            color = glm::vec3(rdcolor(mt), rdcolor(mt), rdcolor(mt));
        }

        object_shape.vertex.insert(object_shape.vertex.end(), {
            v1.x, v1.y, v1.z, color.r, color.g, color.b,
            v2.x, v2.y, v2.z, color.r, color.g, color.b,
            v3.x, v3.y, v3.z, color.r, color.g, color.b
            });
    }

    // --- OBB 설정 및 Model 행렬 초기화 ---
    glm::vec3 local_center = (min_v + max_v) * 0.5f;
    glm::vec3 local_half_length = (max_v - min_v) * 0.5f;

    object_shape.local_obb.center = local_center;
    object_shape.local_obb.half_length = local_half_length;
    object_shape.local_obb.u[0] = glm::vec3(1.0f, 0.0f, 0.0f);
    object_shape.local_obb.u[1] = glm::vec3(0.0f, 1.0f, 0.0f);
    object_shape.local_obb.u[2] = glm::vec3(0.0f, 0.0f, 1.0f);

    shapes.push_back(object_shape);

    // 3단계에서 사용한 최종 모델 데이터 메모리 해제
    free(m_final.vertices);
    free(m_final.faces);

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

void read_mtl_file(const char* filename) {
    FILE* file;
    if (fopen_s(&file, filename, "r") != 0 || file == NULL) {
        std::cerr << "Warning: Could not open MTL file: " << filename << std::endl;
        return;
    }

    char line[MAX_LINE_LENGTH];
    Material current_mat;
    bool is_processing_material = false; // 재질을 처리 중인지 확인하는 플래그

    while (fgets(line, sizeof(line), file)) {
        read_newline(line); // \n 제거

        if (strncmp(line, "newmtl ", 7) == 0) {
            // 1. "newmtl"을 만났을 때,
            //    이전에 처리 중이던 재질이 있었다면(is_processing_material == true) 먼저 저장합니다.
            if (is_processing_material) {
                materials.push_back(current_mat);
            }

            // 2. 새 재질 처리를 시작합니다.
            current_mat = Material(); // 디폴트 값(회색)으로 초기화
            current_mat.name = std::string(line + 7);
            is_processing_material = true; // 플래그 설정
        }
        else if (strncmp(line, "Kd ", 3) == 0) {
            // 3. Kd 값을 읽습니다. (sscanf_s 대신 sscanf 사용)
            float r, g, b;
            if (sscanf(line + 3, "%f %f %f", &r, &g, &b) == 3) {
                current_mat.diffuse = glm::vec3(r, g, b); // 빨간색으로 덮어쓰기
            }
        }
        // Ka, Ks 등 다른 속성도 필요하면 여기에 추가
    }

    // 4. 파일 끝에 도달했을 때, 마지막으로 처리 중이던 재질을 저장합니다.
    if (is_processing_material) {
        materials.push_back(current_mat);
    }

    fclose(file);
}