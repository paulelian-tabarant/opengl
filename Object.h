#pragma once

#include <glm/glm.hpp>

#include "Shader.h"

class Object
{
private:
    unsigned int VAO;
    unsigned int VBO;
    std::string modelMatStr  {"model"},
                ambientStr   {"material.ambient"},
                diffuseStr   {"material.diffuse"},
                specularStr  {"material.specular"},
                shininessStr {"material.shininess"};

    glm::vec3 pos { glm::vec3(0.0f) };
    float rot {0.0f};
    glm::vec3 rotAxis { glm::vec3(1.0f, 0.3f, 0.5f) };

    glm::vec3 ambient {glm::vec3(0.25f, 0.20725f, 0.20725f)};
    glm::vec3 diffuse {glm::vec3(1.0f, 0.829f, 0.829f)};
    glm::vec3 specular {glm::vec3(0.296648f, 0.296648f, 0.296648f)};
    float shininess {0.088f * 128.0};

    float vertices[216] =
        { -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
           0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
           0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
           0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
          -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
          -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

          -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
           0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
           0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
           0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
          -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
          -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

          -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
          -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
          -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
          -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
          -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
          -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

           0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
           0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
           0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
           0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
           0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
           0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

          -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
           0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
           0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
           0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
          -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
          -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

          -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
           0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
           0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
           0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
          -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
          -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f };

public:
    Object()
    {
        // initialize vertex attributs layout
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        // Prepare GPU buffer for receiving 3D positions
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // Vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
    }

    ~Object()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    void setPosition(glm::vec3 position)
    {
        pos = position;
    }

    void setAngle(float angle)
    {
        rot = angle;
    }

    void setMaterial(glm::vec3 &a, glm::vec3 &d, glm::vec3 s = glm::vec3(1.0f), float shin = 64.0f)
    {
        ambient = a;
        diffuse = d;
        specular = s;
        shininess = shin;
    }

    void writeToShader(Shader &shader)
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, pos);
        model = glm::rotate(model, glm::radians(rot), rotAxis);
        shader.setMatrix4f(modelMatStr, model);
        shader.setVec3(ambientStr, ambient);
        shader.setVec3(diffuseStr, diffuse);
        shader.setVec3(specularStr, specular);
        shader.setFloat(shininessStr, shininess);
    }

    void draw()
    {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
};