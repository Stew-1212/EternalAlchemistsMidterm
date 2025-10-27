#include "MPEngine.h"
#include <CSCI441/objects.hpp> //might want this later to generate more objects in the scene

//GET YOUR ARCBALL CAMERA MADE FIRST!!!

MPEngine::MPEngine() : CSCI441::OpenGLEngine(4, 1, 1800, 1200, "MP: Begin The Transformation"),
    _mousePosition({MOUSE_UNINITIALIZED, MOUSE_UNINITIALIZED}),
    _leftMouseButtonState(GLFW_RELEASE),
    _pArcballCam(nullptr),
    _chaoHead(nullptr),
    _chaoHeadBall(nullptr),
    _chaoRArm(nullptr),
    _chaoLArm(nullptr),
    _chaoBody(nullptr),
    _chaoRFoot(nullptr),
    _chaoLFoot(nullptr),
    _chaoTail(nullptr),
    _chaoWings(nullptr),
    _chaoMatCol(glm::vec3{1.f, 1.f, 1.f}), //make base color pure white for pure texture color when intially rendered
    _groundVAO(0),
    _numGroundPoints(0),
    _MPShaderProgram(nullptr),
    _MPShaderUniformLocations({-1}),
    _MPShaderAttributeLocations({-1, -1}),
    _ballPos(glm::vec3{0.008086f, 14.54f, -0.3646f}),
    _theta(0.f),
    _thetaSpeed(0.04f),
    _spiralOut(true),
    _thetaMax(12.f * glm::pi<float>()),
    _ballCenter(glm::vec3{0.008086f, 14.54f, -0.3646f}),
    _chaoPosOffset(glm::vec3{0.f}), //no offest to begin with
    _chaoHeading(glm::vec3{0, 0, 1}), //begins with looking up the z-axis
    _isMoving(false),
    _chaoPos(glm::vec3{0.008084f, 3.196f, 0.1679f}), //this is the center of the body piece, but will be the look at point for our arcball camera
    _origAngle(true),
    _armAngle(0.f),
    _armAngle2(0.f),
    _footAngle(0.f),
    _headAngle(0.f),
    _starPositions(),
    _starColors(),
    _starAngle(0.f)
{}

MPEngine::~MPEngine() {
    //clean up in reverse order of creation
    mCleanupScene();
    mCleanupBuffers();
    mCleanupShaders();
}

/*
 * ENGINE SETUP 
 */
void MPEngine::mSetupGLFW() {
    CSCI441::OpenGLEngine::mSetupGLFW();

    //connect the engine instance to the window
    glfwSetWindowUserPointer(mpWindow, this);
    
    //set our callbacks
    glfwSetKeyCallback(mpWindow, MP_keyboard_callback);
    glfwSetMouseButtonCallback(mpWindow, MP_mouse_button_callback);
    glfwSetCursorPosCallback(mpWindow, MP_cursor_callback);
    glfwSetScrollCallback(mpWindow, MP_scroll_callback);
}

void MPEngine::mSetupOpenGL() {
    glEnable(GL_DEPTH_TEST);    //enable depth testing
    glDepthFunc(GL_LESS); //use less than depth test
    glEnable(GL_BLEND); //enable belnding
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //use on minus blending equation
    glClearColor(0.f, 0.f, 0.f, 1.0f); //clear the frame buffer to black
}

void MPEngine::mSetupShaders() {
    //create and compile shader program
    _MPShaderProgram = new CSCI441::ShaderProgram(
        "shaders/MPShader.v.glsl", //vertex shader path
        "shaders/MPShader.f.glsl"
    );

    //get locations of uniforms and attributes
    //start with uniforms
    _MPShaderUniformLocations.mvpMtx = _MPShaderProgram->getUniformLocation("mvpMtx");
    _MPShaderUniformLocations.materialColor = _MPShaderProgram->getUniformLocation("matColor");
    _MPShaderUniformLocations.lightDir = _MPShaderProgram->getUniformLocation("lightDir");
    _MPShaderUniformLocations.lightColor = _MPShaderProgram->getUniformLocation("lightColor");
    _MPShaderUniformLocations.normMtx = _MPShaderProgram->getUniformLocation("normMtx");
    _MPShaderUniformLocations.texMap = _MPShaderProgram->getUniformLocation("texMap");
    _MPShaderUniformLocations.useTexture = _MPShaderProgram->getUniformLocation("useTexture");
    _MPShaderUniformLocations.useVertexColor = _MPShaderProgram->getUniformLocation("useVertexColor");
    _MPShaderUniformLocations.emissiveColor = _MPShaderProgram->getUniformLocation("emissiveColor");
    _MPShaderUniformLocations.useEmissive = _MPShaderProgram->getUniformLocation("useEmissive");

    //now attributes
    _MPShaderAttributeLocations.vPos = _MPShaderProgram->getAttributeLocation("vPosition");
    _MPShaderAttributeLocations.vNormal = _MPShaderProgram->getAttributeLocation("vNormal");
    _MPShaderAttributeLocations.texCoord = _MPShaderProgram->getAttributeLocation("texCoord");
    _MPShaderAttributeLocations.vColor = _MPShaderProgram->getAttributeLocation("vColor");

    //setup CSCI441 objects
    CSCI441::setVertexAttributeLocations(
        _MPShaderAttributeLocations.vPos,
        _MPShaderAttributeLocations.vNormal,
        _MPShaderAttributeLocations.texCoord 
    );
}

