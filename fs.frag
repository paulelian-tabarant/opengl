#version 330 core
in vec3 FragPos;
in vec3 FragNormal;
in vec2 FragTexCoords;
out vec4 FragColor;

uniform vec3 cameraPos;
uniform vec3 attenuation;

struct Material {
	sampler2D diffuse1;
	sampler2D diffuse2;
	sampler2D diffuse3;
	sampler2D specular1;
	sampler2D specular2;
	sampler2D specular3;

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
	vec3 ambient = light.ambient /** vec3(texture(material.diffuse1, FragTexCoords))*/;

	vec3 lightDir = normalize(-light.direction);
	float diffCoeff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse /** vec3(texture(material.diffuse1, FragTexCoords))*/ * diffCoeff;

	vec3 reflectDir = reflect(-lightDir, normal);
	float specCoeff = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
	vec3 specular = light.specular /** vec3(texture(material.specular1, FragTexCoords))*/ * specCoeff;

	vec3 result = ambient + diffuse + specular;

	return result;
}

void main() 
{
	vec3 result = vec3(0.0);
	vec3 viewDir = normalize(cameraPos - FragPos);
	vec3 normal = normalize(FragNormal);

	result += computeDirLight(dirLight, normal, viewDir);

	FragColor = vec4(result, 1.0);
}
