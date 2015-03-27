#version 150 core

layout(std140) uniform;

uniform transform
{
	mat4 MVP[2];
} Transform;

in vec2 Position;

out VS_OUT
{
  flat int Instance;
} vs_out;

void main()
{
	gl_Position = Transform.MVP[gl_InstanceID] * vec4(Position, 0.0, 1.0);
	vs_out.Instance = gl_InstanceID; //当前实例索引
}

