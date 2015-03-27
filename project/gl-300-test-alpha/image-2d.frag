#version 330

uniform sampler2D sampler;

in vec2 vertTexCoord;

out vec4 fragColor;

void main()
{
   fragColor = texture(sampler, vertTexCoord); //”¶”√Œ∆¿Ìdiffuse
}