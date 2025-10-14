#version 330 core

layout(location = 0) in vec3 aPos;     // ���� ��ġ
layout(location = 1) in vec3 aColor;   // ���� ����

// ��� ������ 3��
uniform mat4 uModel;   // ������ + ȸ�� + �̵�
uniform mat4 uView;    // ī�޶� ��ȯ
uniform mat4 uProj;    // ���� ��ȯ (���� or ����)

out vec3 vColor;

void main()
{
    // ��� ��ȯ�� �ϳ��� ����
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
    vColor = aColor;
}
