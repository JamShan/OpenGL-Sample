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
   vec4 temp = texture(diffuse, fs_in.Texcoord);
   if (temp.a < 0.2)
     discard;
   fragColor = temp;
}
