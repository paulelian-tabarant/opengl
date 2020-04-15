#version 330 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;

out VertexData {
    vec3 FragPos;
    vec3 FragNormal;
    vec2 FragTexCoords;
    vec4 FragPosLightSpace;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main() 
{
    vs_out.FragPos = vec3(model * vec4(vPos, 1.0));
    mat3 normalMat = mat3(transpose(inverse(model)));
    vs_out.FragNormal = normalMat * vNormal;
    vs_out.FragTexCoords = vTexCoords;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);

    gl_Position = projection * view * model * vec4(vPos, 1.0f);
}