#version 330 core
out vec4 FragColor;

in vec2 FragTexCoords;

uniform samplerCube cubeMap;

uniform sampler2D positionTex;
uniform sampler2D normalTex;
uniform sampler2D colorSpecTex;

uniform sampler2D ssaoTex;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLut;

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

#define PI 3.1415926535897932
const float Roughness = 0.2;
const vec3 F0 = vec3(0.2);
// Metallic surface (= 1.0 if metallic)
const float Metallic = 0.0;

float computeAttenuation(vec3 fragPos, vec3 lightPos)
{
	float d = length(fragPos - lightPos);
	return 1 / (attenuation.kc + attenuation.kl * d + attenuation.kq * (d*d));
}

vec3 radiance(vec3 fragPos, Light light)
{
	// TO BE COMPLETED (by a more precise model)
	float attenuation = computeAttenuation(fragPos, light.position);

	return light.diffuse * attenuation;
}

// Trowbridge-Reitz GGX
// alpha stands for the roughness parameter of the surface.
float normalDistribFunc(vec3 normal, vec3 halfDir, float alpha)
{
	float n_dot_h2 = pow(max(dot(normal, halfDir), 0.0), 2.0);
	float alpha2   = alpha * alpha;

	float num = alpha2;
	float denom = PI * pow(n_dot_h2 * (alpha2 - 1.0) + 1.0, 2.0);

	return num / denom;
}

// Shlick-GGX
float geometryFunc(vec3 normal, vec3 viewDir, float alpha)
{
	float n_dot_v = max(dot(normal, viewDir), 0.0);
	float num 	  = n_dot_v;

	// mapping only stands for direct lighting (change for IBL)
	float k_mapping = pow(alpha + 1.0, 2.0) / 8.0;
	float denom = n_dot_v * (1.0 - k_mapping) + k_mapping;

	return num / denom;
}

// Fresnel-Shlick approximation
// f0 stands for the base reflectivity of the surface.
vec3 fresnelFunc(vec3 halfDir, vec3 viewDir, vec3 f0)
{
	return f0 + (vec3(1.0) - f0) * (1.0 - pow(max(dot(halfDir, viewDir), 0.0), 5.0));
}

// © learnOpenGl website (diffuse irradiance)
vec3 fresnelSchlickRoughness(vec3 normal, vec3 viewDir, vec3 f0, float roughness)
{
	float cosTheta = max(dot(normal, viewDir), 0.0);
	return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - cosTheta, 5.0);
}

vec3 computeEnvLight(vec3 irradiance, vec4 albedoSpec, vec3 normal, vec3 fragPos, vec3 viewDir, float ao)
{
	vec3 f = fresnelSchlickRoughness(normal, viewDir, F0, Roughness);

	// fresnel term determines the reflected energy factor ks (energy preservation)
	vec3 kd = vec3(1.0) - f; 
	vec3 diffuse = irradiance * albedoSpec.rgb;
	vec3 ambient = kd * diffuse * ao;

	return ambient;
}

vec3 computePointLight(Light light, vec4 albedoSpec, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);

	// Cook-Torrance BRDF
	// Compute from view (viewPos = vec(0.0))
	vec3 halfDir = normalize(lightDir + viewDir);
	float d = normalDistribFunc(normal, halfDir, Roughness);
	vec3 f 	= fresnelFunc(halfDir, viewDir, F0);
	// Take geometry obstruction and geometry shadowing into account
	float g = geometryFunc(normal, viewDir, Roughness) * geometryFunc(normal, lightDir, Roughness);
	float denom = 4.0 * max(dot(viewDir, normal), 0.0) * max(dot(lightDir, normal), 0.0);
	vec3 specular = (d * f * g) / max(denom, 0.001);

	// Lambert diffuse model
	// fresnel term determines the reflected energy factor ks (energy preservation)
	vec3 kd = vec3(1.0) - f; 
	kd *= 1.0 - Metallic;
	vec3 diffuse = albedoSpec.rgb / PI;

	vec3 l = radiance(fragPos, light);
	float n_dot_lightdir = max(dot(normal, lightDir), 0.0);

	return (kd * diffuse + specular) * l * n_dot_lightdir;
}

void main()
{
    vec4 albedoSpec = texture(colorSpecTex, FragTexCoords);
    vec3 normal 	= normalize(texture(normalTex, FragTexCoords).rgb);
    vec3 position 	= texture(positionTex, FragTexCoords).rgb;
    vec3 viewDir 	= normalize(-position);
	float ao 		= texture(ssaoTex, FragTexCoords).r;
	vec3 irradiance = texture(irradianceMap, normal).rgb;

	if (albedoSpec.rgb == vec3(0.0)) discard;

	// Diffuse environment part
	vec3 diffuseEnvColor = computeEnvLight(irradiance, albedoSpec, normal, position, viewDir, ao);

	// Specular environment part
	vec3 incident = reflect(-viewDir, normal);
	// Prefiltered environment map was computed with 5 lods (0 to 4)
	const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefilteredColor = textureLod(prefilterMap, incident, Roughness * MAX_REFLECTION_LOD).rgb;
	float n_dot_v = max(dot(normal, viewDir), 0.0);
	vec2 brdfFactors = texture(brdfLut, vec2(n_dot_v, Roughness)).rg;
	vec3 f = fresnelSchlickRoughness(normal, viewDir, F0, Roughness);
	vec3 specEnvColor = prefilteredColor * (f * brdfFactors.x + brdfFactors.y);

	vec3 kd = vec3(1.0) - f;
	kd *= 1.0 - Metallic;
	vec3 ambient = ao * (kd * diffuseEnvColor + specEnvColor);

	// TODO: add point lights BRDF contribution
	vec3 fragRgb = ambient;
	fragRgb = fragRgb / (fragRgb + vec3(1.0));
	fragRgb = pow(fragRgb, vec3(1.0 / 2.2));

    //FragColor = vec4(fragRgb, albedoSpec.a);
	FragColor = vec4(fragRgb, 1.0);
}