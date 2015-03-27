#version 150 core

precision highp float;
precision highp int;
layout (std140, column_major) uniform;

uniform mat4 mv_matrix;
uniform mat4 mvp_matrix;
uniform vec3 camera;

const vec3 view = vec3(0, 0, -1);
const vec3 normal = vec3(0, 0, 1);

in vec2 position;

out VS_OUT {
   vec3 reflect;
}vs_out;

void main()
{
   mat4 mv3x3 = mat3(mv_matrix);
   gl_Position = mvp_matrix * vec4(position, 0.0, 1.0);
   vec3 pos = mv3x3 * vec3(position, 0.0);
   vec3 n = mv3x3 * normal;
   vec3 eye = normalize(pos - camera);

   vs_out.reflect = reflect(eye, n);  //∑¥…‰π‚œﬂ
}
  