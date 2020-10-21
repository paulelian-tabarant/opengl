#pragma once

#include <string>
#include "Shader.h"
#include "DrawUtils.h"
#define STB_IMAGE_IMPLEMENTATION ;
#include "stb_image.h"

class ImageBasedLighting
{
public:
	ImageBasedLighting(std::string imagePath, const unsigned int &maxMipLevels)
		: envMapMipLevels(maxMipLevels)
	{
		unsigned int cubeVAO {0}, cubeVBO {0};

		// Setup envmap framebuffer
		glGenFramebuffers(1, &envMapFBO);
		glGenRenderbuffers(1, &envMapRBO);
		glBindFramebuffer(GL_FRAMEBUFFER, envMapFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, envMapRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, envMapRBO);

		// Load envmap image
		stbi_set_flip_vertically_on_load(true);
		int width, height, nrComponents;
		float *data = stbi_loadf(imagePath.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			glGenTextures(1, &hdrTextureId);
			glBindTexture(GL_TEXTURE_2D, hdrTextureId);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load HDR image." << std::endl;
			return;
		}

		// Setup environment cubemap
		glGenTextures(1, &envCubeMapId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMapId);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Convert equirectangular hdr texture to cubemap
		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};
		Shader equirectToCubemapShader("lightingCubeVS.vert", "", "lightingCubeFS.frag");
		equirectToCubemapShader.use();
		equirectToCubemapShader.setInt("equirectangularMap", 0);
		equirectToCubemapShader.setMatrix4f("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrTextureId);
		glViewport(0, 0, 512, 512);
		for (unsigned int i = 0; i < 6; ++i)
		{
			equirectToCubemapShader.setMatrix4f("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubeMapId, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			DrawUtils::DrawUtils::renderCube(cubeVAO, cubeVBO);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Create a convolution of previous cubemap (diffuse irradiance precomputation)
		glGenTextures(1, &irradianceMapId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMapId);
		for (unsigned int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindFramebuffer(GL_FRAMEBUFFER, envMapFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, envMapRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

		Shader convolutionShader("lightingCubeVS.vert", "", "envmapConvolutionFS.frag");
		convolutionShader.use();
		convolutionShader.setInt("environmentMap", 0);
		convolutionShader.setMatrix4f("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMapId);
		glViewport(0, 0, 32, 32);
		glBindFramebuffer(GL_FRAMEBUFFER, envMapFBO);
		for (unsigned int i = 0; i < 6; i++) {
			convolutionShader.setMatrix4f("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMapId, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			DrawUtils::renderCube(cubeVAO, cubeVBO);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Setup specular IBL pre-filtered environment map with several roughness levels
		glGenTextures(1, &prefilterMapId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMapId);
		for (unsigned int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// Enable trilinear filtering (x, y and mipmap levels)
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		Shader prefilterShader("lightingCubeVS.vert", "", "specEnvPrefilterFS.frag");
		prefilterShader.use();
		prefilterShader.setInt("environmentMap", 0);
		prefilterShader.setMatrix4f("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMapId);

		glBindFramebuffer(GL_FRAMEBUFFER, envMapFBO);
		// Render prefiltered environment map for several mipmap levels
		for (unsigned int mip = 0; mip < envMapMipLevels; mip++) {
			unsigned int mipWidth = 128 * pow(0.5, mip);
			unsigned int mipHeight = mipWidth;
			glBindRenderbuffer(GL_RENDERBUFFER, envMapRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
			glViewport(0, 0, mipWidth, mipHeight);
			float roughness = mip / (float)(envMapMipLevels - 1);
			prefilterShader.setFloat("Roughness", roughness);
			for (unsigned int i = 0; i < 6; i++) {
				prefilterShader.setMatrix4f("view", captureViews[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMapId, mip);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				DrawUtils::renderCube(cubeVAO, cubeVBO);
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Generate precomputed lookup texture for envmap brdf
		// a) generate output texture
		glGenTextures(1, &envLutTexId);
		glBindTexture(GL_TEXTURE_2D, envLutTexId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		// b) compute result
		glBindFramebuffer(GL_FRAMEBUFFER, envMapFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, envMapRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, envLutTexId, 0);
		glViewport(0, 0, 512, 512);
		// Illumination vertex shader is used to render a normalized quad on the pinhole camera plan
		Shader lutShader("illumVS.vert", "", "specEnvBrdfFS.frag");
		lutShader.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		unsigned int quadVAO {0}, quadVBO {0};
		DrawUtils::renderQuad(quadVAO, quadVBO);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void setTextures(Shader &shader)
	{
		shader.setInt("irradianceMap", 11);
		shader.setInt("prefilterMap", 12);
		shader.setInt("brdfLut", 13);
	}

	void setEnvMapTextures(Shader &shader)
	{
		shader.setInt("environmentMap", 0);
	}

	void setUniforms(Shader &shader)
	{
		glActiveTexture(GL_TEXTURE11);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMapId);
		glActiveTexture(GL_TEXTURE12);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMapId);
		glActiveTexture(GL_TEXTURE13);
		glBindTexture(GL_TEXTURE_2D, envLutTexId);
	}

	void setEnvMapUniforms(Shader &environmentShader)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMapId);
	}

private:
	unsigned int hdrTextureId;
	unsigned int envCubeMapId;
	// Diffuse irradiance precomputation
	unsigned int irradianceMapId;
	// Specular irradiance precomputation
	unsigned int prefilterMapId;
	// Environment precomputed lookup texture (BRDF)
	unsigned int envLutTexId;
	// Framebuffer & renderbuffer objects
	unsigned int envMapFBO, envMapRBO;

	const unsigned int envMapMipLevels;
};