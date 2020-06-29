#version 330 core
out vec4 FragColor;
in vec3 localPos;

uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main()
{
	vec3 normal = normalize(localPos);

	vec3 right = cross(vec3(0.0, 1.0, 0.0), normal);
	vec3 up = cross(normal, right);

	float angleStep = 0.025;
	int nSamples = 0;
	vec3 irradiance = vec3(0.0);
	for (float phi = 0.0; phi < 2 * PI; phi += angleStep) {
		for (float theta = 0.0; theta < 0.5 * PI; theta += angleStep) {
			vec3 localSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			vec3 globalSample = localSample.x * right + localSample.y * up + localSample.z * normal;
			// cos(theta) takes light incidence into account
			// sin(theta) takes sphere geometry (smaller longitudinal distances for high theta values)
			irradiance += texture(environmentMap, globalSample).rgb * cos(theta) * sin(theta);
			nSamples++;
		}
	}

	irradiance = (PI * irradiance) / float(nSamples);

	FragColor = vec4(irradiance, 1.0);
}