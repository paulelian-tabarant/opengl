#version 330 core
in vec3 FragPos;
in vec3 FragNormal;
in vec2 FragTexCoords;
out vec4 FragColor;

uniform vec3 cameraPos;
uniform vec3 a;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct Light {
	vec3 position;
	vec3 direction;
	float innerAngle;
	float outerAngle;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material material;
uniform Light light;

float angleFromVectors(vec3 v1, vec3 v2)
{
	return acos(dot(v1, v2) / (length(v1) * length(v2)));
}

void main() 
{
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, FragTexCoords));

	vec3 norm = normalize(FragNormal);
	vec3 lightDir = normalize(-(FragPos - light.position));
	float diffCoeff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * vec3(texture(material.diffuse, FragTexCoords)) * diffCoeff;

	vec3 cameraDir = normalize(cameraPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float specCoeff = pow(max(dot(reflectDir, cameraDir), 0.0), material.shininess);
	vec3 specular = light.specular * vec3(texture(material.specular, FragTexCoords)) * specCoeff;

	float dirAngle = angleFromVectors(light.direction, FragPos - light.position);
	float spotCoeff = (light.outerAngle - dirAngle) / (light.innerAngle - light.outerAngle);
	spotCoeff = clamp(spotCoeff, 0.0, 1.0);

	float d = length(FragPos - light.position);
	float att = 1 / (a[0] + a[1] * d + a[2] * pow(d, 2.0));

    vec3 result = ambient + spotCoeff * (diffuse + specular);
	result *= att;
    FragColor = vec4(result, 1.0);
}