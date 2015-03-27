#version 150 core

layout(std140) uniform;

const float Luminance[2] = float[2](1.0, 0.2);

in vs_out
{
	flat int Index;
	vec4 Color;
} fs_in;

out vec4 fragColor;

void main()
{
	fragColor = fs_in.Color * Luminance[fs_in.Index];
}
