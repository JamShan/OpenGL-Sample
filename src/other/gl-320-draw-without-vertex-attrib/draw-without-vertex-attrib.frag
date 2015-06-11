#version 150 core

layout(std140) uniform;

out vec4 fragColor;

void main()
{
	fragColor = vec4(1.0f, 0.5f, 0.0f, 1.0f);
}

