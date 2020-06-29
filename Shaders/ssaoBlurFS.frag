#version 330 core
out float FragBlurredAO;

in vec2 FragTexCoords;

uniform sampler2D ssaoTex;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoTex, 0));
    float result = 0.0;
    for (int i = -2; i < 2; i++) {
        for (int j = -2; j < 2; j++) {
            // Sample AO results around the original one for averaging it
            vec2 offset = vec2(float(i), float(j)) * texelSize;
            result += texture(ssaoTex, FragTexCoords + offset).r;
        }
    }
    // We use a 4x4 kernel filter
    FragBlurredAO = result / 64.0;
}