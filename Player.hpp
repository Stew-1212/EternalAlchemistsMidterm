/**
 * @file Player.hpp
 * @brief Abstract player class
 */

#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

class Player {
  public:
    virtual ~Player() = default;
    

    virtual void draw() = 0;
    virtual void animate(const GLfloat dTime) = 0;

    virtual void moveForward(const GLfloat speed) final;
    virtual void moveBackward(const GLfloat speed) final;
    virtual void rotate(const GLfloat dTheta, const GLfloat dPhi) final;

    // setters and getters
    virtual GLfloat getPhi() const final { return mPhi; }
    virtual GLfloat getTheta() const final { return mTheta; }
    virtual glm::vec3 getPosition() final {return mPosition; }
    virtual void setPhi(const GLfloat p) final { mPhi = p; _computeOrientation(); }
    virtual void setTheta(const GLfloat t) final { mTheta = t; _computeOrientation(); }
    virtual void setPosition(const glm::vec3 pos) final {mPosition = pos; _computeOrientation(); }

    virtual void setProgramUniformLocations( GLuint shaderProgramHandle, GLint mvpMtxUniformLocation, GLint normalMtxUniformLocation ) final {
    mShaderProgramHandle = shaderProgramHandle;
    mShaderProgramUniformLocations = { mvpMtxUniformLocation, normalMtxUniformLocation };
    }

  private:
    glm::vec3 _worldEdges;
    void _clampPhi();
    void _computeOrientation();

  protected:
    Player();
    glm::vec3 mPosition;
    glm::vec3 mDirection; // normalized direction vector
    GLfloat mPhi;
    GLfloat mTheta;

    virtual void mComputeAndSendMatrixUniforms(const glm::mat4& modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const;

    GLuint mShaderProgramHandle;
    struct ShaderProgramUniformLocations {
      GLint mpvMtx;
      GLint normalMtx;
    } mShaderProgramUniformLocations;
};

inline void Player::moveForward(const GLfloat speed) {
  mPosition += mDirection * speed;
  _clampLocation();
}

inline void Player::moveBackward(const GLfloat speed){
  mPosition -= mDirection * speed;
  _clampLocation();
}

inline void Player::rotate(const GLfloat dTheta, const GLfloat dPhi) {
  mPhi += dPhi;
  mTheta += dTheta;
  _clampPhi();
  _computeOrientation();
}

inline Player::Player() :
  _worldEdges(glm::vec3(5.0f, 5.0f, 5.0f)),
  mPosition(glm::vec3(0,0,0)),
  mDirection(glm::vec3(1,0,0)),
  mPhi(glm::pi<float>() / 2.0f),
  mTheta(0.0f) {
}

inline void Player::_clampPhi() {
    if(mPhi <= 0.0f) mPhi = 0.0f + 0.001f;
    if(mPhi >= glm::pi<float>()) mPhi = glm::pi<float>() - 0.001f;
}

inline void Player::_computeOrientation() {
  // TODO: phi
  mDirection.x = glm::cos(_theta);
  mDirection.z = glm::sin(_theta);

  mDirection = glm::normalize(mDirection);

}

inline void Player::_clampPosition() {
  for (size_t i = 0; i < 3; i++) {
    if (mPosition[i] > _worldEdges[i]) {
      mPosition[i] = _worldEdges[i];
    }
    else if (mPosition[i] < -_worldEdges[i]) {
      mPosition[i] = -_worldEdges[i];
    }
  }
}

void Player::mComputeAndSendMatrixUniforms(const glm::mat4& modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const {
    // precompute the Model-View-Projection matrix on the CPU
    glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
    // then send it to the shader on the GPU to apply to every vertex
    glProgramUniformMatrix4fv( mShaderProgramHandle, mShaderProgramUniformLocations.mvpMtx, 1, GL_FALSE, &mvpMtx[0][0] );

    glm::mat3 normalMtx = glm::mat3( glm::transpose( glm::inverse( modelMtx )));
    glProgramUniformMatrix3fv( mShaderProgramHandle, mShaderProgramUniformLocations.normalMtx, 1, GL_FALSE, &normalMtx[0][0] );
}

#endif // PLAYER_HPP