void MPEngine::mSetupBuffers() {
    //first create the gridlined quad
    _createGroundBuffers();
    //now throw in my chao by loading in all the pieces
    _buildChao();
}

void MPEngine::mSetupScene() {
    //seed the rng for _changeChaoCol function
    srand(time(nullptr) * 12122004);
    //create an arcball camera looking at loaded in chao with radius 50
    _pArcballCam = new ArcballCam(glm::vec3(_chaoPos), 50.f);
    _pArcballCam->setTheta(glm::radians(90.0f));
    _pArcballCam->setPhi(glm::radians(70.0f));
    _pArcballCam->recomputeOrientation();    
    //setup the shaders
    mSetupShaders();
    //setup the buffers
    mSetupBuffers();
    //setup the lights
    //first create the variables to store teh light's direction and color
    glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1)); //normalize this before sending for double checks
    glm::vec3 lightColor = glm::vec3(1, 1, 1);
    //now we send over the uniform information using glProgramUniform3fv()
    //first the lightDir uniform
    glProgramUniform3fv(_MPShaderProgram->getShaderProgramHandle(), _MPShaderUniformLocations.lightDir, 1, glm::value_ptr(lightDir));
    //now the lightColor uniform
    glProgramUniform3fv(_MPShaderProgram->getShaderProgramHandle(), _MPShaderUniformLocations.lightColor, 1, glm::value_ptr(lightColor));
    //generate 50 random stars within the world bounds
    for (int i=0; i < 50; i++) {
        float x = ((rand() / (float)RAND_MAX) - 0.5f) * WORLD_SIZE;
        float y = 5.0f + ((rand() / (float)RAND_MAX) * 25.f); //random height between 5-25
        float z = ((rand() / (float)RAND_MAX) - 0.5f) * WORLD_SIZE;
        _starPositions.emplace_back(x, y , z);
        _starColors.emplace_back(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
    }
}

/*
* ENGINE CLEANUP
*/

void MPEngine::mCleanupShaders() {
    //unbind any shader program as good practice
    glUseProgram(0);
    //now delete shader program
    delete _MPShaderProgram;
    _MPShaderProgram = nullptr;
}

void MPEngine::mCleanupBuffers() {
    //clean up ground VAO and VBO
    CSCI441::deleteObjectVAOs();
    CSCI441::deleteObjectVBOs();
    //delete models
    delete _chaoHead;
    _chaoHead = nullptr;
    delete _chaoHeadBall;
    _chaoHeadBall = nullptr;
    delete _chaoRArm;
    _chaoRArm = nullptr;
    delete _chaoLArm;
    _chaoLArm = nullptr;
    delete _chaoBody;
    _chaoBody = nullptr;
    delete _chaoRFoot;
    _chaoRFoot = nullptr;
    delete _chaoLFoot;
    _chaoLFoot = nullptr;
    delete _chaoTail;
    _chaoTail = nullptr;
    delete _chaoWings;
    _chaoWings = nullptr;
}

void MPEngine::mCleanupScene() {
    //delete camera
    delete _pArcballCam;
    _pArcballCam = nullptr;
    //clear the vectors
    _starPositions.clear();
    _starPositions.shrink_to_fit();
    _starColors.clear();
    _starColors.shrink_to_fit();
}

/**
 * Rendering / Drawing functions
 */

void MPEngine::_renderScene(const glm::mat4& viewMtx, const glm::mat4& projMtx) const{
    //draw the ground grid using the _drawGroundGrid() function
    _drawGroundGrid(viewMtx, projMtx);
    //draw the chao using the _drawChao() function
    _drawChao(viewMtx, projMtx);
    //now create the surrounding environment utilizing the class object.hpp file via  _drawEnvironment()
    _drawEnvironment(viewMtx, projMtx);
}

void MPEngine::_updateScene() {
    //animate the Chao's headball passively
    _animateBall();
    //update the camera position as the chao moves
    _pArcballCam->setTarget(_chaoPos);
    //check if _isMoving is true and if so animate the chao's body otherwise reset the body back to normal position when not moving
    if (_isMoving) {
        _animateBody();
    }
    //update the angle of the stars
    _starAngle += 0.06;
}

void MPEngine::run(){
    while (!glfwWindowShouldClose(mpWindow)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 viewMtx = _pArcballCam->getViewMatrix();
        glm::mat4 projMtx = glm::perspective(glm::radians(45.0f), (float)mWindowWidth / mWindowHeight, 0.1f, 300.0f);

        _renderScene(viewMtx, projMtx);

        glfwSwapBuffers(mpWindow);
        glfwPollEvents();
        //updates for animation!
        _updateScene();
    }
}

