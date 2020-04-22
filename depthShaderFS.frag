#version 330 core

in vec4 FragPos;

struct Light {
    vec3 position;
    float farPlane;
};

uniform Light light;

void main()
{
    float lightDistance = length(FragPos.xyz - light.position);

    // Normalize coordinates with far plane value
    lightDistance /= light.farPlane;

    gl_FragDepth = lightDistance;

    /* After the execution of this shader,
    the depth map should be computed from the scene,
    for each cubemap face.
    */
}