#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

class Camera 
{
private:
    glm::vec3 pos   { glm::vec3(0.0f, 0.0f,  0.0f) };
    glm::vec3 front { glm::vec3(0.0f, 0.0f, -1.0f) };
    glm::vec3 up    { glm::vec3(0.0f, 1.0f,  0.0f) };

    float pitch {0.0f}, yaw {-90.0f};
    float fov {44.9f};
    float zNear {0.1f}, zFar {100.0f};

    float speed {2.5f};
    float sensibility {0.05f};

    std::string camPosStr {"cameraPos"};
    std::string viewMatStr {"view"};
    std::string projectionMatStr {"projection"};

public:
    void moveFromInput(float dz, float dx)
    {
        // Front-back movements
        pos += (speed * dz) * front; 
        // Left-right movements
        pos += (speed * dx) * glm::normalize(glm::cross(front, up));
    }

    void rotateFromInput(float dx, float dy)
    {
        dx *= sensibility;
        dy *= sensibility;

        yaw += dx;
        pitch -= dy;

        // Convert euler angles to a direction vector for the camera
        float pitchRad = glm::radians(pitch), yawRad = glm::radians(yaw);
        glm::vec3 direction(cos(yawRad) * cos(pitchRad), sin(pitchRad), sin(yawRad) * cos(pitchRad));
        front = glm::normalize(direction);
    }

    void zoomFromScroll(float d)
    {
        fov -= d;
        if (fov < 1.0f)
            fov = 1.0f;
        else if (fov > 45.0f)
            fov = 45.0f;
    } 

    void writeToShader(Shader &shader, const unsigned int widthPx, const unsigned int heightPx)
    {
        shader.setVec3(camPosStr, pos);
        shader.setMatrix4f(viewMatStr, this->getViewMatrix());
        shader.setMatrix4f(projectionMatStr, this->getProjMatrix(widthPx, heightPx));
    }

private:
    glm::mat4 getViewMatrix() { return glm::lookAt(pos, pos + front, up); }

    glm::mat4 getProjMatrix(int widthPx, int heightPx) 
    { 
        return glm::perspective(glm::radians(fov), 
                        widthPx / (float)heightPx, 
                        zNear, zFar);
    }
};