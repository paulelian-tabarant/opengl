#pragma once

#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#define STB_IMAGE_IMPLEMENTATION ;
#include "stb_image.h"

unsigned int loadTexture(const char *path, const std::string &dir);

class Model
{
public:
    Model(char *path)
    {
        loadModel(path);
    }

    void draw(Shader &shader)
    {
        for (Mesh &mesh : meshes)
            mesh.draw(shader);
    }

private:
    std::vector<Mesh> meshes;
    std::string dir;
    std::vector<Texture> loadedTextures;

private:
    void loadModel(std::string path)
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return;
        }
        dir = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode *node, const aiScene *scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex v;
            v.position[0] = mesh->mVertices[i][0];
            v.position[1] = mesh->mVertices[i][1];
            v.position[2] = mesh->mVertices[i][2];
            v.normal[0] = mesh->mNormals[i][0];
            v.normal[1] = mesh->mNormals[i][1];
            v.normal[2] = mesh->mNormals[i][2];

            if (mesh->mTextureCoords[0]) {
                // A model can have up to 8 different texture coordinates
                // (we use the first one only, at index 0)
                v.texCoords[0] = mesh->mTextureCoords[0][i][0];
                v.texCoords[1] = mesh->mTextureCoords[0][i][1];
            }
            else
                v.texCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(v);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        if (mesh->mMaterialIndex >= 0) {
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
            std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }

        return Mesh(vertices, indices, textures);
    }

    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
    {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);

            bool skip = false;
            // First check if current texture has already been loaded from the path name
            for (unsigned int j = 0; j < loadedTextures.size(); j++) {
                if (std::strcmp(loadedTextures[j].path.data(), str.C_Str()) == 0) {
                    textures.push_back(loadedTextures[j]);
                    skip = true;
                    break;
                }
            }
            if (skip) continue;

            Texture texture;
            std::string filename = str.C_Str();
            filename = filename.substr(filename.find_last_of('\\') + 1, filename.length());
            std::cout << "Loading texture at path " << filename << "...\n";
            texture.id = loadTexture(filename.c_str(), dir);
            texture.type = typeName;
            texture.path = filename;
            textures.push_back(texture);
            // Memorize texture for other potential loadings
            loadedTextures.push_back(texture);
        }
        return textures;
    }

};

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(const char *path, const std::string &dir)
{
    std::string filename(path);
    filename = dir + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << filename << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}