#version 150 core

precision highp float;
precision highp int;

layout (std140, column_major) uniform;
#define COUNT 4

uniform sampler2D diffuse;

out VS_OUT {
  vec2 Texcoord;
  vec4 Lumimance[COUNT];
}fs_in;

out vec4 fragColor;

void main()
{
	highp uint First = uint(0);
	vec4 Luminance = vec4(0.0);
    for(uint i = First; i < uint(COUNT); ++i)
	Luminance += fs_in.Lumimance[i];

    fragColor = texture(diffuse, fs_in.Texcoord) * Luminance;
}
