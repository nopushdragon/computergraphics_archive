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
#include <map> // 맵 컨테이너 추가

// ⚠️ STB_IMAGE 헤더 파일 통합 및 구현 정의
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // 프로젝트 폴더에 stb_image.h가 있다고 가정

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
GLuint load_texture_from_file(const char* filename);
void LoadOBJ(const char* filename, int object_num);
void reset_bool();

//--- 필요한 변수 선언
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체
GLuint vao, vbo;

//
bool depth_on = true;

float x_cam = 2.5f;
float y_cam = 2.5f;
float z_cam = 2.5f;
float x_at = 0.0f;
float y_at = 0.0f;
float z_at = 0.0f;

bool c_p = false;
float x_stack = 0.0f;
float y_stack = 0.0f;
//
// 

struct Material {   //재질
    std::string name;
    glm::vec3 ambient = glm::vec3(0.8f); // Ka
    glm::vec3 diffuse = glm::vec3(0.8f); // Kd
    std::string diffuse_map_filename; // 텍스처 파일 이름 추가 (map_Kd)
};
std::vector<Material> materials;

std::vector<GLfloat> allVertices;
struct SHAPE {
    std::vector<GLfloat> vertex;
    glm::mat4 model = glm::mat4(1.0f);
    int face_count = 0; // 이 SHAPE가 포함하는 face 수
    int object_num;
    GLuint texture_id = 0; // 각 SHAPE가 사용할 텍스처 ID
};
std::vector<SHAPE> shapes;

GLvoid MouseMove(int x, int y);
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

    // 텍스처 로딩: LoadOBJ에서 MTL 파일 이름이 추출된 후 텍스처가 로드됨
    LoadOBJ("tex_cube.obj", 0);
    LoadOBJ("tex_pyramid.obj", 1);
    LoadOBJ("tex_cube.obj", 2);

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
    vertexSource = filetobuf("t_vertex.glsl");
    if (!vertexSource) {
        std::cerr << "ERROR: t_vertex.glsl 파일을 찾을 수 없습니다." << std::endl;
        exit(EXIT_FAILURE);
    }

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
        exit(EXIT_FAILURE); // 컴파일 실패 시 강제 종료
        return;
    }
}

void make_fragmentShaders()
{
    GLchar* fragmentSource;
    fragmentSource = filetobuf("t_fragment.glsl");
    if (!fragmentSource) {
        std::cerr << "ERROR: t_fragment.glsl 파일을 찾을 수 없습니다." << std::endl;
        exit(EXIT_FAILURE);
    }

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
        exit(EXIT_FAILURE); // 컴파일 실패 시 강제 종료
        return;
    }
}

GLuint make_shaderProgram()
{
    GLint result;
    GLchar* errorLog = NULL;
    GLuint shaderID;
    shaderID = glCreateProgram();
    glAttachShader(shaderID, vertexShader);
    glAttachShader(shaderID, fragmentShader);
    glLinkProgram(shaderID);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
        std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
        exit(EXIT_FAILURE); // 연결 실패 시 강제 종료
        return 0;
    }
    glUseProgram(shaderID);

    // 텍스처 유니폼 초기화 (Texture Unit 0 사용)
    GLint texLoc = glGetUniformLocation(shaderID, "uTextureSampler");
    if (texLoc != -1) {
        glUniform1i(texLoc, 0);
    }

    return shaderID;
}

void UpdateBuffer()
{
    allVertices.clear();

    // 분리된 SHAPE 객체의 정점 데이터를 하나의 버퍼로 합친다.
    for (const auto& shape : shapes) {
        allVertices.insert(allVertices.end(), shape.vertex.begin(), shape.vertex.end());
    }

    // 합쳐진 데이터로 VBO 업데이트
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(GLfloat), allVertices.data(), GL_DYNAMIC_DRAW);
}

