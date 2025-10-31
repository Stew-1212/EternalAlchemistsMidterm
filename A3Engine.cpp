#include "A3Engine.h"

#include <CSCI441/FreeCam.hpp>
#include <CSCI441/objects.hpp>

#include <glm/gtc/constants.hpp> // for glm::pi()
#include <glm/gtc/type_ptr.hpp>  // for glm::value_ptr()

#include <cstdlib>
#include <ctime>
#include <iostream>

//*************************************************************************************
//
// Helper Functions

/// \desc Simple helper function to return a random number between 0.0f and 1.0f.
GLfloat getRand() {
    return static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX);
}

//*************************************************************************************
//
// Public Interface

A3Engine::A3Engine()
    : CSCI441::OpenGLEngine(4, 1, 640, 480, "a3: Flight Simulator v0.41 alpha"),
    _mousePosition( {MOUSE_UNINITIALIZED, MOUSE_UNINITIALIZED} ),
    _leftMouseButtonState(GLFW_RELEASE),
    _pMainCam(nullptr),
    _cameraSpeed( {0.0f, 0.0f} ),
    _pCaedilas(nullptr),
    _groundVAO(0),
    _numGroundPoints(0),
    _lightingShaderProgram(nullptr),
    _lightingShaderUniformLocations( {-1, -1} ),
    _lightingShaderAttributeLocations( {-1} ),
    _textureShaderProgram(nullptr),
    _textureShaderUniformLocations( {-1, -1, -1, -1} ),
    _textureShaderAttributeLocations( {-1, -1} )
{
    for(auto& _key : _keys) _key = GL_FALSE;
}


A3Engine::~A3Engine() {
    delete _pMainCam;
    delete _pCaedilas;
    delete _lightingShaderProgram;
    delete _textureShaderProgram;
}

void A3Engine::handleKeyEvent(const GLint KEY, const GLint ACTION) {
    if(KEY != GLFW_KEY_UNKNOWN)
        _keys[KEY] = ((ACTION == GLFW_PRESS) || (ACTION == GLFW_REPEAT));

    if(ACTION == GLFW_PRESS) {
        switch( KEY ) {
            // quit!
            case GLFW_KEY_Q:
            case GLFW_KEY_ESCAPE:
                setWindowShouldClose();
                break;

            case GLFW_KEY_R:
                mReloadShaders();
                _setLightingParameters();
                _pCaedilas->setProgramUniformLocations(
                    _textureShaderProgram->getShaderProgramHandle(),
                    _textureShaderUniformLocations.mvpMatrix
                );
                break;

            default: break; // suppress CLion warning
        }
    }
}

void A3Engine::handleMouseButtonEvent(const GLint BUTTON, const GLint ACTION) {
    // if the event is for the left mouse button
    if( BUTTON == GLFW_MOUSE_BUTTON_LEFT ) {
        // update the left mouse button's state
        _leftMouseButtonState = ACTION;
    }
}

void A3Engine::handleCursorPositionEvent(const glm::vec2 currMousePosition) {
    // if mouse hasn't moved in the window, prevent camera from flipping out
    if(_mousePosition.x == MOUSE_UNINITIALIZED) {
        _mousePosition = currMousePosition;
    }

    // if the left mouse button is being held down while the mouse is moving
    if(_leftMouseButtonState == GLFW_PRESS) {
        if( _keys[GLFW_KEY_LEFT_SHIFT] || _keys[GLFW_KEY_RIGHT_SHIFT] ) {
          _pMainCam->moveForward((currMousePosition.x - _mousePosition.x + _mousePosition.y - currMousePosition.y) * 0.5f);
        }
        // rotate the camera by the distance the mouse moved
        else {
          _pMainCam->rotate((currMousePosition.x - _mousePosition.x) * 0.005f,
              (_mousePosition.y - currMousePosition.y) * 0.005f );
        }
    }

    // update the mouse position
    _mousePosition = currMousePosition;
}

//*************************************************************************************
//
// Engine Setup

void A3Engine::mSetupGLFW() {
    CSCI441::OpenGLEngine::mSetupGLFW();

    // set our callbacks
    glfwSetKeyCallback(mpWindow, a3_engine_keyboard_callback);
    glfwSetMouseButtonCallback(mpWindow, a3_engine_mouse_button_callback);
    glfwSetCursorPosCallback(mpWindow, a3_engine_cursor_callback);
}

