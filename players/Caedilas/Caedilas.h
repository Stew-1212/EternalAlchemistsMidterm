#ifndef LAB05_Caedilas_H
#define LAB05_Caedilas_H

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <CSCI441/MD5Model.hpp>

#include "../../Player.hpp"

class Caedilas : public Player  {
public:
    /// \desc creates a simple Caedilas that gives the appearance of flight
    /// \param shaderProgramHandle shader program handle that the Caedilas should be drawn using
    /// \param mvpMtxUniformLocation uniform location for the full precomputed MVP matrix
    /// \param normalMtxUniformLocation uniform location for the precomputed Normal matrix
    /// \param materialColorUniformLocation uniform location for the material diffuse color
    Caedilas( GLuint shaderProgramHandle, GLint mvpMtxUniformLocation, GLint normalMtxUniformLocation, GLint vPos, GLint vNormal, GLint vTexCoord );

    ~Caedilas();

    /// \desc draws the model Caedilas for a given MVP matrix
    /// \param modelMtx existing model matrix to apply to Caedilas
    /// \param viewMtx camera view matrix to apply to Caedilas
    /// \param projMtx camera projection matrix to apply to Caedilas
    /// \note internally uses the provided shader program and sets the necessary uniforms
    /// for the MVP and Normal Matrices as well as the material diffuse color
    void draw(const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;
    void animate(const GLfloat dTime);

private:
    CSCI441::MD5Model* _model;

    /// \desc precomputes the matrix uniforms CPU-side and then sends them
    /// to the GPU to be used in the shader for each vertex.  It is more efficient
    /// to calculate these once and then use the resultant product in the shader.
    /// \param modelMtx model transformation matrix
    /// \param viewMtx camera view matrix
    /// \param projMtx camera projection matrix
    void _computeAndSendMatrixUniforms(const glm::mat4& modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const;
};


#endif//LAB05_Caedilas_H
