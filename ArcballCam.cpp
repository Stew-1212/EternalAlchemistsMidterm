#include "ArcballCam.h"

ArcballCam::ArcballCam(glm::vec3 target, GLfloat radius) {
    _target = target;
    mCameraRadius = radius;
    mCameraTheta = glm::pi<float>() / 2.0f;
    mCameraPhi = glm::pi<float>() / 2.0f;

    recomputeOrientation();
}

void ArcballCam::recomputeOrientation() {
    //convert spherical to Cartestian
    mCameraPosition = glm::vec3(
        _target.x + mCameraRadius * sin(mCameraPhi) * cos(mCameraTheta),
        _target.y + mCameraRadius * cos(mCameraPhi),
        _target.z + mCameraRadius * sin(mCameraPhi) * sin(mCameraPhi) * sin(mCameraTheta)
    );

    mCameraLookAtPoint = _target;
    mCameraUpVector = glm::vec3(0.0f, 1.0f, 0.0f);

    computeViewMatrix();
}

void ArcballCam::rotateTheta(GLfloat dTheta) {
    mCameraTheta += dTheta;
}

void ArcballCam::rotatePhi(GLfloat dPhi) {
    mCameraPhi += dPhi;

    //clamp phi to avoid flipping upside-down
    const GLfloat epsilon = 0.001f;
    mCameraPhi = glm::clamp(mCameraPhi, epsilon, glm::pi<GLfloat>() - epsilon);
}

void ArcballCam::moveForward(GLfloat movementFactor) {
    //zoom in: reduce radius
    mCameraRadius -= movementFactor;
    if (mCameraRadius < 0.1f) mCameraRadius = 0.1f; //prevent flipping
    recomputeOrientation();
}

void ArcballCam::moveBackward(GLfloat movementFactor) {
    //zoom out: increase radius
    mCameraRadius += movementFactor;
    recomputeOrientation();
}

void ArcballCam::setTarget(const glm::vec3& target) {
    _target = target;
    recomputeOrientation();
}

glm::vec3 ArcballCam::getTarget() const {
    return _target;
}