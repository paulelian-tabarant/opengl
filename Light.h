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
    float nearPlane {1.0f}, farPlane {25.0f};

public:
    unsigned int SHADOW_WIDTH {2048}, SHADOW_HEIGHT {2048};
    unsigned int depthMapFBO;

    Light() {
        glGenFramebuffers(1, &depthMapFBO);
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