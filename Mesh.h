#pragma once

#include "Shader.h"

#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    float shininess {100.0f};

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) :
        vertices(vertices), indices(indices), textures(textures)
    {
        setupMesh();
    }

    void setSpecularShininess(const float &s) { shininess = s; }

    void draw(Shader &shader)
    {
        unsigned int diffuseIdx {1}, specularIdx {1};

        shader.setBool("material.hasDiffuse",  false);
        shader.setBool("material.hasSpecular", false);

        for (unsigned int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i + 1);
            std::ostringstream texUniformName;
            texUniformName << "material.";
            if (textures[i].type == "diffuse") {
                texUniformName << "diffuse";
                shader.setBool("material.hasDiffuse", true);
            }
            else if (textures[i].type == "specular") {
                texUniformName << "specular";
                shader.setBool("material.hasSpecular", true);
            }
            shader.setInt(texUniformName.str(), i + 1);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
        shader.setFloat("material.shininess", shininess);

        // Detach lastly used texture
        glActiveTexture(GL_TEXTURE0);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        // Detach verex array after use
        glBindVertexArray(0);
    }

private:
    unsigned int VAO, VBO, EBO;
private:
    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

        // Detach vertex array
        glBindVertexArray(0);
    }
};
