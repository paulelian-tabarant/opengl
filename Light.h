#pragma once

#include "Shader.h"
#include "Object.h"

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

class Light
{
private:
    std::string ambientStr  {"light.ambient"},
                diffuseStr  {"light.diffuse"},
                specularStr {"light.specular"};

    std::string positionStr   {"light.position"},
                directionStr  {"light.direction"},
                innerAngleStr {"light.innerAngle"},
                outerAngleStr {"light.outerAngle"};

    glm::vec3 ambient  {glm::vec3(0.2f, 0.2f, 0.2f)};
    glm::vec3 diffuse  {glm::vec3(0.7f, 0.7f, 0.7f)};
    glm::vec3 specular {glm::vec3(1.0f, 1.0f, 1.0f)};
    glm::vec3 position {glm::vec3(1.2f, 0.5f, 2.0f)};
    glm::vec3 direction {glm::vec3(-0.2f, -0.2f, -1.0f)};
    float innerAngle = 40.0f;
    float outerAngle = 35.0f;

    Object modelObj;

public:
    Light() : modelObj(Object()) {}

    void setPosition(glm::vec3 position)
    {
        this->position = position;
    }

    void setDirection(glm::vec3 direction)
    {
        this->direction = direction;
    }

    void setColor(const glm::vec3& ambient, const glm::vec3& diffuse)
    {
        this->ambient = ambient;
        this->diffuse = diffuse;
    }

    void writeModelMatrixInShader(Shader &shader, std::string name)
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(0.2f));
        shader.setMatrix4f(name, model);
    }

    void writeToShader(Shader &shader)
    {
        shader.setVec3(ambientStr, ambient);
        shader.setVec3(diffuseStr, diffuse);
        shader.setVec3(specularStr, specular);
        shader.setVec3(positionStr, position);
        shader.setVec3(directionStr, direction);
        shader.setFloat(innerAngleStr, glm::radians(innerAngle));
        shader.setFloat(outerAngleStr, glm::radians(outerAngle));
    }

    void draw()
    {
        modelObj.draw();
    }
};