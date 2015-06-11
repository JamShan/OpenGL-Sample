#version 150 core

precision highp float;
precision highp int;
layout(std140, column_major) uniform;  //uniform ±Í÷æ

uniform transform //ubo
{
	mat4 mvp_matrix;
} Transform;

in vec2 Position;
in vec2 Texcoord;

out VS_OUT
{
	vec2 Texcoord;
} vs_out;

void main()
{	
	vs_out.Texcoord = Texcoord;
	gl_Position = Transform.mvp_matrix * vec4(Position, 0.0, 1.0);
}
