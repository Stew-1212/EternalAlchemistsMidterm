#include "Caedilas.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <CSCI441/objects.hpp>
#include <CSCI441/OpenGLUtils.hpp>

Caedilas::Caedilas(
    const GLuint shaderProgramHandle,
    const GLint mvpMtxUniformLocation,
    //const GLint normalMtxUniformLocation,
    const GLint vPos,
    //const GLint vNormal,
    const GLint vTexCoord
) : _shaderProgramHandle(0),
    _shaderProgramUniformLocations({-1} ),
    _location( {1.0f, 1.0f, 0.0f}),
    _direction( {1.0f, 0.0f, 0.0f} ),
    _phi( glm::pi<float>() / 2.0f ),
    _theta(0.0f),
    _worldEdges(5.0f, 5.0f, 5.0f)
{
    setProgramUniformLocations(shaderProgramHandle, mvpMtxUniformLocation);

  _model = new CSCI441::MD5Model();
    //if ( _model->loadMD5Model("assets/models/monsters/hellknight/mesh/hellknight.md5mesh", "assets/models/monsters/hellknight/animations/idle2.md5anim") ) {
    if ( _model->loadMD5Model("assets/models/Caedilas/mesh/Caedilas.md5mesh", "assets/models/Caedilas/animations/move.md5anim") ) {
        _model->allocVertexArrays(vPos, 0, vTexCoord);
    } else {
        fprintf(stderr, "[ERROR]: Could not open MD5 Model\n");
        delete _model;
        _model = nullptr;
    }
}

Caedilas::~Caedilas() {
  delete _model;
  _model = nullptr;
}

void Caedilas::draw(const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    glm::mat4 modelMtx(1.0f);

    modelMtx = glm::translate(modelMtx, _location);
    modelMtx = glm::rotate(modelMtx, -_theta, CSCI441::Y_AXIS);
    //modelMtx = glm::rotate(modelMtx, _phi, CSCI441::Z_AXIS);
    
    // don't hover
    modelMtx = glm::translate(modelMtx, glm::vec3(0.0f, -1.0f, 0.0f));
    // stand upright
    modelMtx = glm::rotate( modelMtx, glm::radians(-90.0f), CSCI441::X_AXIS );
    // face the right direction
    modelMtx = glm::rotate( modelMtx, glm::radians(90.0f), CSCI441::Z_AXIS );
    modelMtx = glm::scale(modelMtx, glm::vec3(2.5f, 2.5f, 2.5f));

    _computeAndSendMatrixUniforms(modelMtx, viewMtx, projMtx);

    //glm::vec3 color(0.0f, 0.0f, 0.0f);

    //glProgramUniform3fv(_shaderProgramHandle, _shaderProgramUniformLocations.materialColor, 1, glm::value_ptr(color));
    _model->draw();
}

void Caedilas::animate(const GLfloat dTime) {
  _model->animate(dTime);
}

void Caedilas::_computeAndSendMatrixUniforms(const glm::mat4& modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const {
    // precompute the Model-View-Projection matrix on the CPU
    glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
    // then send it to the shader on the GPU to apply to every vertex
    glProgramUniformMatrix4fv( _shaderProgramHandle, _shaderProgramUniformLocations.mvpMtx, 1, GL_FALSE, &mvpMtx[0][0] );

    //glm::mat3 normalMtx = glm::mat3( glm::transpose( glm::inverse( modelMtx )));
    //glProgramUniformMatrix3fv( _shaderProgramHandle, _shaderProgramUniformLocations.normalMtx, 1, GL_FALSE, &normalMtx[0][0] );
}
