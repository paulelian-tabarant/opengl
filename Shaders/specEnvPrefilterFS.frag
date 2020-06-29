#version 330 core
out vec4 FragColor;
in vec3 localPos;

uniform samplerCube environmentMap;
uniform float Roughness;

#define PI 3.14159265359

// Van Der Corpus sequence
float radicalInverse(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// Hammersley sequence
vec2 hammersley(uint i, uint n)
{
	return vec2(float(i) / float(n), radicalInverse(i));
}

// Epic games technique for environment specular importance sampling
// Combination of importance sampling with GGX normal distribution function 
vec3 importanceSampleGgx(vec2 xi, vec3 n, float roughness)
{
	float alpha = roughness * roughness;

	float phi = 2.0 * PI * xi.x;
	float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (alpha*alpha - 1.0) * xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	vec3 h = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

	vec3 up = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 t = normalize(cross(up, n));
	vec3 b = cross(n, t);
	vec3 globalSample = t * h.x + b * h.y + n * h.z;

	return normalize(globalSample);
}

const uint SAMPLES_NB = 4096u;

void main()
{
	vec3 n = normalize(localPos);
	// Epic games approximation
	vec3 viewDir = n;

	float totalWeight = 0.0;
	vec3 outputColor = vec3(0.0);
	for (uint i = 0u; i < SAMPLES_NB; i++) {
		vec2 xi = hammersley(i, SAMPLES_NB);
		// Compute a sample oriented with microfacet model
		vec3 h = importanceSampleGgx(xi, n, Roughness);
		// Reflection vector from incidence h
		vec3 l = normalize(2.0 * dot(viewDir, h) * h - viewDir);
		float n_dot_l = max(dot(n, l), 0.0);
		if (n_dot_l > 0.0) {
			outputColor += texture(environmentMap, l).rgb * n_dot_l;
			totalWeight += n_dot_l;
		}
	}
	outputColor /= totalWeight;

	FragColor = vec4(outputColor, 1.0);
}