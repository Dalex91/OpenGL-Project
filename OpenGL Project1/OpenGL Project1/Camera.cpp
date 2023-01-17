#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        this->cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        cameraTarget = cameraPosition + cameraFrontDirection;
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
        displayCameraParameters();
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    void Camera::displayCameraParameters() {
        //printf("\n");
         //printf("Camera position: %f %f %f\n", cameraPosition.x, cameraPosition.y, cameraPosition.z);
         //printf("Camera target: %f %f %f\n", cameraTarget.x, cameraTarget.y, cameraTarget.z);
         //printf("Camera up: %f %f %f\n", cameraUpDirection.x, cameraUpDirection.y, cameraUpDirection.z);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {

        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));

        switch (direction) {
            case MOVE_FORWARD:
                cameraPosition += cameraFrontDirection * speed;
                displayCameraParameters();
                return;
            case MOVE_BACKWARD:
                cameraPosition -= cameraFrontDirection * speed;
                displayCameraParameters();
                return;
            case MOVE_RIGHT:
                cameraPosition += cameraRightDirection * speed;
                displayCameraParameters();
                return;
            case MOVE_LEFT:
                cameraPosition -= cameraRightDirection * speed;
                displayCameraParameters();
                return;
            case MOVE_DOWN:
                cameraPosition -= cameraUpDirection * speed;
                displayCameraParameters();
                return;
            case MOVE_UP:
                cameraPosition += cameraUpDirection * speed;
                displayCameraParameters();
                return;
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        cameraFrontDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFrontDirection.y = sin(glm::radians(pitch));
        cameraFrontDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        this->cameraFrontDirection = glm::normalize(cameraFrontDirection);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        this->cameraTarget = cameraPosition + cameraFrontDirection;
        displayCameraParameters();
    }
}