#version 330 core
in vec3 vPos;
in vec3 vNormal;
out vec4 FragColor;

uniform vec3 cameraPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objColor;

float ambientStrength = 0.1;
float specularStrength = 0.5;

void main() 
{
    vec3 ambient = ambientStrength * lightColor;

	vec3 norm = normalize(vNormal);
	vec3 lightDir = normalize(lightPos - vPos);
	float diffCoeff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diffCoeff * lightColor;

	vec3 cameraDir = normalize(cameraPos - vPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float specCoeff = pow(max(dot(reflectDir, cameraDir), 0.0), 32.0);
	vec3 specular = specularStrength * specCoeff * lightColor;

    vec3 result = (ambient + diffuse + specular) * objColor;
    FragColor = vec4(result, 1.0);
}