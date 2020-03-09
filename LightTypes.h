#pragma once

#include "Light.h"
#include "Shader.h"
#include "Object.h"

#include <glm/glm.hpp>

class DirLight : public Light
{
private:
    std::string directionFieldName {"direction"};
    glm::vec3 direction;

public:
    DirLight(float x, float y, float z) : direction(glm::vec3(x,y,z)) {}

    void setDirection(glm::vec3 direction)
    {
        this->direction = direction;
    }

    void writeToShader(Shader &shader)
    {
        Light::writeToShader(shader);

        std::ostringstream directionStream;
        directionStream << lightName << "." << directionFieldName;
        shader.setVec3(directionStream.str(), direction);
    }
};

class PointLight : public Light
{
private:
    std::string positionFieldName {"position"};
    glm::vec3 position;

    Object modelObj;

public:
    PointLight(float x, float y, float z) : position(glm::vec3(x,y,z)) {}

    void setPosition(const glm::vec3 &position)
    {
        this->position = position;
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
        Light::writeToShader(shader);

        std::ostringstream positionStream;
        positionStream << lightName << "." << positionFieldName;
        shader.setVec3(positionStream.str(), position);
    }

    void draw()
    {
        modelObj.draw();
    }
};