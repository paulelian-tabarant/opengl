#version 330 core
out vec4 FragColor;

in vec2 FragTexCoords;

uniform samplerCube cubeMap;

uniform sampler2D positionTex;
uniform sampler2D normalTex;
uniform sampler2D colorSpecTex;

uniform sampler2D ssaoTex;

uniform vec3 cameraPos;
uniform mat4 view;

struct Light {
	vec3 position;
	float farPlane;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Attenuation {
	float kc;
	float kl;
	float kq;
};
uniform Attenuation attenuation;

#define LIGHTS_NB 20
uniform Light lights[LIGHTS_NB];

vec3 computePointLight(Light light, vec4 colorSpec, vec3 normal, vec3 fragPos, vec3 viewDir, float ao)
{
    vec3 ambient = 2.0 * colorSpec.rgb * light.ambient;

	vec3 lightDir = normalize(-(fragPos - light.position));
	float diffCoeff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = colorSpec.rgb * light.diffuse * diffCoeff;

	vec3 reflectDir = reflect(-lightDir, normal);
	float specCoeff = pow(max(dot(reflectDir, viewDir), 0.0), 100.0);
	vec3 specular = light.specular * specCoeff;

	vec3 result = ao * ambient + diffuse + specular;

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
    vec3 viewDir = normalize(-position);
	float ao = texture(ssaoTex, FragTexCoords).r;

	vec3 result = vec3(0.0);
	for (int i = 0; i < LIGHTS_NB; i++)
    	result += computePointLight(lights[i], albedoSpec, normal, position, viewDir, ao);

	// Reinhard tone mapping
    vec3 mapped = result / (result + vec3(1.0));

    FragColor = vec4(mapped, 1.0);
}