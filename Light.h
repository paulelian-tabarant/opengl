#pragma once

#include "Shader.h"

#include <stdio.h>
#include <glm/glm.hpp>

class Light
{
private:
    std::string ambientFieldName  {"ambient"},
                diffuseFieldName  {"diffuse"},
                specularFieldName {"specular"};

    glm::vec3 ambient  {glm::vec3(0.4f, 0.4f, 0.4f)};
    glm::vec3 diffuse  {glm::vec3(0.7f, 0.7f, 0.7f)};
    glm::vec3 specular {glm::vec3(1.0f, 1.0f, 1.0f)};

protected:
    std::string lightName {"light"};
    // CAUTION: be sure that far plane value acccurately embeds all your scene details
    const float nearPlane {1.0f}, farPlane {10.0f};

public:
    const unsigned int SHADOW_WIDTH {2048}, SHADOW_HEIGHT {2048};
    unsigned int depthMapFBO;
    unsigned int depthMap;

    Light() {
        glGenFramebuffers(1, &depthMapFBO);
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // Fragments which are outside the depth map will we rendered the same way as the border values
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); 
        float borderColor[] = { 1.0, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void setColor(const glm::vec3 &ambient, const glm::vec3 &diffuse)
    {
        this->ambient = ambient;
        this->diffuse = diffuse;
    }

    void setUniformName(const std::string &name)
    {
        lightName = name;
    }

    virtual void writeToShader(Shader &shader)
    {
        std::ostringstream ambientStream, diffuseStream, specularStream;
        ambientStream << lightName << "." << ambientFieldName;
        diffuseStream << lightName << "." << diffuseFieldName;
        specularStream << lightName << "." << specularFieldName;

        shader.setVec3(ambientStream.str(), ambient);
        shader.setVec3(diffuseStream.str(), diffuse);
        shader.setVec3(specularStream.str(), specular);
    }
};