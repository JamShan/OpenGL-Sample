#version 150 core

layout(std140) uniform;

uniform transform
{
	mat4 MVP;
} Transform;

in vec3 Position;
in vec4 Color;

out VS_OUT
{
	flat int Index;
	vec4 Color;
} vs_out;

void main()
{
	gl_Position = Transform.MVP * vec4(Position, 1.0);
	vs_out.Color = Color;
	vs_out.Index = gl_VertexID / 4;
}

