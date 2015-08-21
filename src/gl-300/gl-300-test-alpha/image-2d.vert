#version 130

in vec2 Position; 
in vec2 TexCoord;

out vec2 tc;

void main()
{
   tc = TexCoord;
   gl_Position = vec4(Position, 0.0, 1.0);

}