#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in VertexData {
	vec3 FragPos;
	vec3 FragNormal;
	vec2 FragTexCoords;
} fs_in;

struct Material {
	sampler2D diffuseTex;
	sampler2D specularTex;
	bool hasDiffuseTex;
	bool hasSpecularTex;

	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;

	float shininess;
};
uniform Material material;

void main() 
{
	gPosition = fs_in.FragPos;
	gNormal = normalize(fs_in.FragNormal);
	gAlbedoSpec.rgb = material.hasDiffuseTex ? 
		texture(material.diffuseTex, fs_in.FragTexCoords).rgb : material.diffuseColor;
	gAlbedoSpec.a = material.hasSpecularTex ? 
		texture(material.specularTex, fs_in.FragTexCoords).r : material.shininess;
}