GLvoid drawScene() {
    if (depth_on)
        glEnable(GL_DEPTH_TEST); // 은면제거
    else
        glDisable(GL_DEPTH_TEST);

    // 🚨 배경색을 밝은 회색으로 변경하여 모델과 구분
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramID);

    // View 및 Projection 행렬 설정
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

    glBindVertexArray(vao);
    GLint first = 0;

    glActiveTexture(GL_TEXTURE0); // Texture Unit 0 활성화

    // SHAPE 객체를 순회하며 렌더링
    for (int i = 0; i < shapes.size(); i++) {

        // 텍스처 바인딩 (SHAPE에 해당하는 텍스처)
        glBindTexture(GL_TEXTURE_2D, shapes[i].texture_id);

        glm::mat4 model = shapes[i].model;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

        // 현재 SHAPE의 정점 개수 (정점당 8개의 GLfloat)
        int vertexCount = shapes[i].vertex.size() / 8;

        // VBO에서 현재 SHAPE의 데이터가 시작되는 위치(offset)부터 렌더링
        if(c_p == false && shapes[i].object_num == 0 || shapes[i].object_num == 2) glDrawArrays(GL_TRIANGLES, first, vertexCount);
        else if (c_p == true && shapes[i].object_num == 1 || shapes[i].object_num == 2) glDrawArrays(GL_TRIANGLES, first, vertexCount);

        // 다음 SHAPE의 시작 위치로 인덱스 업데이트
        first += vertexCount;
    }

    glBindTexture(GL_TEXTURE_2D, 0); // 텍스처 바인딩 해제
    glBindVertexArray(0);
    glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
    glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'c': c_p = false; break;
    case 'p': c_p = true; break;
    case 'h': depth_on = !depth_on; break;
    case 'q': exit(0); break;
    case 'x':
        x_stack += 15.0f;
        break;
    case 'X':
        x_stack -= 15.0f;
        break;
    case 'y':
        y_stack += 15.0f;
        break;
    case 'Y':
        y_stack -= 15.0f;
        break;
    case 's':
        x_stack = 0.0f;
        y_stack = 0.0f;
        break;
    }
    glutPostRedisplay();
}

GLvoid Timer(int value)
{
    for( int i = 0 ; i<shapes.size(); i++ )
    {
        shapes[i].model = glm::mat4(1.0f);
        if (shapes[i].object_num == 0 || shapes[i].object_num == 1) {
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(y_stack), glm::vec3(0.0f, 1.0f, 0.0f));
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(x_stack), glm::vec3(1.0f, 0.0f, 0.0f));
        }
        else {
            shapes[i].model = glm::translate(shapes[i].model, glm::vec3(-3.0f, -3.0f, -3.0f));
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            shapes[i].model = glm::rotate(shapes[i].model, glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            shapes[i].model = glm::scale(shapes[i].model, glm::vec3(10.0f, 10.0f,1.0f));
        }
    }

    glutPostRedisplay();
    glutTimerFunc(1000 / 60, Timer, 1);
}

GLvoid Mouse(int button, int state, int x, int y) {}
GLvoid MouseMove(int x, int y) {}


void InitBuffer()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // 정점당 8개의 GLfloat (위치 3 + 텍스처좌표 2 + 색상 3)
    const GLsizei stride = 8 * sizeof(float);

    // --- 위치 속성 (location = 0, vec3)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // --- 텍스처 좌표 속성 (location = 1, vec2)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // --- 색상 속성 (location = 2, vec3)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

struct Vertex { float x, y, z; };
struct TexCoord { float u, v; };
struct Face {
    unsigned int v1, v2, v3;
    unsigned int vt1, vt2, vt3;
    int material_index;
};

struct Model {
    Vertex* vertices; size_t vertex_count;
    TexCoord* texcoords; size_t texcoord_count;
    Face* faces; size_t face_count;
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

