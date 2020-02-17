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

    std::string posStr {"light.position"};

    glm::vec3 ambient  {glm::vec3(0.2f, 0.2f, 0.2f)};
    glm::vec3 diffuse  {glm::vec3(0.5f, 0.5f, 0.5f)};
    glm::vec3 specular {glm::vec3(1.0f, 1.0f, 1.0f)};
    glm::vec3 pos {glm::vec3(1.2f, 1.0f, 2.0f)};

    Object modelObj;

public:
    Light() : modelObj(Object()) {}

    void setPosition(glm::vec3 pos)
    {
        this->pos = pos;
    }

    void setColor(const glm::vec3& ambient, const glm::vec3& diffuse)
    {
        this->ambient = ambient;
        this->diffuse = diffuse;
    }

    void writeModelMatrixInShader(Shader &shader, std::string name)
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, pos);
        model = glm::scale(model, glm::vec3(0.2f));
        shader.setMatrix4f(name, model);
    }

    void writeToShader(Shader &shader)
    {
        shader.setVec3(ambientStr, ambient);
        shader.setVec3(diffuseStr, diffuse);
        shader.setVec3(specularStr, specular);
        shader.setVec3(posStr, pos);
    }

    void draw()
    {
        modelObj.draw();
    }
};