/**
 * PRIVATE HELPER FUNCTIONS
 */

 void MPEngine::_buildChao() {
    //create all the modelLoaders we will need and place them in a container
    _chaoHead = new CSCI441::ModelLoader();
    _chaoHeadBall = new CSCI441::ModelLoader();
    _chaoRArm = new CSCI441::ModelLoader();
    _chaoLArm = new CSCI441::ModelLoader();
    _chaoBody = new CSCI441::ModelLoader();
    _chaoRFoot = new CSCI441::ModelLoader();
    _chaoLFoot = new CSCI441::ModelLoader();
    _chaoTail = new CSCI441::ModelLoader();
    _chaoWings = new CSCI441::ModelLoader();

    // load chao piece by piece by loading in each respective file
    //first the head
    if( _chaoHead->loadModelFile("models/ChaoParts/chaoHead.obj" ) ) {
        //set attributes
        _chaoHead->setAttributeLocations(_MPShaderAttributeLocations.vPos,
                                        _MPShaderAttributeLocations.vNormal,
                                        _MPShaderAttributeLocations.texCoord);
    } else {
        fprintf(stderr, "[ERROR]: Could not open OBJ Model for Head\n");
        delete _chaoHead;
        _chaoHead = nullptr;
    }
    //now the headBall
    if( _chaoHeadBall->loadModelFile("models/ChaoParts/chaoHeadBall.obj" ) ) {
        //set attributes
        _chaoHeadBall->setAttributeLocations(_MPShaderAttributeLocations.vPos,
                                        _MPShaderAttributeLocations.vNormal,
                                        _MPShaderAttributeLocations.texCoord);
    } else {
        fprintf(stderr, "[ERROR]: Could not open OBJ Model for HeadBall\n");
        delete _chaoHeadBall;
        _chaoHeadBall = nullptr;
    }
    //now RArm
    if( _chaoRArm->loadModelFile("models/ChaoParts/chaoRArm.obj" ) ) {
        //set attributes
        _chaoRArm->setAttributeLocations(_MPShaderAttributeLocations.vPos,
                                        _MPShaderAttributeLocations.vNormal,
                                        _MPShaderAttributeLocations.texCoord);
    } else {
        fprintf(stderr, "[ERROR]: Could not open OBJ Model for RArm\n");
        delete _chaoRArm;
        _chaoRArm = nullptr;
    }
    //now LArm
    if( _chaoLArm->loadModelFile("models/ChaoParts/chaoLArm.obj" ) ) {
        //set attributes
        _chaoLArm->setAttributeLocations(_MPShaderAttributeLocations.vPos,
                                        _MPShaderAttributeLocations.vNormal,
                                        _MPShaderAttributeLocations.texCoord);
    } else {
        fprintf(stderr, "[ERROR]: Could not open OBJ Model for LArm\n");
        delete _chaoLArm;
        _chaoLArm = nullptr;
    }
    //now Body
    if( _chaoBody->loadModelFile("models/ChaoParts/chaoBody.obj" ) ) {
        //set attributes
        _chaoBody->setAttributeLocations(_MPShaderAttributeLocations.vPos,
                                        _MPShaderAttributeLocations.vNormal,
                                        _MPShaderAttributeLocations.texCoord);
    } else {
        fprintf(stderr, "[ERROR]: Could not open OBJ Model for Body\n");
        delete _chaoBody;
        _chaoBody = nullptr;
    }
    //now RFoot
    if( _chaoRFoot->loadModelFile("models/ChaoParts/chaoRFoot.obj" ) ) {
        //set attributes
        _chaoRFoot->setAttributeLocations(_MPShaderAttributeLocations.vPos,
                                        _MPShaderAttributeLocations.vNormal,
                                        _MPShaderAttributeLocations.texCoord);
    } else {
        fprintf(stderr, "[ERROR]: Could not open OBJ Model for RFoot\n");
        delete _chaoRFoot;
        _chaoRFoot = nullptr;
    }
    //now LFoot
    if( _chaoLFoot->loadModelFile("models/ChaoParts/chaoLFoot.obj" ) ) {
        //set attributes
        _chaoLFoot->setAttributeLocations(_MPShaderAttributeLocations.vPos,
                                        _MPShaderAttributeLocations.vNormal,
                                        _MPShaderAttributeLocations.texCoord);
    } else {
        fprintf(stderr, "[ERROR]: Could not open OBJ Model for LFoot\n");
        delete _chaoLFoot;
        _chaoLFoot = nullptr;
    }
    //now Tail
    if( _chaoTail->loadModelFile("models/ChaoParts/chaoTail.obj" ) ) {
        //set attributes
        _chaoTail->setAttributeLocations(_MPShaderAttributeLocations.vPos,
                                        _MPShaderAttributeLocations.vNormal,
                                        _MPShaderAttributeLocations.texCoord);
    } else {
        fprintf(stderr, "[ERROR]: Could not open OBJ Model for Tail\n");
        delete _chaoTail;
        _chaoTail = nullptr;
    }
    //now Wings
    if( _chaoWings->loadModelFile("models/ChaoParts/chaoWings.obj" ) ) {
        //set attributes
        _chaoWings->setAttributeLocations(_MPShaderAttributeLocations.vPos,
                                        _MPShaderAttributeLocations.vNormal,
                                        _MPShaderAttributeLocations.texCoord);
    } else {
        fprintf(stderr, "[ERROR]: Could not open OBJ Model for Wings\n");
        delete _chaoWings;
        _chaoWings = nullptr;
    }
 }

 void MPEngine::_drawChao(const glm::mat4& viewMtx, const glm::mat4& projMtx) const {
    //let shader know this requires a texture but not vertex color or emissiveColor
    glUniform1i(_MPShaderProgram->getUniformLocation("useTexture"), GL_TRUE);
    glUniform1i(_MPShaderProgram->getUniformLocation("useVertexColor"), GL_FALSE);
    glUniform1i(_MPShaderProgram->getUniformLocation("useEmissive"), GL_FALSE);

    //_chaoMatColor acts as base color (multiplies with texture)
    glUniform3fv(_MPShaderUniformLocations.materialColor, 1, glm::value_ptr(_chaoMatCol));
    //begin drawing the chao from all the loaded in parts
    //draw chao head
    if (_chaoHead) {
        glm::mat4 modelMtx = glm::mat4(1.0f);
        //translate upwards a bit (will replace this with where we really want to put it for transfromations later)
        modelMtx = glm::translate(modelMtx, _chaoPosOffset);
        //compute y-axis rotation from heading
        float headingAngle = atan2(_chaoHeading.x, _chaoHeading.z); //y rotation in radians
        modelMtx = glm::rotate(modelMtx, headingAngle, glm::vec3(0, 1, 0));
        //apply local offsets
        modelMtx = glm::translate(modelMtx, glm::vec3(0.008086f, 4.772f, -0.6555f));
        //rotate the head about the y-axis via _headAngle
        modelMtx = glm::rotate(modelMtx, glm::radians(_headAngle), glm::vec3(0, 1, 0));
        //recompute mvp and the normal matrix
        glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
        glm::mat3 normMtx = glm::mat3(glm::transpose(glm::inverse(modelMtx)));
        //activate shader program!
        _MPShaderProgram->useProgram();
        //send over the mvp and normMtx to gpu
        glUniformMatrix4fv(_MPShaderUniformLocations.mvpMtx, 1, GL_FALSE, &mvpMtx[0][0]);
        glUniformMatrix3fv(_MPShaderUniformLocations.normMtx, 1, GL_FALSE, &normMtx[0][0]);
        _chaoHead->draw(_MPShaderProgram->getShaderProgramHandle());
    }
    //draw chao headball
    if (_chaoHeadBall) {
        glm::mat4 modelMtx = glm::mat4(1.0f);
        //translate upwards a bit (will replace this with where we really want to put it for transfromations later)
        modelMtx = glm::translate(modelMtx, _chaoPosOffset);
        //compute y-axis rotation from heading
        float headingAngle = atan2(_chaoHeading.x, _chaoHeading.z); //y rotation in radians
        modelMtx = glm::rotate(modelMtx, headingAngle, glm::vec3(0, 1, 0));
        //apply local offsets
        modelMtx = glm::translate(modelMtx, glm::vec3(_ballPos));
        //recompute mvp and the normal matrix
        glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
        glm::mat3 normMtx = glm::mat3(glm::transpose(glm::inverse(modelMtx)));
        //activate shader program!
        _MPShaderProgram->useProgram();
        //send over the mvp and normMtx to gpu
        glUniformMatrix4fv(_MPShaderUniformLocations.mvpMtx, 1, GL_FALSE, &mvpMtx[0][0]);
        glUniformMatrix4fv(_MPShaderUniformLocations.normMtx, 1, GL_FALSE, &normMtx[0][0]);
        _chaoHeadBall->draw(_MPShaderProgram->getShaderProgramHandle());
    }
    //draw chao RArm
    if (_chaoRArm) {
        glm::mat4 modelMtx = glm::mat4(1.0f);
        //translate upwards a bit (will replace this with where we really want to put it for transfromations later)
        modelMtx = glm::translate(modelMtx, _chaoPosOffset);
        //compute y-axis rotation from heading
        float headingAngle = atan2(_chaoHeading.x, _chaoHeading.z); //y rotation in radians
        modelMtx = glm::rotate(modelMtx, headingAngle, glm::vec3(0, 1, 0));
        //apply local offsets
        modelMtx = glm::translate(modelMtx, glm::vec3(1.312f, 4.657f, 0.07665f));
        //rotate the arms about the x-axis via _armAngle
        modelMtx = glm::rotate(modelMtx, glm::radians(_armAngle), glm::vec3(1, 0, 0));
        //rotate the arms about the z-axis via _armAngle2
        modelMtx = glm::rotate(modelMtx, glm::radians(_armAngle2), glm::vec3(0, 0, 1));
        //recompute mvp and the normal matrix
        glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
        glm::mat3 normMtx = glm::mat3(glm::transpose(glm::inverse(modelMtx)));
        //activate shader program!
        _MPShaderProgram->useProgram();
        //send over the mvp and normMtx to gpu
        glUniformMatrix4fv(_MPShaderUniformLocations.mvpMtx, 1, GL_FALSE, &mvpMtx[0][0]);
        glUniformMatrix4fv(_MPShaderUniformLocations.normMtx, 1, GL_FALSE, &normMtx[0][0]);
        _chaoRArm->draw(_MPShaderProgram->getShaderProgramHandle());
    }
    //draw chao LArm
    if (_chaoLArm) {
        glm::mat4 modelMtx = glm::mat4(1.0f);
        //translate upwards a bit (will replace this with where we really want to put it for transfromations later)
        modelMtx = glm::translate(modelMtx, _chaoPosOffset);
        //compute y-axis rotation from heading
        float headingAngle = atan2(_chaoHeading.x, _chaoHeading.z); //y rotation in radians
        modelMtx = glm::rotate(modelMtx, headingAngle, glm::vec3(0, 1, 0));
        //apply local offsets
        modelMtx = glm::translate(modelMtx, glm::vec3(-1.296f, 4.657f, 0.07665f));
        //rotate the arms about the x-axis via _armAngle (make this one opposite of the Rarm)
        modelMtx = glm::rotate(modelMtx, glm::radians(-_armAngle), glm::vec3(1, 0, 0));
        //rotate the arms about the z-axis via _armAngle2 (make this one opposite of the Rarm)
        modelMtx = glm::rotate(modelMtx, glm::radians(-_armAngle2), glm::vec3(0, 0, 1));
        //recompute mvp and the normal matrix
        glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
        glm::mat3 normMtx = glm::mat3(glm::transpose(glm::inverse(modelMtx)));
        //activate shader program!
        _MPShaderProgram->useProgram();
        //send over the mvp and normMtx to gpu
        glUniformMatrix4fv(_MPShaderUniformLocations.mvpMtx, 1, GL_FALSE, &mvpMtx[0][0]);
        glUniformMatrix4fv(_MPShaderUniformLocations.normMtx, 1, GL_FALSE, &normMtx[0][0]);
        _chaoLArm->draw(_MPShaderProgram->getShaderProgramHandle());
    }
    //draw chao body
    if (_chaoBody) {
        glm::mat4 modelMtx = glm::mat4(1.0f);
        //translate upwards a bit (will replace this with where we really want to put it for transfromations later)
        modelMtx = glm::translate(modelMtx, _chaoPosOffset);
        //compute y-axis rotation from heading
        float headingAngle = atan2(_chaoHeading.x, _chaoHeading.z); //y rotation in radians
        modelMtx = glm::rotate(modelMtx, headingAngle, glm::vec3(0, 1, 0));
        //apply local offsets
        modelMtx = glm::translate(modelMtx, glm::vec3(0.008084f, 3.196f, 0.1679f));
        //recompute mvp and the normal matrix
        glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
        glm::mat3 normMtx = glm::mat3(glm::transpose(glm::inverse(modelMtx)));
        //activate shader program!
        _MPShaderProgram->useProgram();
        //send over the mvp and normMtx to gpu
        glUniformMatrix4fv(_MPShaderUniformLocations.mvpMtx, 1, GL_FALSE, &mvpMtx[0][0]);
        glUniformMatrix4fv(_MPShaderUniformLocations.normMtx, 1, GL_FALSE, &normMtx[0][0]);
        _chaoBody->draw(_MPShaderProgram->getShaderProgramHandle());
    }
    //draw chao RFoot
    if (_chaoRFoot) {
        glm::mat4 modelMtx = glm::mat4(1.0f);
        //translate upwards a bit (will replace this with where we really want to put it for transfromations later)
        modelMtx = glm::translate(modelMtx, _chaoPosOffset);
        //compute y-axis rotation from heading
        float headingAngle = atan2(_chaoHeading.x, _chaoHeading.z); //y rotation in radians
        modelMtx = glm::rotate(modelMtx, headingAngle, glm::vec3(0, 1, 0));
        //apply local offsets
        modelMtx = glm::translate(modelMtx, glm::vec3(1.427f, 1.811f, 0.001732f));
        //rotate the feet about the x-axis via _footAngle
        modelMtx = glm::rotate(modelMtx, glm::radians(_footAngle), glm::vec3(1, 0, 0));
        //recompute mvp and the normal matrix
        glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
        glm::mat3 normMtx = glm::mat3(glm::transpose(glm::inverse(modelMtx)));
        //activate shader program!
        _MPShaderProgram->useProgram();
        //send over the mvp and normMtx to gpu
        glUniformMatrix4fv(_MPShaderUniformLocations.mvpMtx, 1, GL_FALSE, &mvpMtx[0][0]);
        glUniformMatrix4fv(_MPShaderUniformLocations.normMtx, 1, GL_FALSE, &normMtx[0][0]);
        _chaoRFoot->draw(_MPShaderProgram->getShaderProgramHandle());
    }
    //draw chao LFoot
    if (_chaoLFoot) {
        glm::mat4 modelMtx = glm::mat4(1.0f);
        //translate upwards a bit (will replace this with where we really want to put it for transfromations later)
        modelMtx = glm::translate(modelMtx, _chaoPosOffset);
        //compute y-axis rotation from heading
        float headingAngle = atan2(_chaoHeading.x, _chaoHeading.z); //y rotation in radians
        modelMtx = glm::rotate(modelMtx, headingAngle, glm::vec3(0, 1, 0));
        //apply local offsets
        modelMtx = glm::translate(modelMtx, glm::vec3(-1.411f, 1.811f, 0.001732f));
        //rotate the feet about the x-axis via _footAngle (make this one opposite of the RFoot)
        modelMtx = glm::rotate(modelMtx, glm::radians(-_footAngle), glm::vec3(1, 0, 0));
        //recompute mvp and the normal matrix
        glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
        glm::mat3 normMtx = glm::mat3(glm::transpose(glm::inverse(modelMtx)));
        //activate shader program!
        _MPShaderProgram->useProgram();
        //send over the mvp and normMtx to gpu
        glUniformMatrix4fv(_MPShaderUniformLocations.mvpMtx, 1, GL_FALSE, &mvpMtx[0][0]);
        glUniformMatrix4fv(_MPShaderUniformLocations.normMtx, 1, GL_FALSE, &normMtx[0][0]);
        _chaoLFoot->draw(_MPShaderProgram->getShaderProgramHandle());
    }
    //draw chao Tail
    if (_chaoTail) {
        glm::mat4 modelMtx = glm::mat4(1.0f);
        //translate upwards a bit (will replace this with where we really want to put it for transfromations later)
        modelMtx = glm::translate(modelMtx, _chaoPosOffset);
        //compute y-axis rotation from heading
        float headingAngle = atan2(_chaoHeading.x, _chaoHeading.z); //y rotation in radians
        modelMtx = glm::rotate(modelMtx, headingAngle, glm::vec3(0, 1, 0));
        //apply local offsets
        modelMtx = glm::translate(modelMtx, glm::vec3(0.008086f, 2.991f, -2.484f));
        //recompute mvp and the normal matrix
        glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
        glm::mat3 normMtx = glm::mat3(glm::transpose(glm::inverse(modelMtx)));
        //activate shader program!
        _MPShaderProgram->useProgram();
        //send over the mvp and normMtx to gpu
        glUniformMatrix4fv(_MPShaderUniformLocations.mvpMtx, 1, GL_FALSE, &mvpMtx[0][0]);
        glUniformMatrix4fv(_MPShaderUniformLocations.normMtx, 1, GL_FALSE, &normMtx[0][0]);
        _chaoTail->draw(_MPShaderProgram->getShaderProgramHandle());
    }
    //draw chao Wings
    if (_chaoWings) {
        glm::mat4 modelMtx = glm::mat4(1.0f);
        //translate upwards a bit (will replace this with where we really want to put it for transfromations later)
        modelMtx = glm::translate(modelMtx, _chaoPosOffset);
        //compute y-axis rotation from heading
        float headingAngle = atan2(_chaoHeading.x, _chaoHeading.z); //y rotation in radians
        modelMtx = glm::rotate(modelMtx, headingAngle, glm::vec3(0, 1, 0));
        //apply local offsets
        modelMtx = glm::translate(modelMtx, glm::vec3(0.008086f, 4.426f, -1.563f));
        //recompute mvp and the normal matrix
        glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
        glm::mat3 normMtx = glm::mat3(glm::transpose(glm::inverse(modelMtx)));
        //activate shader program!
        _MPShaderProgram->useProgram();
        //send over the mvp and normMtx to gpu
        glUniformMatrix4fv(_MPShaderUniformLocations.mvpMtx, 1, GL_FALSE, &mvpMtx[0][0]);
        glUniformMatrix4fv(_MPShaderUniformLocations.mvpMtx, 1, GL_FALSE, &mvpMtx[0][0]);
        _chaoWings->draw(_MPShaderProgram->getShaderProgramHandle());
    }
 }

 void MPEngine::_createGroundBuffers() {
    //create struct to hold vertex data
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec3 color;
    };

    //create vector of type Vertex (our struct)
    std::vector<Vertex> groundVertices;
    //specify grid lines and spacing between them
    const GLfloat halfSize = WORLD_SIZE / 2.0f;
    const GLint numLines = 40;
    const GLfloat spacing = WORLD_SIZE / numLines;
    ///upward normal for all grid lines
    glm::vec3 upNormal = glm::vec3(0.0f, 1.0f, 0.0f);

    //lines parallel to X-axis
    for (int i = 0; i <= numLines; i++) {
        GLfloat z = -halfSize + i * spacing;
        //assign colors in alternating fashion
        glm::vec3 color = (i % 2 == 0)
            ? glm::vec3(1.f, 1.f, 1.f)
            : glm::vec3(0.2f, 0.84f, 0.84f);
        //emplace data into vector
        groundVertices.push_back({{-halfSize, 0.0f, z}, upNormal, color});
        groundVertices.push_back({{halfSize, 0.0f, z}, upNormal, color});
    }
    
    //line parallel to Z-axis
    for (int i = 0; i <= numLines; i++) {
        GLfloat x = -halfSize + i * spacing;
        //assign colors again
        glm::vec3 color = (i % 2 == 0)
            ? glm::vec3(1.f, 1.f, 1.f)
            : glm::vec3(0.2f, 0.84f, 0.84f);
        //emplace data again
        groundVertices.push_back({{x, 0.0f, -halfSize}, upNormal, color});
        groundVertices.push_back({{x, 0.0f, halfSize}, upNormal, color});
    }
    //get the size of our ground vertices vector in GL friendly format
    _numGroundPoints = static_cast<GLsizei>(groundVertices.size());
    //setup vbo, attach it to _groundVAO, and populate with the data in our vector
    GLuint vbo;
    glGenVertexArrays(1, &_groundVAO);
    glGenBuffers(1, &vbo);
    glBindVertexArray(_groundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, groundVertices.size() * sizeof(Vertex), groundVertices.data(), GL_STATIC_DRAW);
    //send data over to GPU/Shaders  
    //position attribute
    GLint posAttrib = _MPShaderAttributeLocations.vPos;
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    //normal attribute
    GLint normAttrib = _MPShaderAttributeLocations.vNormal;
    glEnableVertexAttribArray(normAttrib);
    glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    //color attribute
    GLint colAttrib = _MPShaderAttributeLocations.vColor;
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    //unbind the _groundVAO
    glBindVertexArray(0);
 }

 void MPEngine::_drawGroundGrid(const glm::mat4& viewMtx, const glm::mat4& projMtx) const {
    //compute mvp matrix
    glm::mat4 modelMtx = glm::mat4(1.0f);
    glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
    glm::mat3 normMtx = glm::mat3(glm::transpose(glm::inverse(modelMtx)));
    //activate the shader program!
    _MPShaderProgram->useProgram();

    //send MVP and Norm matrix to GPU
    glUniformMatrix4fv(_MPShaderUniformLocations.mvpMtx, 1, GL_FALSE, &mvpMtx[0][0]);
    glUniformMatrix3fv(_MPShaderUniformLocations.normMtx, 1, GL_FALSE, &normMtx[0][0]);

    //grid color
    glm::vec3 gridTint = glm::vec3(1.f, 1.f, 1.f); //no tint (pure vColor)
    glUniform3fv(_MPShaderUniformLocations.materialColor, 1, glm::value_ptr(gridTint));

    //grid does not use textures or emissive color but does use vertex color
    glUniform1i(_MPShaderProgram->getUniformLocation("useTexture"), GL_FALSE);
    glUniform1i(_MPShaderProgram->getUniformLocation("useVertexColor"), GL_TRUE);
    glUniform1i(_MPShaderProgram->getUniformLocation("useEmissive"), GL_FALSE);

    //draw grid lines
    glBindVertexArray(_groundVAO);
    glDrawArrays(GL_LINES, 0, _numGroundPoints);
    glBindVertexArray(0);
 }

 void MPEngine::_drawEnvironment(const glm::mat4& viewMtx, const glm::mat4& projMtx) const {
    //activate the shader program
    _MPShaderProgram->useProgram();
    //these drawings will not use texture or vertex color but will use emissive color
    glUniform1i(_MPShaderProgram->getUniformLocation("useEmissive"), GL_TRUE);
    glUniform1i(_MPShaderUniformLocations.useTexture, GL_FALSE);
    glUniform1i(_MPShaderUniformLocations.useVertexColor, GL_FALSE);
    for (int i=0; i<_starPositions.size(); i++) {
        //base transform
        glm::mat4 base = glm::translate(glm::mat4(1.0f), _starPositions[i]);
        glm::vec3 starColor = _starColors[i];
        //send over uniform data
        glUniform3fv(_MPShaderUniformLocations.materialColor, 1, glm::value_ptr(starColor));
        glUniform3fv(_MPShaderUniformLocations.emissiveColor, 1, glm::value_ptr(starColor));
        //draw the 4 cubes that will make the start but make them slightly offset
        for (int j=0; j<4; j++) {
            glm::mat4 model = base;
            if (j == 1) model = glm::rotate(model, glm::radians(45.f + _starAngle), glm::vec3(1, 0, 0));
            if (j == 2) model = glm::rotate(model, glm::radians(45.f + _starAngle), glm::vec3(0, 1, 0));
            if (j == 3) model = glm::rotate(model, glm::radians(45.f + _starAngle), glm::vec3(0, 0, 1));
            model = glm::scale(model, glm::vec3(1.01f + 0.01f * j)); //tiny scale difference to avoid artifact
            //calculate mvp and norm matrices
            glm::mat4 mvp = projMtx * viewMtx * model;
            glm::mat3 normMtx = glm::mat3(glm::transpose(glm::inverse(model)));
            glUniformMatrix4fv(_MPShaderUniformLocations.mvpMtx, 1, GL_FALSE, &mvp[0][0]);
            glUniformMatrix4fv(_MPShaderUniformLocations.normMtx, 1, GL_FALSE, &normMtx[0][0]);
            //draw the cube
            CSCI441::drawSolidCube(5.0f);
        }
    }
}

