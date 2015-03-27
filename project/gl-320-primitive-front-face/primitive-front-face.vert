#version 150 core

precision highp float; //��֤���ݵ�һ����
precision highp int;
layout(std140, column_major) uniform;

uniform mat4 mvp_matrix;

in vec2 Position;

void main()
{	
	gl_Position = mvp_matrix * vec4(Position, 0.0, 1.0);
}

