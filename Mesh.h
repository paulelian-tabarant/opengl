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

const std::string materialUniformName {"material"};

const std::string diffuseTexField     {"diffuseTex"     },
                  specularTexField    {"specularTex"    },
                  hasDiffuseTexField  {"hasDiffuseTex"  },
                  hasSpecularTexField {"hasSpecularTex" },
                  ambientColorField   {"ambientColor"   },
                  diffuseColorField   {"diffuseColor"   },
                  specularColorField  {"specularColor"  },
                  shininessField      {"shininess"      };

std::string diffuseTexUniformName,
            specularTexUniformName,
            hasDiffuseTexUniformName,
            hasSpecularTexUniformName,
            ambientColorUniformName,
            diffuseColorUniformName,
            specularColorUniformName,
            shininessUniformName;

class Mesh
{
public:
    struct Material {
        glm::vec3 ambientColor;
        glm::vec3 diffuseColor;
        glm::vec3 specularColor;

        float shininess {100.0f};
    };
private:
    Material material;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
public:

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) :
        vertices(vertices), indices(indices), textures(textures)
    {
        setupMesh();

        // Setup shader material properties
        std::ostringstream uniformStream;
        uniformStream << materialUniformName << "." << diffuseTexField;
        diffuseTexUniformName = uniformStream.str();
        uniformStream = std::ostringstream();
        uniformStream << materialUniformName << "." << specularTexField;
        specularTexUniformName = uniformStream.str();
        uniformStream = std::ostringstream();
        uniformStream << materialUniformName << "." << hasDiffuseTexField;
        hasDiffuseTexUniformName = uniformStream.str();
        uniformStream = std::ostringstream();
        uniformStream << materialUniformName << "." << hasSpecularTexField;
        hasSpecularTexUniformName = uniformStream.str();
        uniformStream = std::ostringstream();
        uniformStream << materialUniformName << "." << ambientColorField;
        ambientColorUniformName = uniformStream.str();
        uniformStream = std::ostringstream();
        uniformStream << materialUniformName << "." << diffuseColorField;
        diffuseColorUniformName = uniformStream.str();
        uniformStream = std::ostringstream();
        uniformStream << materialUniformName << "." << specularColorField;
        specularColorUniformName = uniformStream.str();
        uniformStream = std::ostringstream();
        uniformStream << materialUniformName << "." << shininessField;
        shininessUniformName = uniformStream.str();
    }

    void setMaterial(const Material &mat) { material = mat; }

    void draw(Shader &shader)
    {
        unsigned int diffuseIdx {1}, specularIdx {1};

        shader.setBool(hasDiffuseTexUniformName,  false);
        shader.setBool(hasSpecularTexUniformName, false);

        for (unsigned int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i + 1);
            if (textures[i].type == "diffuse") {
                // i + 1 is because we use texture 0 for shadow mapping
                shader.setInt(diffuseTexUniformName, i + 1);
                glBindTexture(GL_TEXTURE_2D, textures[i].id);
                shader.setBool(hasDiffuseTexUniformName, true);
            }
            else if (textures[i].type == "specular") {
                shader.setInt(specularTexUniformName, i + 1);
                glBindTexture(GL_TEXTURE_2D, textures[i].id);
                shader.setBool(hasSpecularTexUniformName, true);
            }
        }
        // Apply material properties
        shader.setVec3(ambientColorUniformName, material.ambientColor);
        shader.setVec3(diffuseColorUniformName, material.diffuseColor);
        shader.setVec3(specularColorUniformName, material.specularColor);
        shader.setFloat(shininessUniformName, material.shininess);

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
