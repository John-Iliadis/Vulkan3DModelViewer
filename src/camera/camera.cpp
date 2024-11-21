//
// Created by Gianni on 21/11/2024.
//

#include "camera.hpp"


Camera::Camera()
    : mView(1.f)
    , mProjection(1.f)
    , mViewProjection(1.f)
    , mPosition()
    , mAngleX()
    , mAngleZ()
    , mFov()
    , mNear()
    , mFar()
{
}

Camera::Camera(float fovy, float width, float height, float near, float far)
    : mView(1.f)
    , mProjection(glm::perspective(fovy, width / height, near, far))
    , mViewProjection(mProjection)
    , mPosition()
    , mAngleX()
    , mAngleZ()
    , mFov(fovy)
    , mNear(near)
    , mFar(far)
{
}

void Camera::setPosition(float x, float y, float z)
{
    setPosition({x, y, z});
}

void Camera::setPosition(const glm::vec3 &position)
{
    mPosition = position;
    recalculateView();
}

void Camera::setRotation(float angleX, float angleZ)
{
    mAngleX = angleX;
    mAngleZ = angleZ;
    recalculateView();
}

void Camera::resize(float width, float height)
{
    mProjection = glm::perspective(mFov, width / height, mNear, mFar);
    mViewProjection = mProjection * mView;
}

const glm::mat4 &Camera::view() const
{
    return mView;
}

const glm::mat4 &Camera::projection() const
{
    return mProjection;
}

const glm::mat4 &Camera::viewProjection() const
{
    return mViewProjection;
}

const glm::vec3 &Camera::position() const
{
    return mPosition;
}

void Camera::recalculateView()
{
    glm::mat4 transform = glm::translate(glm::mat4(1.f), mPosition);
    transform = glm::rotate(transform, mAngleX, glm::vec3(1.f, 0.f, 0.f));
    transform = glm::rotate(transform, mAngleZ, glm::vec3(0.f, 0.f, 1.f));

    mView = glm::inverse(transform);
    mViewProjection = mProjection * mView;
}
