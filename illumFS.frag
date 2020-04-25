#version 330 core
out vec4 FragColor;

in vec2 FragTexCoords;

uniform samplerCube cubeMap;

uniform sampler2D positionTex;
uniform sampler2D normalTex;
uniform sampler2D colorSpecTex;

uniform vec3 cameraPos;

struct Light {
	vec3 position;
	float farPlane;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float radius;
};

struct Attenuation {
	float kc;
	float kl;
	float kq;
};
uniform Attenuation attenuation;

#define LIGHTS_NB 20
uniform Light lights[LIGHTS_NB];

float computeShadow(vec3 fragPos, Light light)
{
	vec3 lightToFrag = fragPos - light.position;
	float depth = length(lightToFrag);
	if (depth > light.farPlane)
		return 0.0;

	// Average on a certain radius around the main direction
	// (shadow smoothing)
	vec3 sampleOffsetDirections[20] = vec3[]
	(
	vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
	vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
	vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
	vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
	vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
	); 
	float viewDist = length(fragPos - cameraPos);
	float radius = (1.0 + (viewDist / light.farPlane)) / 25.0;
	int samples = 20;
	float bias = 0.05;
	float shadow = 0.0;
	for (int i = 0; i < samples; i++) {
		vec3 sampleDir = lightToFrag + sampleOffsetDirections[i] * radius;
		// Only give direction & the cube map will do the rest for sampling
		float textureDepth = texture(cubeMap, sampleDir).r;
		textureDepth *= light.farPlane;
		if (depth - bias > textureDepth) shadow += 1.0;
	}
	shadow /= float(samples);

	return shadow;
}

vec3 computePointLight(Light light, vec4 colorSpec, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 ambient = colorSpec.rgb * light.ambient;

	vec3 lightDir = normalize(-(fragPos - light.position));
	float diffCoeff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = colorSpec.rgb * light.diffuse * diffCoeff;

	vec3 reflectDir = reflect(-lightDir, normal);
	float specCoeff = pow(max(dot(reflectDir, viewDir), 0.0), 100.0);
	vec3 specular = light.specular * specCoeff;

	//float shadow = computeShadow(fragPos, light);

	vec3 result = ambient + /*(1.0 - shadow) **/ (diffuse + specular);

	float d = length(fragPos - light.position);

	float a = 1 / (attenuation.kc + attenuation.kl * d + attenuation.kq * (d*d));
	result *= a;

	return result;
}

void main()
{
    vec3 position = texture(positionTex, FragTexCoords).rgb;
    vec3 normal = normalize(texture(normalTex, FragTexCoords).rgb);
    vec4 albedoSpec = texture(colorSpecTex, FragTexCoords);
    vec3 viewDir = normalize(cameraPos - position);

	vec3 result = vec3(0.0);
	for (int i = 0; i < LIGHTS_NB; i++) {
		float distance = length(lights[i].position - position);
		// Note: avoid branching in shaders
		if (distance > lights[i].radius) continue;

    	result += computePointLight(lights[i], albedoSpec, normal, position, viewDir);
	}

	// Reinhard tone mapping
    vec3 mapped = result / (result + vec3(1.0));

    FragColor = vec4(mapped, 1.0);
}