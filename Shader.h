#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader 
{
private:
    static const std::string baseDir;

public:
    // Program ID
    unsigned int ID;

    Shader(const GLchar *vertexPath, const GLchar *geometryPath, const GLchar *fragmentPath) {

        // Read shader source files and convert them into strings
        std::string vertexCode;
        std::string geometryCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream gShaderFile;
        std::ifstream fShaderFile;
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            vShaderFile.open(baseDir + vertexPath);
            fShaderFile.open(baseDir + fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            vShaderFile.close();
            fShaderFile.close();
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
            if (geometryPath != "") {
                gShaderFile.open(baseDir + geometryPath);
                std::stringstream gShaderStream;
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();
            }
        }
        catch(std::ifstream::failure e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
        }
        const char *vShaderCode = vertexCode.c_str();
        const char *gShaderCode = geometryCode.c_str();
        const char *fShaderCode = fragmentCode.c_str();

        // Create and compile shaders
        unsigned int vertex, geometry, fragment;
        int success;
        char infoLog[512];

        // Vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // Geometry shader
        bool hasGeometryShader = geometryCode.size() > 0;
        if (hasGeometryShader) {
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(vertex, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
            }
        }
        // Fragment shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragment, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        // Create shader program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        if (hasGeometryShader) glAttachShader(ID, geometry);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(ID, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        // Shader objects are useless once linked to the program
        glDeleteShader(vertex);
        if (hasGeometryShader) glDeleteShader(geometry);
        glDeleteShader(fragment);
    }

    void use() {
        glUseProgram(ID);
    }

    void setBool(const std::string &name, const bool &value) {         
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }
    void setInt(const std::string &name, const int &value) { 
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }
    void setFloat(const std::string &name, float value) { 
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    } 

    void setVec3(const std::string &name, const glm::vec3 &value) {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), value[0], value[1], value[2]);
    }

    void setVec4(const std::string &name, const glm::vec4 &value) {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), value[0], value[1], value[2], value[3]);
    }

    void setMatrix4f(const std::string &name, const glm::mat4 &value) {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }
};

const std::string Shader::baseDir {"Shaders/"};