void MPEngine::_changeChaoCol() {
    //generate a random vec3 color
    _chaoMatCol = glm::vec3(rand() / (float)RAND_MAX,
                            rand() / (float)RAND_MAX,
                            rand() / (float)RAND_MAX);
}

 void MPEngine::_updateChaoHeading(float angle) {
    //create rotation around the y-axis
    glm::mat4 rotMtx = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
    glm::vec4 newHeading = rotMtx * glm::vec4(_chaoHeading, 0.0f);
    _chaoHeading = glm::normalize(glm::vec3(newHeading));
 }

 void MPEngine::_updateChaoPos(float moveAmount) {
    //turn on _isMoving bool so our feet and arms animate as we walk
    _isMoving = true;
    //update the position offset
    _chaoPosOffset += moveAmount * _chaoHeading;
    //bounds check via the same way the grid was made: WORLD_SIZE/2
    _chaoPosOffset.x = glm::clamp(_chaoPosOffset.x, -WORLD_SIZE/2, WORLD_SIZE/2);
    _chaoPosOffset.z = glm::clamp(_chaoPosOffset.z, -WORLD_SIZE/2, WORLD_SIZE/2);

    //update the _chaoPos variable for camera
    _chaoPos += moveAmount * _chaoHeading;
    //bounds check the camera via _chaoPos
    _chaoPos.x = glm::clamp(_chaoPos.x, -WORLD_SIZE/2, WORLD_SIZE/2);
    _chaoPos.z = glm::clamp(_chaoPos.z, -WORLD_SIZE/2, WORLD_SIZE/2);
 }

 /**
  *ANIMATION FUNCTIONS
  */
 //resuse headBall animation from A2 but convert for 3-D space
 void MPEngine::_animateBall() {
    //update theat based on direction
    if (_spiralOut) {
        _theta += _thetaSpeed;
        if (_theta >= _thetaMax) {
            _theta = _thetaMax;
            _spiralOut = false; //start coming back to center/ origin of spiral
        }
    } else {
        _theta -= _thetaSpeed;
        if (_theta <= 0.f) {
            _theta = 0.f;
            _spiralOut = true; //start going back out of origin
        }
    }
    //compute spiral radius
    float r = 0.018 * _theta;
    //horizontal spiral in XZ plane
    _ballPos.x = _ballCenter.x + r * glm::cos(_theta);
    _ballPos.z = _ballCenter.z + r * glm::sin(_theta);
    //vertical up-and-down motion using sine wave
    float verticalAmplitude = .3f;
    float verticalFrequency = 4.f;
    _ballPos.y = _ballCenter.y + verticalAmplitude * glm::sin(verticalFrequency * glm::pi<float>() * (_theta / _thetaMax));
}

