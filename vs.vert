#version 330 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;

out vec3 FragPos;
out vec3 FragNormal;
out vec2 FragTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

float near = 0.1;
float far = 100.0;

void main() 
{
    mat4 proj = mat4(1.0);
    float sx = 2.0;
    float sy = 2.0 / (16.0 / 9.0);
    float sz = -2.0 / (far - near);
    proj[0][0] = sx;
    proj[1][1] = sy;
    proj[2][2] = sz;
    proj[2][3] = -1.0;

    vec4 hyperPos;

    vec4 orientedViewPos = mat4(mat3(view)) * model * vec4(vPos, 1.0f);
    vec4 camPos = view[3];
    float x = orientedViewPos.x;
    float y = orientedViewPos.y;
    float z = orientedViewPos.z;
    float w = orientedViewPos.w;

    // Hyperbolic transform with fixed half plane direction (X axis)
    /*
    vec4 modelPos = model * vec4(vPos, 1.0f);
    vec4 camPos = view[3];
    float x = modelPos.x;
    float y = modelPos.y;
    float z = modelPos.z;
    float w = modelPos.w;
    */

    // Euclidian
    //hyperPos =  orientedViewPos;

    float dx = x - camPos.x;
    float dx2 = dx * dx;
    float dy = y - camPos.y;
    float dz = (z - camPos.z);
    float az = (z + camPos.z);
    float d1 = sqrt(dx2 + dz*dz);
    float d2 = sqrt(dx2 + az*az);
    float s = log((d2 + d1) / (d2 - d1)) / (d1 * d2);
    vec4 newDir = vec4(s * 2.0 * camPos.z * dx, dy, s * (dx2 + dz * az), 0.0);
    hyperPos = camPos + newDir;

    gl_Position = proj * view * hyperPos;

    FragPos = vec3(model * vec4(vPos, 1.0));
    FragNormal = vNormal;
    FragTexCoords = vTexCoords;
}