#version 330 core
in VertexData {
	vec3 FragPos;
	vec3 FragNormal;
	vec2 FragTexCoords;
	vec4 FragPosLightSpace;
} fs_in;

out vec4 FragColor;

uniform vec3 cameraPos;
uniform vec3 attenuation;

struct Material {
	sampler2D diffuse;
	sampler2D specular;

	float shininess;
};
uniform Material material;

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform DirLight dirLight;

struct PointLight {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
#define POINT_LIGHTS_NB 3
uniform PointLight pointLights[POINT_LIGHTS_NB];

uniform sampler2D shadowMap;

float computeShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
	// Perspective division (clip-space coordinates)
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// Convert from -1.0;1.0 to 0.0;1.0
	projCoords = (projCoords + 1.0) / 2.0;
	// for dirLight for the moment
	float depth = projCoords.z;
	if (depth > 1.0) // after the far plane, force shadow value to 0.0
		return 0.0;

	float bias = 0.005;

	float textureDepth = texture(shadowMap, projCoords.xy).r;
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	// Average shadow from tests on neighbour patch
	for (int i = -2; i <= 2; i++) {
		for (int j = -2; j <= 2; j++) {
			float textureDepth = texture(shadowMap, projCoords.xy + vec2(i, j) * texelSize).r;
			if (depth - bias > textureDepth)
				shadow += 1.0 / 25.0;
		}
	}

	return shadow;
}

float ks = 0.2;

vec3 computeDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, fs_in.FragTexCoords));

	vec3 lightDir = normalize(-light.direction);
	float diffCoeff = max(dot(normal, lightDir), 0.0);

	vec3 diffuse = light.diffuse * vec3(texture(material.diffuse, fs_in.FragTexCoords)) * diffCoeff;

	vec3 reflectDir = reflect(-lightDir, normal);
	float specCoeff = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
	vec3 specular = light.specular /** vec3(texture(material.specular, fs_in.FragTexCoords))*/ * specCoeff;

	float shadow = computeShadow(fs_in.FragPosLightSpace, normal, lightDir);

	vec3 result = ambient + (1.0 - shadow) * (diffuse +  ks * specular);

	return result;
}

vec3 computePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 att)
{
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, fs_in.FragTexCoords));

	vec3 lightDir = normalize(-(fragPos - light.position));
	float diffCoeff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * vec3(texture(material.diffuse, fs_in.FragTexCoords)) * diffCoeff;

	vec3 reflectDir = reflect(-lightDir, normal);
	float specCoeff = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
	vec3 specular = light.specular /** vec3(texture(material.specular, fs_in.FragTexCoords))*/ * specCoeff;

	vec3 result = ambient + diffuse + ks * specular;

	float d = length(fragPos - light.position);
	float a = 1 / (att[0] + att[1] * d + att[2] * pow(d, 2.0));
	result *= a;

	return result;
}

void main() 
{
	vec3 result = vec3(0.0);
	vec3 viewDir = normalize(cameraPos - fs_in.FragPos);
	vec3 normal = normalize(fs_in.FragNormal);

	result += computeDirLight(dirLight, normal, viewDir);
	/*
	for (int i = 0; i < POINT_LIGHTS_NB; i++) {
		result += computePointLight(pointLights[i], normal, fs_in.FragPos, viewDir, attenuation);
	}
	*/

    FragColor = vec4(result, 1.0);
}
