#version 150 core

layout(std140) uniform;

uniform material
{
	vec4 Diffuse;
} Material;

out vec4 fragColor;

void main()
{
	fragColor = Material.Diffuse;
}
