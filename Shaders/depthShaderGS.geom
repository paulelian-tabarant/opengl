#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];

out vec4 FragPos;

void main()
{
    for (int f = 0; f < 6; f++) {
        gl_Layer = f;
        for (int i = 0; i < 3; i++) {
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[f] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}