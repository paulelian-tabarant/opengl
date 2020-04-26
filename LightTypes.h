#pragma once

#include "Light.h"
#include "Shader.h"
#include "Object.h"

#include <glm/glm.hpp>
#include <cmath>

class DirLight : public Light
{
private:
    std::string directionFieldName {"direction"};

    glm::vec3 direction;
    glm::mat4 lightSpaceMatrix;
    unsigned int depthMap;

public:
    DirLight(const float &x, const float &y, const float &z) :
        direction(glm::vec3(x, y, z)) 
    {
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
        glm::mat4 lightView = glm::lookAt(-direction, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        lightSpaceMatrix = lightProjection * lightView;
    }

    // Initialize a depth map consisting in a single 2D texture to be placed perpendicularly
    // to the light direction
    void initDepthMap()
    {
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

    void setDirection(const glm::vec3 &direction)
    {
        this->direction = direction;
    }

    virtual void writeToShader(Shader &shader)
    {
        Light::writeToShader(shader);

        std::ostringstream directionStream;
        directionStream << lightName << "." << directionFieldName;
        shader.setVec3(directionStream.str(), direction);
    }

    glm::mat4 getLightSpaceMatrix()
    {
        return lightSpaceMatrix;
    }
};

class PointLight : public Light
{
public:
    struct AttenuationParams {
        float constant;
        float linear;
        float quadratic;
        AttenuationParams(const float &c, const float &l, const float &q) : constant(c), linear(l), quadratic(q) {}
    };
    static const AttenuationParams attenuation;

private:
    std::string positionFieldName       {"position"      },
                farPlaneFieldName       {"farPlane"      },
                shadowMatricesFieldName {"shadowMatrices"};

    glm::vec3 position;
    glm::mat4 shadowMatrices[6];

    unsigned int cubeMapTextureID;

    Object modelObj;

private:
    void writeInnerParams(Shader &shader)
    {
        std::ostringstream positionStream, farPlaneStream;
        positionStream << lightName << "." << positionFieldName;
        shader.setVec3(positionStream.str(), position);
        farPlaneStream << lightName << "." << farPlaneFieldName;
        shader.setFloat(farPlaneStream.str(), farPlane);
    }

    void updateShadowMatrices()
    {
        const glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), SHADOW_WIDTH / (float)SHADOW_HEIGHT, nearPlane, farPlane);
        glm::mat4 newShadowMatrices[6] = {
            shadowProj * glm::lookAt(position, position + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            shadowProj * glm::lookAt(position, position + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            shadowProj * glm::lookAt(position, position + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            shadowProj * glm::lookAt(position, position + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            shadowProj * glm::lookAt(position, position + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            shadowProj * glm::lookAt(position, position + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };

        std::copy(newShadowMatrices, newShadowMatrices + 6, shadowMatrices);
    }

public:
    PointLight(const float &x = {0.0f}, const float &y = {0.0f}, const float &z = {0.0f})
    {
        setPosition(glm::vec3(x, y, z));
    }

    void initCubeMap() 
    {
        // Cube map generation
        glGenTextures(1, &cubeMapTextureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
        for (unsigned int i = 0; i < 6; i++) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
            SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubeMapTextureID, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void setPosition(const glm::vec3 &position) { 
        this->position = position; 

        updateShadowMatrices();
    }

    void writeToLightShader(Shader &shader)
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(0.2f));
        shader.setMatrix4f("model", model);
        shader.setVec3("color", ambient);
    }

    virtual void writeToShader(Shader &shader)
    {
        Light::writeToShader(shader);

        writeInnerParams(shader);
    }

    void writeToShaderFromView(Shader &shader, glm::mat4 view)
    {
        Light::writeToShader(shader);

        glm::vec3 viewPosition = glm::vec3(view * glm::vec4(position, 1.0f));
        shader.setVec3(lightName + ".position", viewPosition);
    }

    void writeToDepthShader(Shader &depthShader)
    {
        writeInnerParams(depthShader);

        for (unsigned short i = 0; i < 6; i++) {
            std::ostringstream shadowMatrixStream;
            shadowMatrixStream << shadowMatricesFieldName << "[" << i << "]";
            depthShader.setMatrix4f(shadowMatrixStream.str(), shadowMatrices[i]);
        }
    }

    unsigned int getCubeMapTextureId() { return cubeMapTextureID; }

    void draw()
    {
        modelObj.draw();
    }
};

const PointLight::AttenuationParams PointLight::attenuation {1.0f, 0.7f, 1.8f};