void A3Engine::mSetupOpenGL() {
    glEnable( GL_DEPTH_TEST );					                        // enable depth testing
    glDepthFunc( GL_LESS );							                // use less than depth test

    glEnable(GL_BLEND);									            // enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	    // use one minus blending equation

    glClearColor( 0.4f, 0.4f, 0.4f, 1.0f );	        // clear the frame buffer to gray
}

void A3Engine::mSetupShaders() {
  // material color shaders
    _lightingShaderProgram = new CSCI441::ShaderProgram("shaders/solid.v.glsl", "shaders/solid.f.glsl" );
    // assign uniforms
    _lightingShaderUniformLocations.mvpMatrix      = _lightingShaderProgram->getUniformLocation("mvpMatrix");
    _lightingShaderUniformLocations.materialColor  = _lightingShaderProgram->getUniformLocation("materialColor");
    _lightingShaderUniformLocations.lightDirection = _lightingShaderProgram->getUniformLocation("lightDirection");
    _lightingShaderUniformLocations.lightColor = _lightingShaderProgram->getUniformLocation("lightColor");
    _lightingShaderUniformLocations.normalMatrix = _lightingShaderProgram->getUniformLocation("normalMatrix");

    // attributes
    _lightingShaderAttributeLocations.vPos         = _lightingShaderProgram->getAttributeLocation("vPos");
    _lightingShaderAttributeLocations.vNormal         = _lightingShaderProgram->getAttributeLocation("vNormal");


    // texture shaders
    _textureShaderProgram = new CSCI441::ShaderProgram("shaders/texture.v.glsl", "shaders/texture.f.glsl" );
    // uniforms
    _textureShaderUniformLocations.mvpMatrix      = _textureShaderProgram->getUniformLocation("mvpMatrix");
    _textureShaderUniformLocations.texMap = _textureShaderProgram->getUniformLocation("texMap");
    _textureShaderUniformLocations.lightDirection = _lightingShaderProgram->getUniformLocation("lightDirection");
    _textureShaderUniformLocations.lightColor = _lightingShaderProgram->getUniformLocation("lightColor");

    // attribute locations
    _textureShaderAttributeLocations.vPos         = _textureShaderProgram->getAttributeLocation("vPos");
    //_textureShaderAttributeLocations.vNormal      = _textureShaderProgram->getAttributeLocation("vNormal");
    _textureShaderAttributeLocations.vTexCoord = _textureShaderProgram->getAttributeLocation("vTexCoord");

    // static uniform
    _textureShaderProgram->setProgramUniform(_textureShaderUniformLocations.texMap, 0);
}

void A3Engine::mSetupBuffers() {
    _pCaedilas = new Caedilas(_textureShaderProgram->getShaderProgramHandle(),
                        _textureShaderUniformLocations.mvpMatrix,
                        //_textureShaderUniformLocations.normalMatrix,
                        _textureShaderAttributeLocations.vPos,
                        //_textureShaderAttributeLocations.vNormal,
                        _textureShaderAttributeLocations.vTexCoord);

    _pCaedilas->setWorldEdges(WORLD_SIZE, 1.0f, WORLD_SIZE);
    _createGroundBuffers();
    _generateEnvironment();
}

