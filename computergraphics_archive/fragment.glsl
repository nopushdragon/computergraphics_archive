#version 330 core
out vec4 color;

uniform vec4 outColor;

void main ()
{
	color = outColor;
	//color = vec4 (1.0, 0.0, 0.0, 1.0);
}