void MPEngine::_animateBody() {
    //will update the arms in a swaying pattern
    if (_origAngle) {
        //if the angle is where its original position is then we want to increase the angle
        //will increase the x-axis rotation angle by 4 degrees and z-axis rotation angle by 1
        _armAngle += 4.f;
        _armAngle2 += 1.4f;
        //now update the foot angle
        _footAngle += 3.f;
        //now update the head angle
        _headAngle += 2.f;
        //now we check if we have reached our desired angle to go back to original (only need to check one foot)
        if (_armAngle >= 48.f) {
            //if we have reached our desired angle then update the _origAngle bool so we can go back to the original angle
            _origAngle = false;
        }
    } else {
        //this means our _origAngle is false and we need to go back to the original angle
        //so decrement _armAngle by 4 and _armAngle2 by 1
        _armAngle -= 4.f;
        _armAngle2 -= 1.4f;
        //now update the foot angle
        _footAngle -= 3.f;
        //now update the head angle
        _headAngle -= 2.f;
        //now do another check to update the bool
        if (_armAngle <= -48.f) {
            _origAngle = true;
        }
    }

    //turning off the bool for _isMoving otherwise the feet will continue to "animate" when were not moving
    _isMoving = false;
}

void MPEngine::_resetBodyState() {
    //reset all the body angles back to zero
    _armAngle = 0.f;
    _armAngle2 = 0.f;
    _footAngle = 0.f;
    _headAngle = 0.f;
    //reset chao color back to normal
    _chaoMatCol = glm::vec3(1.f, 1.f, 1.f);
}

 /**
  * CALLBACK FUNCTIONS
  */
