#version 150 core

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

uniform samplerCube environment;

in VS_OUT {
  vec3 reflect;
}fs_in;

out vec4 fragColor;

void main()
{
   fragColor = texture(environment, fs_in.reflect);
}