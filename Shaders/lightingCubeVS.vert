#version 330 core
layout (location = 0) in vec3 vPos;

out vec3 localPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
	localPos = vPos;
	gl_Position = projection * view * vec4(localPos, 1.0);
}