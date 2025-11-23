#version 330 core

in vec3 vColor;
in vec2 vTexCoord; // 텍스처 좌표 입력 변수

uniform sampler2D uTextureSampler; // 텍스처 샘플러 유니폼

out vec4 FragColor;

void main()
{
    // 텍스처에서 색상 샘플링
    vec4 texColor = texture(uTextureSampler, vTexCoord);
    
    // 최종 색상: 정점 색상(흰색)과 텍스처 색상의 곱으로 텍스처 색상만 출력
    FragColor = texColor * vec4(vColor, 1.0); 
}