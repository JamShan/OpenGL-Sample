#version 150 core

layout(std140) uniform;

uniform transform  
{
	mat4 MVP;  //mvpæÿ’Û
} Transform;

in vec2 Position;
uniform mat4 mvp_matrix;

void main()
{
	gl_Position = mvp_matrix * vec4(Position, 0.0, 1.0);
}
