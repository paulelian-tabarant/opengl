#version 330 core
in VertexData {
	vec3 FragPos;
	vec3 FragNormal;
	vec2 FragTexCoords;
} fs_in;

out vec4 FragColor;

uniform vec3 cameraPos;

struct Material {
	sampler2D diffuseTex;
	sampler2D specularTex;
	bool hasDiffuseTex;
	bool hasSpecularTex;

	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;

	float shininess;
};
uniform Material material;

struct Light {
	vec3 position;
	float farPlane;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform Light light;

uniform samplerCube cubeMap;

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

vec3 computePointLight(Light light, vec3 normal, vec3 fragPos, vec2 texCoords, vec3 viewDir)
{
	vec3 ambient = material.ambientColor * light.ambient;
	if (material.hasDiffuseTex) ambient *= vec3(texture(material.diffuseTex, texCoords));

	vec3 lightDir = normalize(-(fragPos - light.position));
	float diffCoeff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = material.diffuseColor * light.diffuse * diffCoeff;
	if (material.hasDiffuseTex) diffuse *= vec3(texture(material.diffuseTex, texCoords));

	vec3 reflectDir = reflect(-lightDir, normal);
	float specCoeff = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
	vec3 specular = material.specularColor * light.specular * specCoeff;
	//if (material.hasSpecularTex) specular *= vec3(texture(material.specularTex, texCoords));

	float shadow = computeShadow(fragPos, light);
	vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);

	float d = length(fragPos - light.position);
	// TODO: re-implement distance attenuation
	float a = 1 / (d * d);
	result *= a;

	return result;
}

void main() 
{
	vec3 result = vec3(0.0);
	vec3 viewDir = normalize(cameraPos - fs_in.FragPos);
	vec3 normal = normalize(fs_in.FragNormal);

	result += computePointLight(light, normal, fs_in.FragPos, fs_in.FragTexCoords, viewDir);

    FragColor = vec4(result, 1.0);
}