void A3Engine::_createGroundBuffers() {
    // TODO #8: expand our struct
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;

    };

    // TODO #9: add normal data
    constexpr Vertex groundQuad[4] = {
            { {-1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f} },
            { { 1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
            { {-1.0f, 0.0f,  1.0f}, {0.0f, 1.0f, 0.0f}},
            { { 1.0f, 0.0f,  1.0f}, {0.0f, 1.0f, 0.0f}}
    };

    constexpr GLushort indices[4] = {0,1,2,3};

    _numGroundPoints = 4;

    glGenVertexArrays(1, &_groundVAO);
    glBindVertexArray(_groundVAO);

    GLuint vbods[2];       // 0 - VBO, 1 - IBO
    glGenBuffers(2, vbods);
    glBindBuffer(GL_ARRAY_BUFFER, vbods[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundQuad), groundQuad, GL_STATIC_DRAW);

    glEnableVertexAttribArray(_lightingShaderAttributeLocations.vPos);
    glVertexAttribPointer(_lightingShaderAttributeLocations.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)nullptr);

    // TODO #10: hook up vertex normal attribute
    glEnableVertexAttribArray(_lightingShaderAttributeLocations.vNormal);
    glVertexAttribPointer(_lightingShaderAttributeLocations.vNormal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float)*3));


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbods[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void A3Engine::_generateEnvironment() {
    //******************************************************************
    // parameters to make up our grid size and spacing, feel free to
    // play around with this
    constexpr GLfloat GRID_WIDTH = WORLD_SIZE * 1.8f;
    constexpr GLfloat GRID_LENGTH = WORLD_SIZE * 1.8f;
    constexpr GLfloat GRID_SPACING_WIDTH = 1.0f;
    constexpr GLfloat GRID_SPACING_LENGTH = 1.0f;
    // precomputed parameters based on above
    constexpr GLfloat LEFT_END_POINT = -GRID_WIDTH / 2.0f - 5.0f;
    constexpr GLfloat RIGHT_END_POINT = GRID_WIDTH / 2.0f + 5.0f;
    constexpr GLfloat BOTTOM_END_POINT = -GRID_LENGTH / 2.0f - 5.0f;
    constexpr GLfloat TOP_END_POINT = GRID_LENGTH / 2.0f + 5.0f;
    //******************************************************************

    srand( time(0) );                                                   // seed our RNG

    // psych! everything's on a grid.
    for(int i = LEFT_END_POINT; i < RIGHT_END_POINT; i += GRID_SPACING_WIDTH) {
        for(int j = BOTTOM_END_POINT; j < TOP_END_POINT; j += GRID_SPACING_LENGTH) {
            // don't just draw a building ANYWHERE.
            if( i % 2 && j % 2 && getRand() < 0.2f ) {
                // translate to spot
                glm::mat4 transToSpotMtx = glm::translate( glm::mat4(1.0), glm::vec3(i, 0.0f, j) );

                // compute random height
                GLdouble height = powf(getRand(), 2.5)*10 + 1;
                // scale to building size
                glm::mat4 scaleToHeightMtx = glm::scale( glm::mat4(1.0), glm::vec3(1, height, 1) );

                // translate up to grid
                glm::mat4 transToHeight = glm::translate( glm::mat4(1.0), glm::vec3(0, height/2.0f, 0) );

                // compute full model matrix
                glm::mat4 modelMatrix = transToHeight * scaleToHeightMtx * transToSpotMtx;

                // compute random color
                glm::vec3 color( getRand(), getRand(), getRand() );
                // store building properties
                BuildingData currentBuilding = {modelMatrix, color};
                _buildings.emplace_back( currentBuilding );
            }
        }
    }
}

void A3Engine::mSetupScene() {
    _pMainCam = new ArcBallCam();
    _pMainCam->setTheta(glm::pi<float>() / -3.0f );
    _pMainCam->setPhi(glm::pi<float>() / 1.5f);
    _pMainCam->setLookAtPoint(_pCaedilas->getLocation() + 1.0f);
    _pMainCam->recomputeOrientation();

    _pSecondaryCam = new ArcBallCam();
    _pSecondaryCam->setTheta(_pCaedilas->getTheta() + glm::radians(-90.0f));
    _pSecondaryCam->setPhi(glm::pi<float>() / 2.0f);
    _pSecondaryCam->setLookAtPoint(_pCaedilas->getLocation() + 1.0f);
    _pSecondaryCam->recomputeOrientation();

    _cameraSpeed = glm::vec2(0.25f, 0.02f);
    _playerSpeed = glm::vec2(0.25f, 0.02f);

    _setLightingParameters();
}

void A3Engine::_setLightingParameters() {
  glm::vec3 lightDirection(-1, -1, -1);
  glm::vec3 lightColor(1, 1, 1);

  // send to material color shaders
  glProgramUniform3fv(
      _lightingShaderProgram->getShaderProgramHandle(),
      _lightingShaderUniformLocations.lightDirection,
      1,
      glm::value_ptr(lightDirection)
      );

  glProgramUniform3fv(
      _lightingShaderProgram->getShaderProgramHandle(),
      _lightingShaderUniformLocations.lightColor,
      1,
      glm::value_ptr(lightColor)
      );

  // send to texture shaders

  glProgramUniform3fv(
      _textureShaderProgram->getShaderProgramHandle(),
      _textureShaderUniformLocations.lightColor,
      1,
      glm::value_ptr(lightColor)
      );
}

//*************************************************************************************
//
// Engine Cleanup

void A3Engine::mCleanupShaders() {
    fprintf( stdout, "[INFO]: ...deleting Shaders.\n" );
    delete _lightingShaderProgram;
    _lightingShaderProgram = nullptr;
    delete _textureShaderProgram;
    _textureShaderProgram = nullptr;
}

void A3Engine::mCleanupBuffers() {
    fprintf( stdout, "[INFO]: ...deleting VAOs....\n" );
    CSCI441::deleteObjectVAOs();
    glDeleteVertexArrays( 1, &_groundVAO );
    _groundVAO = 0;

    fprintf( stdout, "[INFO]: ...deleting VBOs....\n" );
    CSCI441::deleteObjectVBOs();

    fprintf( stdout, "[INFO]: ...deleting models..\n" );
    delete _pCaedilas;
    _pCaedilas = nullptr;
}

void A3Engine::mCleanupScene() {
    fprintf( stdout, "[INFO]: ...deleting camera..\n" );
    delete _pMainCam;
    _pMainCam = nullptr;
    delete _pSecondaryCam;
    _pSecondaryCam = nullptr;
}

//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

void A3Engine::_renderScene(const glm::mat4& viewMtx, const glm::mat4& projMtx) const {
    // use our lighting shader program
    _lightingShaderProgram->useProgram();
    CSCI441::setVertexAttributeLocations( _lightingShaderAttributeLocations.vPos, _lightingShaderAttributeLocations.vNormal );

    //// BEGIN DRAWING THE GROUND Caedilas ////
    // draw the ground Caedilas
    const glm::mat4 groundModelMtx = glm::scale( glm::mat4(1.0f), glm::vec3(WORLD_SIZE, 1.0f, WORLD_SIZE));
    _computeAndSendMatrixUniforms(groundModelMtx, viewMtx, projMtx);

    constexpr glm::vec3 groundColor(0.3f, 0.8f, 0.2f);
    _lightingShaderProgram->setProgramUniform(_lightingShaderUniformLocations.materialColor, groundColor);

    glBindVertexArray(_groundVAO);
    glDrawElements(GL_TRIANGLE_STRIP, _numGroundPoints, GL_UNSIGNED_SHORT, (void*)0);
    //// END DRAWING THE GROUND Caedilas ////

    //// BEGIN DRAWING THE BUILDINGS ////
    for( const BuildingData& currentBuilding : _buildings ) {
        _computeAndSendMatrixUniforms(currentBuilding.modelMatrix, viewMtx, projMtx);

        _lightingShaderProgram->setProgramUniform(_lightingShaderUniformLocations.materialColor, currentBuilding.color);

        CSCI441::drawSolidCube(1.0);
    }
    //// END DRAWING THE BUILDINGS ////

    //// BEGIN DRAWING THE MODEL ////
    _textureShaderProgram->useProgram();
    CSCI441::setVertexAttributeLocations(_textureShaderAttributeLocations.vPos,
//                                         _textureShaderAttributeLocations.vNormal,
                                         _textureShaderAttributeLocations.vTexCoord);

    _pCaedilas->drawCaedilas( viewMtx, projMtx );
    
    //// END DRAWING THE MODEL ////
}

void A3Engine::_updateScene() {
    // animate
    _currTime = (GLfloat)glfwGetTime();
    if (_pCaedilas != nullptr) {
      _pCaedilas->animate(_currTime - _lastTime);
    }
    _lastTime = _currTime;

    //_pSecondaryCam->setPhi(glm::pi<float>() / 1.0f);
    
    // turn right
    if( _keys[GLFW_KEY_D] || _keys[GLFW_KEY_RIGHT] ) {
        _pCaedilas->rotate(_cameraSpeed.y, 0.0f);
        _pSecondaryCam->setTheta(_pCaedilas->getTheta() + glm::radians(-90.0f));
        _pSecondaryCam->recomputeOrientation();
    }
    // turn left
    if( _keys[GLFW_KEY_A] || _keys[GLFW_KEY_LEFT] ) {
        _pCaedilas->rotate(-_cameraSpeed.y, 0.0f);
        _pSecondaryCam->setTheta(_pCaedilas->getTheta() + glm::radians(-90.0f));
        _pSecondaryCam->recomputeOrientation();
    }
    
    
    // move Caedilas, then move camera to match
    // move forward
    if( _keys[GLFW_KEY_W] || _keys[GLFW_KEY_UP] ) {
      _pCaedilas->moveForward(_playerSpeed.x);
      _pMainCam->setLookAtPoint(_pCaedilas->getLocation() + 1.0f);
      _pMainCam->recomputeOrientation();

      _pSecondaryCam->setLookAtPoint(_pCaedilas->getLocation() + 1.0f);
      _pSecondaryCam->recomputeOrientation();
    }
    // move forward
    if( _keys[GLFW_KEY_S] || _keys[GLFW_KEY_DOWN] ) {
      _pCaedilas->moveBackward(_playerSpeed.x);
      _pMainCam->setLookAtPoint(_pCaedilas->getLocation() + 1.0f);
      _pMainCam->recomputeOrientation();

      _pSecondaryCam->setLookAtPoint(_pCaedilas->getLocation() + 1.0f);
      _pSecondaryCam->recomputeOrientation();
    }

  // rotate light direction by direction character is facing
  glm::mat4 r = glm::rotate(glm::mat4(1.0f), -_pCaedilas->getTheta(), CSCI441::Y_AXIS);
  glm::vec3 lightDirectionRotated = r * glm::vec4(-1,-1,-1,0);
  glProgramUniform3fv(
      _textureShaderProgram->getShaderProgramHandle(),
      _textureShaderUniformLocations.lightDirection,
      1,
      glm::value_ptr(lightDirectionRotated)
      );
    
}

void A3Engine::run() {
    //  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
    //	until the user decides to close the window and quit the program.  Without a loop, the
    //	window will display once and then the program exits.
    while( !glfwWindowShouldClose(mpWindow) ) {	        // check if the window was instructed to be closed
        glDrawBuffer( GL_BACK );				        // work with our back frame buffer
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// clear the current color contents and depth buffer in the window

        // Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
        // when using a Retina display the actual window can be larger than the requested window.  Therefore,
        // query what the actual size of the window we are rendering to is.
        GLint framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize( mpWindow, &framebufferWidth, &framebufferHeight );

        // update the viewport - tell OpenGL we want to render to the whole window
        glViewport( 0, 0, framebufferWidth, framebufferHeight );

        // draw everything to the window
        _renderScene(_pMainCam->getViewMatrix(), _pMainCam->getProjectionMatrix());

        // secondary viewport
        // clear out rectangle
        glEnable(GL_SCISSOR_TEST);
        glScissor( framebufferWidth/3 * 2, framebufferHeight / 3 * 2, framebufferWidth/3, framebufferHeight/3 );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// clear the current color contents and depth buffer in the rectangle 
        glDisable(GL_SCISSOR_TEST);
        glViewport( framebufferWidth/3 * 2, framebufferHeight / 3 * 2, framebufferWidth/3, framebufferHeight/3 );
        _renderScene(_pSecondaryCam->getViewMatrix(), _pSecondaryCam->getProjectionMatrix());
        _updateScene();

        glfwSwapBuffers(mpWindow);                       // flush the OpenGL commands and make sure they get rendered!
        glfwPollEvents();				                // check for any events and signal to redraw screen
                                                //

    }
}

//*************************************************************************************
//
// Private Helper FUnctions

void A3Engine::_computeAndSendMatrixUniforms(const glm::mat4& modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const {
    // precompute the Model-View-Projection matrix on the CPU
    const glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
    // then send it to the shader on the GPU to apply to every vertex
    _lightingShaderProgram->setProgramUniform(_lightingShaderUniformLocations.mvpMatrix, mvpMtx);

    // compute and send the normal matrix
    const glm::mat3 normalMtx = glm::mat3(glm::transpose(glm::inverse(modelMtx)));
    _lightingShaderProgram->setProgramUniform(_lightingShaderUniformLocations.normalMatrix, normalMtx);

    // send to texture shader
    _textureShaderProgram->setProgramUniform(_textureShaderUniformLocations.mvpMatrix, mvpMtx);
    //_textureShaderProgram->setProgramUniform(_textureShaderUniformLocations.normalMatrix, normalMtx);

}

//*************************************************************************************
//
// Callbacks

void a3_engine_keyboard_callback(GLFWwindow *window, const int key, const int scancode, const int action, const int mods ) {
   const auto engine = static_cast<A3Engine *>(glfwGetWindowUserPointer(window));

    // pass the key and action through to the engine
    engine->handleKeyEvent(key, action);
}

void a3_engine_cursor_callback(GLFWwindow *window, const double x, const double y ) {
    const auto engine = static_cast<A3Engine *>(glfwGetWindowUserPointer(window));

    // pass the cursor position through to the engine
    engine->handleCursorPositionEvent(glm::vec2(x, y));
}

void a3_engine_mouse_button_callback(GLFWwindow *window, const int button, const int action, const int mods ) {
    const auto engine = static_cast<A3Engine *>(glfwGetWindowUserPointer(window));

    // pass the mouse button and action through to the engine
    engine->handleMouseButtonEvent(button, action);
}
