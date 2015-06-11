#version 150 core

precision highp float;
precision highp int;

layout (std140, column_major) uniform; //¡–”≈œ»
uniform tranform  //uniformøÈ
{
   mat4 mvp_matrix;
} Tranform;

in vec2 Position;
in vec2 Texcoord;

out VS_OUT {
  vec2 Texcoord;
}vs_out;

out gl_PerVertex 
{ 
   vec4 gl_Position;
};

void mian()
{
   vs_out.Texcoord = Texcoord;
   gl_Position = Transform.mvp_matrix * vec4(Position, 0.0, 1.0);
}


