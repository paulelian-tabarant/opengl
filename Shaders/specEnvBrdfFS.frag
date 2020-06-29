#version 330 core
out vec2 FragColor;

in vec2 FragTexCoords;

#define PI 3.141592653589

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

// Taken directly from learnOpenGl specular IBL chapter
float geometrySchlickGGX(float NdotV, float roughness)
{
    float a = roughness;
    float k = (a * a) / 2.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
} 

vec2 integrateBrdf(float n_dot_v, float roughness)
{
	vec3 v;
	v = vec3(sqrt(1.0 - n_dot_v * n_dot_v), 0.0, n_dot_v);
	float res_factor1 = 0.0;
	float res_factor2 = 0.0;

	vec3 n = vec3(0.0, 0.0, 1.0);
	const uint SAMPLES_NB = 1024u;

	for (uint i = 0u; i < SAMPLES_NB; i++) {
		vec2 xi = hammersley(i, SAMPLES_NB);
		vec3 h = importanceSampleGgx(xi, n, roughness);
		vec3 l = -reflect(v, h); // TO BE VERIFIED
		float n_dot_l = max(l.z, 0.0);
		float n_dot_h = max(h.z, 0.0);
		float v_dot_h = max(dot(v, h), 0.0);

		if (n_dot_l > 0.0) {
			float g = geometrySmith(n, v, l, roughness);
			float g_vis = (g * v_dot_h) / (n_dot_h * n_dot_v);
			float fc = pow(1.0 - v_dot_h, 5.0);
			res_factor1 += (1.0 - fc) * g_vis;
			res_factor2 += fc * g_vis;
		}
	}
	res_factor1 /= float(SAMPLES_NB);
	res_factor2 /= float(SAMPLES_NB);

	return vec2(res_factor1, res_factor2);
}

void main()
{
	vec2 brdfResult = integrateBrdf(FragTexCoords.x, FragTexCoords.y);
	FragColor = brdfResult;
}