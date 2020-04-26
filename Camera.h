#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

class Camera 
{
private:
    glm::vec3 pos   { glm::vec3(0.0f, 0.0f,  3.0f) };
    glm::vec3 front { glm::vec3(0.0f, 0.0f, -1.0f) };
    glm::vec3 up    { glm::vec3(0.0f, 1.0f,  0.0f) };

    float pitch {0.0f}, yaw {-90.0f};
    float fov {44.9f};
    float zNear {0.1f}, zFar {100.0f};

    float speed {2.5f};
    float sensibility {0.05f};

    std::string camPosStr        {"cameraPos" },
                viewMatStr       {"view"      },
                projectionMatStr {"projection"};

public:

    Camera(const float &x = {0.0f}, const float &y = {0.0f}, const float &z = {0.0f}) :
        pos(glm::vec3(x, y, z)) {}

    void moveFromInput(const float &dz, const float &dx)
    {
        // Front-back movements
        pos += (speed * dz) * front; 
        // Left-right movements
        pos += (speed * dx) * glm::normalize(glm::cross(front, up));
    }

    void rotateFromInput(const float &dx, const float &dy)
    {
        yaw   += dx * sensibility;
        pitch -= dy * sensibility;

        // Convert euler angles to a direction vector for the camera
        float pitchRad = glm::radians(pitch), yawRad = glm::radians(yaw);
        glm::vec3 direction(cos(yawRad) * cos(pitchRad), sin(pitchRad), sin(yawRad) * cos(pitchRad));
        front = glm::normalize(direction);
    }

    void zoomFromScroll(const float &d)
    {
        fov -= d;
        if (fov < 1.0f)
            fov = 1.0f;
        else if (fov > 45.0f)
            fov = 45.0f;
    } 

    void writeToShader(Shader &shader, const unsigned int &widthPx, const unsigned int &heightPx)
    {
        shader.setVec3(camPosStr, pos);
        shader.setMatrix4f(viewMatStr, getViewMatrix());
        shader.setMatrix4f(projectionMatStr, getProjMatrix(widthPx, heightPx));
    }

    glm::vec3 getPosition() { return pos; }
    glm::mat4 getViewMatrix() { return glm::lookAt(pos, pos + front, up); }

private:

    glm::mat4 getProjMatrix(const int &widthPx, const int &heightPx) 
    { 
        return glm::perspective(glm::radians(fov), 
                        widthPx / (float)heightPx, 
                        zNear, zFar);
    }
};