#version 150 core

precision highp float;
precision highp int;

layout (std140, column_major) uniform; //¡–”≈œ»
#define COUNT 4

uniform tranform  //uniformøÈ
{
   mat4 mvp_matrix;
} Tranform;

in vec2 Position;
in vec2 Texcoord;

out VS_OUT {
  vec2 Texcoord;
  vec4 Lumimance[COUNT];
}vs_out;

void mian()
{
   lowp int Count = lowp int(COUNT);
   for(lowp int i = 0; i < Count; ++i)
		vs_out.Lumimance[i] = vec4(1.0) / vec4(COUNT);
   vs_out.Texcoord = Texcoord;
   gl_Position = Transform.mvp_matrix * vec4(Position, 0.0, 1.0);
}


