#version 330 core
in vec3 FragPos;
in vec3 FragNormal;
in vec2 FragTexCoords;
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
#define POINT_LIGHTS_NB 4
uniform PointLight pointLights[POINT_LIGHTS_NB];

vec3 computeDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, FragTexCoords));

	vec3 lightDir = normalize(-light.direction);
	float diffCoeff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * vec3(texture(material.diffuse, FragTexCoords)) * diffCoeff;

	vec3 reflectDir = reflect(-lightDir, normal);
	float specCoeff = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
	vec3 specular = light.specular * vec3(texture(material.specular, FragTexCoords)) * specCoeff;

	return ambient + diffuse + specular;
}

vec3 computePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 att)
{
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, FragTexCoords));

	vec3 lightDir = normalize(-(fragPos - light.position));
	float diffCoeff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * vec3(texture(material.diffuse, FragTexCoords)) * diffCoeff;

	vec3 reflectDir = reflect(-lightDir, normal);
	float specCoeff = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
	vec3 specular = light.specular * vec3(texture(material.specular, FragTexCoords)) * specCoeff;

	float d = length(FragPos - light.position);
	float a = 1 / (att[0] + att[1] * d + att[2] * pow(d, 2.0));

    vec3 result = ambient + diffuse + specular;
	return result * a;
}

void main() 
{
	vec3 result = vec3(0.0);
	vec3 viewDir = normalize(cameraPos - FragPos);
	vec3 normal = normalize(FragNormal);

	result += computeDirLight(dirLight, normal, viewDir);

	for (int i = 0; i < POINT_LIGHTS_NB; i++) {
		result += computePointLight(pointLights[i], normal, FragPos, viewDir, attenuation);
	}

    FragColor = vec4(result, 1.0);
}