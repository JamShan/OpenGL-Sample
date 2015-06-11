#version 150 core

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

uniform material {
  vec4 diffuse;
} Material;

out vec4 fragColor;

void main()
{
   fragColor = Material.diffuse;
}