    if (fopen_s(&file, filename, "r") != 0 || file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    char line[MAX_LINE_LENGTH];

    std::vector<Vertex> v_positions_temp;
    std::vector<TexCoord> vt_coords_temp;
    std::vector<Face> faces_temp;
    int current_material_index = 0;

    while (fgets(line, sizeof(line), file)) {
        read_newline(line);

        if (line[0] == 'v' && line[1] == ' ') {
            Vertex v;
            sscanf_s(line + 2, "%f %f %f", &v.x, &v.y, &v.z);
            v_positions_temp.push_back(v);

        }
        else if (line[0] == 'v' && line[1] == 't' && line[2] == ' ') {
            TexCoord vt;
            sscanf_s(line + 3, "%f %f", &vt.u, &vt.v);
            vt_coords_temp.push_back(vt);
        }
        else if (strncmp(line, "mtllib ", 7) == 0) {
            model.mtl_filename = std::string(line + 7);

        }
        else if (strncmp(line, "usemtl ", 7) == 0) {
            std::string mat_name = std::string(line + 7);
            current_material_index = 0;
            for (size_t i = 0; i < materials.size(); ++i) {
                if (materials[i].name == mat_name) {
                    current_material_index = (int)i;
                    break;
                }
            }

        }
        else if (line[0] == 'f' && line[1] == ' ') {
            char line_copy[MAX_LINE_LENGTH];
            strcpy(line_copy, line + 2);

            std::vector<unsigned int> v_indices;
            std::vector<unsigned int> vt_indices;

            char* token_context = NULL;
            char* token = strtok_s(line_copy, " ", &token_context);

            while (token != NULL) {
                unsigned int v = 0, vt = 0;
                // v/vt/vn 또는 v/vt 또는 v//vn 형식에 대응
                if (sscanf_s(token, "%u/%u", &v, &vt) == 2) {
                    v_indices.push_back(v);
                    vt_indices.push_back(vt);
                }
                else if (sscanf_s(token, "%u//%*u", &v) == 1) { // v//vn
                    v_indices.push_back(v);
                    vt_indices.push_back(0);
                }
                else if (sscanf_s(token, "%u", &v) == 1) { // v
                    v_indices.push_back(v);
                    vt_indices.push_back(0);
                }

                token = strtok_s(NULL, " ", &token_context);
            }

            // 삼각형 분할 (Fan Triangulation)
            if (v_indices.size() >= 3) {
                unsigned int v_idx_start = v_indices[0];
                unsigned int vt_idx_start = vt_indices[0];

                for (size_t j = 0; j < v_indices.size() - 2; ++j) {
                    Face f;
                    f.v1 = v_idx_start - 1;
                    f.v2 = v_indices[j + 1] - 1;
                    f.v3 = v_indices[j + 2] - 1;

                    // 텍스처 좌표 인덱스 (1-based -> 0-based)
                    f.vt1 = (vt_idx_start > 0) ? vt_idx_start - 1 : UINT_MAX;
                    f.vt2 = (vt_indices[j + 1] > 0) ? vt_indices[j + 1] - 1 : UINT_MAX;
                    f.vt3 = (vt_indices[j + 2] > 0) ? vt_indices[j + 2] - 1 : UINT_MAX;

                    f.material_index = current_material_index;

                    faces_temp.push_back(f);
                }
            }
        }
    }

    fclose(file);

    model.vertex_count = v_positions_temp.size();
    model.texcoord_count = vt_coords_temp.size();
    model.face_count = faces_temp.size();

    model.vertices = (Vertex*)malloc(model.vertex_count * sizeof(Vertex));
    model.texcoords = (TexCoord*)malloc(model.texcoord_count * sizeof(TexCoord));
    model.faces = (Face*)malloc(model.face_count * sizeof(Face));

    if (!model.vertices || !model.texcoords || !model.faces) {
        fprintf(stderr, "Final memory allocation failed.\n");
        if (model.vertices) free(model.vertices);
        if (model.texcoords) free(model.texcoords);
        if (model.faces) free(model.faces);
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < model.vertex_count; ++i) { model.vertices[i] = v_positions_temp[i]; }
    for (size_t i = 0; i < model.texcoord_count; ++i) { model.texcoords[i] = vt_coords_temp[i]; }
    for (size_t i = 0; i < model.face_count; ++i) { model.faces[i] = faces_temp[i]; }

    return model;
}

// stb_image를 사용한 텍스처 로딩 함수
GLuint load_texture_from_file(const char* filename) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;
        else {
            std::cerr << "ERROR: Image format not supported for " << filename << std::endl;
            stbi_image_free(data);
            return 0;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
        std::cout << "Texture loaded successfully: " << filename << std::endl;
    }
    else {
        std::cerr << "ERROR: Texture failed to load at path: " << filename << std::endl;
        stbi_image_free(data);
        glDeleteTextures(1, &textureID);
        return 0;
    }

