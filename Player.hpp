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
    

    /// \param modelMtx existing model matrix to apply to Caedilas
    /// \param viewMtx camera view matrix to apply to Caedilas
    /// \param projMtx camera projection matrix to apply to Caedilas
    /// \note internally uses the provided shader program and sets the necessary uniforms
    /// for the MVP and Normal Matrices as well as the material diffuse color
    virtual void draw(const glm::mat4& viewMtx, const glm::mat4& projMtx ) const = 0;
    virtual void animate(const GLfloat dTime) = 0;

    virtual void moveForward(const GLfloat speed) final;
    virtual void moveBackward(const GLfloat speed) final;
    virtual void rotate(const GLfloat dTheta, const GLfloat dPhi) final;

    // set world boundaries (assumed to be symmetrical from 0,0,0)
    virtual void setWorldEdges(const GLfloat x, const GLfloat y, const GLfloat z) final {
      _worldEdges = glm::vec3(x, y, z);
    }

    // setters and getters
    virtual GLfloat getPhi() const final { return mPhi; }
    virtual GLfloat getTheta() const final { return mTheta; }
    virtual glm::vec3 getPosition() final {return mPosition; }
    virtual void setPhi(const GLfloat p) final { mPhi = p; _computeOrientation(); }
    virtual void setTheta(const GLfloat t) final { mTheta = t; _computeOrientation(); }
    virtual void setPosition(const glm::vec3 pos) final {mPosition = pos; _computeOrientation(); }

    /// \param shaderProgramHandle shader program handle that the Caedilas should be drawn using
    /// \param mvpMtxUniformLocation uniform location for the full precomputed MVP matrix
    /// \param normalMtxUniformLocation uniform location for the precomputed Normal matrix
    /// \param materialColorUniformLocation uniform location for the material diffuse color
    virtual void setProgramUniformLocations( GLuint shaderProgramHandle, GLint mvpMtxUniformLocation, GLint normalMtxUniformLocation ) final {
    mShaderProgramHandle = shaderProgramHandle;
    mShaderProgramUniformLocations = { mvpMtxUniformLocation, normalMtxUniformLocation };
    }

  private:
    glm::vec3 _worldEdges;
    void _clampPhi();
    // \desc keep player inside of world boundaries
    void _computeOrientation();
    void _clampPosition();

  protected:
    // helpers
    static constexpr GLfloat s_PI = glm::pi<float>();
    static constexpr GLfloat s_2PI = glm::two_pi<float>();
    static constexpr GLfloat s_PI_OVER_2 = glm::half_pi<float>();

    Player();
    glm::vec3 mPosition;
    glm::vec3 mDirection; // normalized direction vector
    GLfloat mPhi;
    GLfloat mTheta;

    
    virtual void mComputeAndSendMatrixUniforms(const glm::mat4& modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const;

    /// \desc handle of the shader program to use when drawing
    GLuint mShaderProgramHandle;
    /// \desc stores the uniform locations needed
    struct ShaderProgramUniformLocations {
      /// \desc location of the precomputed ModelViewProjection matrix
      GLint mvpMtx;
      /// \desc location of the precomputed Normal matrix
      GLint normalMtx;
    } mShaderProgramUniformLocations;
};

inline void Player::moveForward(const GLfloat speed) {
  mPosition += mDirection * speed;
  _clampPosition();
}

inline void Player::moveBackward(const GLfloat speed){
  mPosition -= mDirection * speed;
  _clampPosition();
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
  mDirection.x = glm::cos(mTheta);
  mDirection.z = glm::sin(mTheta);

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

inline void Player::mComputeAndSendMatrixUniforms(const glm::mat4& modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const {
    // precompute the Model-View-Projection matrix on the CPU
    glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
    // then send it to the shader on the GPU to apply to every vertex
    glProgramUniformMatrix4fv( mShaderProgramHandle, mShaderProgramUniformLocations.mvpMtx, 1, GL_FALSE, &mvpMtx[0][0] );

    glm::mat3 normalMtx = glm::mat3( glm::transpose( glm::inverse( modelMtx )));
    glProgramUniformMatrix3fv( mShaderProgramHandle, mShaderProgramUniformLocations.normalMtx, 1, GL_FALSE, &normalMtx[0][0] );
}

#endif // PLAYER_HPP
