#version 330 core

layout(location = 0) in vec3 aPos;     // 정점 위치
layout(location = 1) in vec3 vNormal;  // 노멀 벡터
layout(location = 2) in vec3 aColor;   // 정점 색상

// 노멀 행렬을 유니폼으로 추가하거나, uModel에서 계산하여 사용
uniform mat3 uNormalMatrix; // uModel의 역행렬의 전치행렬 (3x3)

out vec3 FragPos;
out vec3 Normal;

// 행렬 유니폼 3개
uniform mat4 uModel;   // 스케일 + 회전 + 이동
uniform mat4 uView;    // 카메라 변환
uniform mat4 uProj;    // 투영 변환 (직교 or 원근)

out vec3 vColor;

void main()
{
    // 모든 변환을 하나로 적용
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
    vColor = aColor;

    FragPos = vec3(uModel * vec4(aPos,1.0));
    Normal = normalize(uNormalMatrix * vNormal); // 2. 월드 공간 노멀 변환 (uModel의 역전치 행렬 사용)
}