void MP_keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    //get handle to engine
    auto engine = static_cast<MPEngine*>(glfwGetWindowUserPointer(window));
    //now we check if certain keys are pressed
    switch (key) {
        //if 'A' or 'D' (or left arrow/right arrow) are pressed we update the direction of our Chao
        case GLFW_KEY_LEFT:
        case GLFW_KEY_A:
            engine->_updateChaoHeading(5.f); //have 5 degrees for now
            break;
        case GLFW_KEY_RIGHT:
        case GLFW_KEY_D:
            engine->_updateChaoHeading(-5.f);
            break;
        //if 'W' or 'S' (or up/down arrow) are pressed we update just the position of our chao
        case GLFW_KEY_UP:
        case GLFW_KEY_W:
            engine->_updateChaoPos(2.f); //make the speed of movement 2 for now
            break;
        case GLFW_KEY_DOWN:
        case GLFW_KEY_S:
            engine->_updateChaoPos(-2.f);
            break;
        case GLFW_KEY_R:
            //rest the body part angles
            engine->_resetBodyState();
            break;
        case GLFW_KEY_C:
            //change the color of our chao
            engine->_changeChaoCol();
            break;
        case GLFW_KEY_SPACE:
            //take a screenshot
            engine->saveScreenshot(nullptr);
            break;
        case GLFW_KEY_Q:
        case GLFW_KEY_ESCAPE:
            //close program
            engine->setWindowShouldClose();
            break;
    }
}

