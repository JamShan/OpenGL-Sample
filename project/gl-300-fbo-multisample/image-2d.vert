#version 130

in vec2 Position;  //顶点和纹理坐标
in vec2 TexCoord;

out vec2 vertTexCoord;
uniform mat4 mvp_matrix;

void main()
{
   vertTexCoord = TexCoord;
   gl_Position = mvp_matrix * vec4(Position, 0.0, 1.0);

}