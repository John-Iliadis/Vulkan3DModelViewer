//
// Created by Gianni on 21/11/2024.
//

#ifndef VULKAN3DMODELVIEWER_CAMERA_HPP
#define VULKAN3DMODELVIEWER_CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class Camera
{
public:
    Camera();
    Camera(float fovy, float width, float height, float near = 0.1f, float far = 100.f);

    void setPosition(float x, float y, float z);
    void setPosition(const glm::vec3& position);
    void setRotation(float angleX, float angleZ);

    void translate(float x, float y, float z);
    void resize(float width, float height);

    const glm::mat4& view() const;
    const glm::mat4& projection() const;
    const glm::mat4& viewProjection() const;
    const glm::vec3& position() const;

private:
    void recalculateView();

    glm::mat4 mView;
    glm::mat4 mProjection;
    glm::mat4 mViewProjection;
    glm::vec3 mPosition;
    float mAngleX;
    float mAngleZ;
    float mFov;
    float mNear;
    float mFar;
};


#endif //VULKAN3DMODELVIEWER_CAMERA_HPP
