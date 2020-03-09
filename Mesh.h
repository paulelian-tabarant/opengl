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

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) :
        vertices(vertices), indices(indices), textures(textures)
    {
        setupMesh();
    }

    void draw(Shader &shader)
    {
        unsigned int diffuseIdx {1}, specularIdx {1};
        for (unsigned int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            std::ostringstream texUniformName;
            texUniformName << "material.";
            if (textures[i].type == "diffuse")
                texUniformName << "diffuse"<<diffuseIdx++;
            else if (textures[i].type == "specular")
                texUniformName << "specular"<<specularIdx++;
            shader.setFloat(texUniformName.str(), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
        shader.setFloat("material.shininess", 30.0f);

        // Detach lastly used texture
        glActiveTexture(GL_TEXTURE0);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        // Detach vertex array after use
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
