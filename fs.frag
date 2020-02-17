#version 330 core
in vec3 FragPos;
in vec3 FragNormal;
out vec4 FragColor;

uniform vec3 cameraPos;

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct Light {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material material;
uniform Light light;

void main() 
{
    vec3 ambient = light.ambient * material.ambient;

	vec3 norm = normalize(FragNormal);
	vec3 lightDir = normalize(light.position - FragPos);
	float diffCoeff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * material.diffuse * diffCoeff;

	vec3 cameraDir = normalize(cameraPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float specCoeff = pow(max(dot(reflectDir, cameraDir), 0.0), material.shininess);
	vec3 specular = light.specular * material.specular * specCoeff;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}