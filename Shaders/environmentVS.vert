#version 330 core
layout (location = 0) in vec3 vPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 localPos;

void main()
{
	localPos = vPos;

	mat4 rotView = mat4(mat3(view));
	vec4 clipPos = projection * rotView * vec4(localPos, 1.0);

	gl_Position = clipPos.xyww;
}