    return textureID;
}


// 큐브의 6개 면에 6개 텍스처를 적용하도록 로직 변경
void LoadOBJ(const char* filename, int object_num)
{
    // OBJ 파일 vt 좌표는 보통 뒤집혀 있으므로 로드 시 뒤집는다.
    stbi_set_flip_vertically_on_load(true);

    // 1. MTL 파일 로드
    Model m_mtl_info = read_obj_file(filename);

    if (!m_mtl_info.mtl_filename.empty()) {
        materials.clear();
        read_mtl_file(m_mtl_info.mtl_filename.c_str());
    }

    free(m_mtl_info.vertices); free(m_mtl_info.texcoords); free(m_mtl_info.faces);

    // 2. 최종 모델 로드
    Model m_final = read_obj_file(filename);

    // 3. 모든 재질에 대해 텍스처를 미리 로드하고 맵에 저장 (재질 인덱스 -> 텍스처 ID)
    std::map<int, GLuint> texture_map;
    for (size_t i = 0; i < materials.size(); ++i) {
        if (!materials[i].diffuse_map_filename.empty()) {
            // 파일을 로드하고 ID를 얻는다.
            GLuint id = load_texture_from_file(materials[i].diffuse_map_filename.c_str());
            texture_map[(int)i] = id;
        }
        else {
            texture_map[(int)i] = 0; // 텍스처 없는 재질은 ID 0
        }
    }

    // 4. SHAPE 객체 분할 및 정점 데이터 생성
    // 큐브의 면(usemtl 그룹)이 바뀔 때마다 새로운 SHAPE 객체를 생성한다.

    // 초기 SHAPE 객체 생성
    SHAPE current_shape;
    current_shape.object_num = object_num;

    if (m_final.face_count == 0) return;

    int current_material_index = m_final.faces[0].material_index;
    current_shape.texture_id = texture_map[current_material_index];


    for (size_t i = 0; i < m_final.face_count; i++) {
        Face f = m_final.faces[i];

        // **[핵심 로직]** 재질 인덱스가 바뀌면 (즉, 큐브의 다른 면을 만나면)
        if (f.material_index != current_material_index) {
            // 이전 SHAPE를 저장
            if (!current_shape.vertex.empty()) {
                shapes.push_back(current_shape);
            }

            // 새 SHAPE 시작
            current_shape = SHAPE();
            current_shape.object_num = object_num;
            current_material_index = f.material_index;
            current_shape.texture_id = texture_map[current_material_index];
        }

        // 정점 추출
        Vertex v1 = m_final.vertices[f.v1];
        Vertex v2 = m_final.vertices[f.v2];
        Vertex v3 = m_final.vertices[f.v3];

        // 텍스처 좌표 추출 (인덱스 유효성 검사 강화)
        bool t1_ok = (f.vt1 != UINT_MAX && f.vt1 < m_final.texcoord_count);
        bool t2_ok = (f.vt2 != UINT_MAX && f.vt2 < m_final.texcoord_count);
        bool t3_ok = (f.vt3 != UINT_MAX && f.vt3 < m_final.texcoord_count);

        TexCoord t1 = t1_ok ? m_final.texcoords[f.vt1] : TexCoord{ 0.0f, 0.0f };
        TexCoord t2 = t2_ok ? m_final.texcoords[f.vt2] : TexCoord{ 0.0f, 0.0f };
        TexCoord t3 = t3_ok ? m_final.texcoords[f.vt3] : TexCoord{ 0.0f, 0.0f };


        // 정점 색상: 텍스처가 로드되었으면 흰색(1.0)으로 고정하여 텍스처 색상만 사용
        glm::vec3 color;
        if (current_shape.texture_id > 0) {
            color = glm::vec3(1.0f, 1.0f, 1.0f);
        }
        else {
            // 텍스처 로드 실패 시 MTL의 확산 색상 사용 (어두운 파란색)
            color = materials[current_material_index].diffuse;
        }

        // 정점 버퍼에 위치(3), 텍스처좌표(2), 색상(3) 순서로 저장
        current_shape.vertex.insert(current_shape.vertex.end(), {
            v1.x, v1.y, v1.z, t1.u, t1.v, color.r, color.g, color.b,
            v2.x, v2.y, v2.z, t2.u, t2.v, color.r, color.g, color.b,
            v3.x, v3.y, v3.z, t3.u, t3.v, color.r, color.g, color.b
            });
    }

    // 마지막 SHAPE 저장
    if (!current_shape.vertex.empty()) {
        shapes.push_back(current_shape);
    }

    // 메모리 해제
    free(m_final.vertices); free(m_final.texcoords); free(m_final.faces);

    UpdateBuffer();
}

