#version 150 core

precision highp float;
precision highp int;

layout (std140, column_major) uniform;

uniform sampler2D diffuse;

in VS_OUT {
   vec2 Texcoord;
}fs_in;

out vec4 fragColor;

void main()
{
   fragColor = texture(diffuse, fs_in.Texcoord);
}