void MP_cursor_callback(GLFWwindow *window, double x, double y) {
    //get handle to engine
    auto engine = static_cast<MPEngine*>(glfwGetWindowUserPointer(window));

    //compute mouse delta
    double dx = x - engine->getMousePosition().x;
    double dy = y - engine->getMousePosition().y;
    
    //only act if left mouse button is pressed
    if (engine->getLeftMouseButtoneState() == GLFW_PRESS) {
        //check if Shift is held
        bool shiftHeld = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
        if (shiftHeld) {
            //shift + left drag = zoom
            engine->getArcballcam()->moveForward(-dy * 0.01f);
        } else {
            //normal left drag = orbit rotation
            //update camera rotation (scale motion to radians)
            engine->getArcballcam()->rotateTheta(dx * 0.005f);
            engine->getArcballcam()->rotatePhi(dy * 0.005f);
            engine->getArcballcam()->recomputeOrientation();
        }        
    }
    //update last mouse position
    engine->setMousePosition({x, y});
}

void MP_mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    //get handle
    auto engine = static_cast<MPEngine*>(glfwGetWindowUserPointer(window));
    //check if left mouse button was pressed and if so update the class variable to pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        engine->setLeftMouseButtonState(action);
    }
}

void MP_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    auto engine = static_cast<MPEngine*>(glfwGetWindowUserPointer(window));
    //update the radius of the arcball cam with scrolling in or out  
    if (yoffset > 0) engine->getArcballcam()->moveForward(0.5f);
    else if (yoffset < 0) engine->getArcballcam()->moveBackward(0.5f);
}
