//1-8
#version 330 core


in vec3 outColor; //--- 버텍스 세이더에게서 전달 받음
out vec4 FragColor; //--- 색상 출력
void main ()
{
	FragColor = vec4 (outColor, 1.0);
}
/*
//--- fragment shader: fragment.glsl 파일에 저장
#version 330 core
//--- out_Color: 버텍스 세이더에서 입력받는 색상 값
//--- FragColor: 출력할 색상의 값으로 프레임 버퍼로 전달 됨.
in vec3 out_Color; //--- 버텍스 세이더에게서 전달 받음
out vec4 FragColor; //--- 색상 출력
void main(void)
{
FragColor = vec4 (out_Color, 1.0);
}
*/

/*
//--- in_Position: attribute index 0
//--- in_Color: attribute index 1
layout (location = 0) in vec3 in_Position; //--- 위치 변수: attribute position 0
layout (location = 1) in vec3 in_Color; //--- 컬러 변수: attribute position 1
out vec3 out_Color; //--- 프래그먼트 세이더에게 전달
void main(void)
{
gl_Position = vec4 (in_Position.x, in_Position.y, in_Position.z, 1.0);
out_
*/

/*
out vec4 FragColor;
in vec3 outColor;
void main()
{
FragColor = vec4 (outColor, 1.0);
}
*/