void read_mtl_file(const char* filename) {
    FILE* file;
    if (fopen_s(&file, filename, "r") != 0 || file == NULL) {
        std::cerr << "Warning: Could not open MTL file: " << filename << std::endl;
        return;
    }

    char line[MAX_LINE_LENGTH];
    Material current_mat;
    bool is_processing_material = false;

    while (fgets(line, sizeof(line), file)) {
        read_newline(line);

        // 🚨 라인의 앞부분 공백/탭을 건너뛰는 포인터
        char* current_line = line;
        while (*current_line == ' ' || *current_line == '\t') {
            current_line++;
        }

        // 빈 라인이거나 주석 라인은 건너뜁니다.
        if (current_line[0] == '#' || current_line[0] == '\0') continue;

        if (strncmp(current_line, "newmtl ", 7) == 0) {
            if (is_processing_material) {
                materials.push_back(current_mat);
            }
            current_mat = Material();
            current_mat.name = std::string(current_line + 7);
            is_processing_material = true;
        }
        else if (strncmp(current_line, "Kd ", 3) == 0) {
            float r, g, b;
            if (sscanf(current_line + 3, "%f %f %f", &r, &g, &b) == 3) {
                current_mat.diffuse = glm::vec3(r, g, b);
            }
        }
        else if (strncmp(current_line, "map_Kd ", 7) == 0) {
            // map_Kd 키워드 뒤의 파일 이름을 읽습니다.
            current_mat.diffuse_map_filename = std::string(current_line + 7);

            // 텍스트 파일명을 공백/줄바꿈 없이 깔끔하게 처리
            size_t end = current_mat.diffuse_map_filename.find_last_not_of(" \t\n\r");
            if (end != std::string::npos) {
                current_mat.diffuse_map_filename.resize(end + 1);
            }
            else {
                current_mat.diffuse_map_filename.clear(); // 파일명이 없으면 비움
            }

            // 텍스처 로드 실패 문제를 줄이기 위해 확장자를 .png로 강제 변환
            if (!current_mat.diffuse_map_filename.empty()) {
                size_t dot_pos = current_mat.diffuse_map_filename.find_last_of('.');
                if (dot_pos != std::string::npos) {
                    current_mat.diffuse_map_filename.replace(dot_pos, current_mat.diffuse_map_filename.length() - dot_pos, ".png");
                }
                else {
                    current_mat.diffuse_map_filename += ".png";
                }
            }

            if (!current_mat.diffuse_map_filename.empty()) {
                std::cout << "MTL found texture map: " << current_mat.diffuse_map_filename << std::endl;
            }
        }
    }

    // 마지막으로 처리 중이던 재질을 저장합니다.
    if (is_processing_material) {
        materials.push_back(current_mat);
    }

    fclose(file);
}