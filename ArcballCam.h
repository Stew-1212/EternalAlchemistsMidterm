#ifndef ARCBALLCAM_H
#define ARCBALLCAM_H

#include <CSCI441/Camera.hpp>

class ArcballCam : public CSCI441::Camera {
    public:
        /**
         * making an Arcball camera with initial paramters
         * 
         */
        ArcballCam(glm::vec3 target = glm::vec3(0.0f), GLfloat radius = 10.0f);

        //required overrides
        void recomputeOrientation() override;
        void moveForward(GLfloat movementFactor) override;
        void moveBackward(GLfloat movementFactor) override;
        //methods we need
        void rotateTheta(GLfloat dTheta);
        void rotatePhi(GLfloat dPhi);
        void setTarget(const glm::vec3& target);
        glm::vec3 getTarget() const;
    private:
        glm::vec3 _target;
};



#endif