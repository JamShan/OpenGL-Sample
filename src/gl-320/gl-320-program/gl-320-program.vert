#version 150 core

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

uniform transform {
   mat4 mvp_matrix;
} Transform;

in vec2 position;

void main()
{
   gl_Position = Transform.mvp_matrix * vec4(position, 0.0, 1.0);
}

