#version 330 core

layout(location = 0) in vec3 aPos;     // 정점 위치
layout(location = 1) in vec2 aTexCoord; // 텍스처 좌표 (location=1)
layout(location = 2) in vec3 aColor;   // 정점 색상 (location=2)

// 행렬 유니폼 3개
uniform mat4 uModel;   // 스케일 + 회전 + 이동
uniform mat4 uView;    // 카메라 변환
uniform mat4 uProj;    // 투영 변환 (직교 or 원근)

out vec3 vColor;
out vec2 vTexCoord;    // 텍스처 좌표 출력 변수

void main()
{
    // 모든 변환을 하나로 적용
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
    vColor = aColor;
    vTexCoord = aTexCoord; // 텍스처 좌표 전달
}