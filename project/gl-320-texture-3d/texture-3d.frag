#version 150 core

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

in vec4 gl_FragCoord;

uniform sampler3D diffuse;
uniform mat3 orientation;

out vec4 Color;

void main()
{
	vec3 SNorm = orientation * vec3(gl_FragCoord.xy / vec2(640, 480) - 0.5, 0.0);

	Color = texture(diffuse, SNorm * 0.707106781 + 0.5);
}
