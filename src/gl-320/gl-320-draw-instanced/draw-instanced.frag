#version 150 core

layout(std140) uniform;

in vs_out
{
	flat int Instance;
} fs_in;

const vec4 Diffuse[2] = vec4[2](vec4(1.0, 0.5, 0.0, 1.0), 
                                vec4(0.0, 0.5, 1.0, 1.0));

out vec4 fragColor;

void main()
{
	fragColor = Diffuse[fs_in.Instance];
}
