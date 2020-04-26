#version 330 core
out float FragAO;

in vec2 FragTexCoords;

uniform sampler2D positionTex;
uniform sampler2D normalTex;

uniform sampler2D noiseTex;

const int kernelSize = 64;
uniform vec3 kernelSamples[kernelSize];

uniform mat4 view;
uniform mat4 projection;

const vec2 noiseScale = vec2(1920.0 / 4.0, 1080.0 / 4.0);
const float radius = 0.5;
const float bias = 0.025;

void main()
{
    vec3 fragPos = texture(positionTex, FragTexCoords).xyz;
    vec3 normal = normalize(texture(normalTex, FragTexCoords)).rgb;
    vec3 randomRotationVector = normalize(texture(noiseTex, FragTexCoords * noiseScale).xyz);

    // TBN orthonormal basis (gramm-schmidt process)
    vec3 tangent = normalize(randomRotationVector - normal * dot(randomRotationVector, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; i++) {
        // Convert sample from tangent to view space
        vec3 sample = TBN * kernelSamples[i];
        sample = fragPos + radius * sample;
        vec4 offsetFragPos = vec4(sample, 1.0);
        offsetFragPos = projection * offsetFragPos;
        offsetFragPos.xyz /= offsetFragPos.w;
        // Map view space values to texture space ([-1; 1] => [0; 1])
        offsetFragPos.xyz = offsetFragPos.xyz * 0.5 + 0.5;
        float sampleDepth = texture(positionTex, offsetFragPos.xy).z;
        // Remove effect of fragments far behind or in front of current fragment
        //float rangeDiscardFactor = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        float rangeDiscardFactor = 1.0;

        // Check if sample is "above" or "below" the fragment neighbourhood
        occlusion += (sampleDepth > sample.z + bias ? 1.0 : 0.0) * rangeDiscardFactor;
    }

    occlusion /= float(kernelSize);
    FragAO = 1.0 - occlusion;
}