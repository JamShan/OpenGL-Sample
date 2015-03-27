#version 150 core

layout(std140) uniform;

uniform sampler2D Diffuse;

in vec4 gl_FragCoord;
out vec4 fragColor;

void main()
{           //纹理坐标基于片元坐标
	fragColor = texture(Diffuse, vec2(gl_FragCoord.x, 1.0 - gl_FragCoord.y) / vec2(640, 480));
}
