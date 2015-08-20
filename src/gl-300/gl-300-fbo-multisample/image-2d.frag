#version 130

uniform sampler2D tex;

in vec2 tc;
out vec4 fragColor;

void main()
{
   fragColor = texture(tex, tc); 
}