#version 330 core
out vec4 FragColor;
in vec3 localPos;

// Taken from learnOpenGL tutorial

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 mapToTexRGB(vec3 v)
{
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

void main()
{
	vec2 uv = mapToTexRGB(normalize(localPos));
	vec3 color = texture(equirectangularMap, uv).rgb;

	FragColor = vec4(color, 1.0);
}