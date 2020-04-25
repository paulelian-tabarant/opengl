#version 330 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;

out VertexData {
    vec3 FragPos;
    vec3 FragNormal;
    vec2 FragTexCoords;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform bool reverseNormals;

void main() 
{
    vec4 worldPos = model * vec4(vPos, 1.0);
    vs_out.FragPos = worldPos.xyz;

    mat3 normalMat = mat3(transpose(inverse(model)));
    if (!reverseNormals) 
        vs_out.FragNormal = normalMat * vNormal;
    else 
        vs_out.FragNormal = normalMat * (-1.0 * vNormal);
    vs_out.FragTexCoords = vTexCoords;

    gl_Position = projection * view *  worldPos;
}