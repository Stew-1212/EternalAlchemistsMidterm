#include "MPEngine.h"
#include <CSCI441/objects.hpp> //might want this later to generate more objects in the scene

//GET YOUR ARCBALL CAMERA MADE FIRST!!!

MPEngine::MPEngine() : CSCI441::OpenGLEngine(4, 1, 1000, 1000, "MP: The Eternal Alchemists!"),
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
    _groundVAO(0),
    _numGroundPoints(0),
    _MPShaderProgram(nullptr),
    _MPShaderUniformLocations({-1}),
    _MPShaderAttributeLocations({-1, -1})
{}

MPEngine::~MPEngine() {
    delete _pArcballCam;
    delete _MPShaderProgram;
    delete _chaoHead;
    delete _chaoHeadBall;
    delete _chaoRArm;
    delete _chaoLArm;
    delete _chaoBody;
    delete _chaoRFoot;
    delete _chaoLFoot;
    delete _chaoTail;
    delete _chaoWings;
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
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f); //clear the frame buffer to gray
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
    //now attributes
    _MPShaderAttributeLocations.vPos = _MPShaderProgram->getAttributeLocation("vPosition");
    _MPShaderAttributeLocations.vNormal = _MPShaderProgram->getAttributeLocation("vNormal");
    _MPShaderAttributeLocations.texCoord = _MPShaderProgram->getAttributeLocation("texCoord");
    
}

void MPEngine::mSetupBuffers() {
    //first create the gridlined quad
    _createGroundBuffers();

    //now throw in my chao hero
    _buildAndDrawChao();
}

void MPEngine::mSetupScene() {
    //create an arcball camera looking at world origin (for now)
    _pArcballCam = new ArcballCam(glm::vec3(0.0f), 20.f);
    _pArcballCam->setTheta(glm::radians(45.0f));
    _pArcballCam->setPhi(glm::radians(45.0f));
    _pArcballCam->recomputeOrientation();    

    mSetupShaders();

    mSetupBuffers();
}

/*
* ENGINE CLEANUP
*/

void MPEngine::mCleanupShaders() {
    delete _MPShaderProgram;
    _MPShaderProgram = nullptr;
}

void MPEngine::mCleanupBuffers() {}

void MPEngine::mCleanupScene() {}

/**
 * Rendering / Drawing functions
 */

void MPEngine::_renderScene(const glm::mat4& viewMtx, const glm::mat4& projMtx) const{
    //compute mvp matrix
    glm::mat4 modelMtx = glm::mat4(1.0f);
    glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
    //activate the shader program!
    _MPShaderProgram->useProgram();

    //send MVP matrix to GPU
    glUniformMatrix4fv(_MPShaderUniformLocations.mvpMtx, 1, GL_FALSE, &mvpMtx[0][0]);

    //draw grid lines
    glBindVertexArray(_groundVAO);
    glDrawArrays(GL_LINES, 0, _numGroundPoints);
    glBindVertexArray(0);

    //draw chao head
    if (_chaoHead) {
        glm::mat4 modelMtx = glm::mat4(1.0f);
        //translate upwards a bit (will replace this with where we really want to put it for transfromations later)
        modelMtx = glm::translate(modelMtx, glm::vec3(0.0f, 5.0f, 0.0f));
        //recompute mvp
        glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
        //activate shader program!
        _MPShaderProgram->useProgram();
        glUniformMatrix4fv(_MPShaderUniformLocations.mvpMtx, 1, GL_FALSE, &mvpMtx[0][0]);
        _chaoHead->draw(_MPShaderProgram->getShaderProgramHandle());
    }
}

void MPEngine::_updateScene() {}

void MPEngine::run(){
    while (!glfwWindowShouldClose(mpWindow)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 viewMtx = _pArcballCam->getViewMatrix();
        glm::mat4 projMtx = glm::perspective(glm::radians(45.0f), (float)mWindowWidth / mWindowHeight, 0.1f, 100.0f);

        _renderScene(viewMtx, projMtx);

        glfwSwapBuffers(mpWindow);
        glfwPollEvents();
    }
}

/**
 * PRIVATE HELPER FUNCTIONS
 */

 void MPEngine::_buildAndDrawChao() {
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
        // TODO #18 - set attribute
        _chaoHead->setAttributeLocations(_MPShaderAttributeLocations.vPos,
                                        _MPShaderAttributeLocations.vNormal,
                                        _MPShaderAttributeLocations.texCoord);
    } else {
        fprintf(stderr, "[ERROR]: Could not open OBJ Model for Head\n");
        delete _chaoHead;
        _chaoHead = nullptr;
    }
    /* COMMENTED OUT FOR NOW, AS I NEED TO GENERATE EACH .OBJ FILE FIRST, GOING TO TEST WITH ONE TO GET IT TO WORK
    //now the headBall
    if( _chaoHeadBall->loadModelFile("assets/models/Chaoparts/chaoHeadBall.obj" ) ) {
        // TODO #18 - set attribute
        _chaoHeadBall->setAttributeLocations(_MPShaderAttributeLocations.vPos,
                                        _MPShaderAttributeLocations.vNormal,
                                        _MPShaderAttributeLocations.texCoord);
    } else {
        fprintf(stderr, "[ERROR]: Could not open OBJ Model for HeadBall\n");
        delete _chaoHeadBall;
        _chaoHeadBall = nullptr;
    }
    */
    //now RArm
    //now LArm
    //now Body
    //now RFoot
    //now LFoot
    //now Tail
    //now Wings
 }

 void MPEngine::_createGroundBuffers() {
    std::vector<GLfloat> groundVertices;
    const GLfloat halfSize = WORLD_SIZE / 2.0f;
    const GLint numLines = 25;
    const GLfloat spacing = WORLD_SIZE / numLines;

    //lines parallel to X-axis
    for (int i = 0; i <= numLines; i++) {
        GLfloat z = -halfSize + i * spacing;
        groundVertices.insert(groundVertices.end(), {-halfSize, 0.0f, z, halfSize, 0.0f, z});
    }

    //lines parallel to z-axis
    for (int i = 0; i <= numLines; i++) {
        GLfloat x = -halfSize + i * spacing;
        groundVertices.insert(groundVertices.end(), {x, 0.0f, -halfSize, x, 0.0f, halfSize});
    }

    _numGroundPoints = static_cast<GLsizei>(groundVertices.size() / 3);

    GLuint vbo;
    glGenVertexArrays(1, &_groundVAO);
    glGenBuffers(1, &vbo);

    glBindVertexArray(_groundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, groundVertices.size() * sizeof(GLfloat), groundVertices.data(), GL_STATIC_DRAW);

    GLint posAttrib = _MPShaderAttributeLocations.vPos;
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0);
 }

 /**
  * CALLBACK FUNCTIONS
  */
void MP_keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {}

void MP_cursor_callback(GLFWwindow *window, double x, double y) {
    //get handle to engine
    auto engine = static_cast<MPEngine*>(glfwGetWindowUserPointer(window));

    if (engine->getLeftMouseButtoneState() == GLFW_PRESS) {
        //compute delta
        double dx = x - engine->getMousePosition().x;
        double dy = y - engine->getMousePosition().y;

        //update camera rotation (scale motion to radians)
        engine->getArcballcam()->rotateTheta(dx * 0.005f);
        engine->getArcballcam()->rotatePhi(dy * 0.005f);

        engine->getArcballcam()->recomputeOrientation();
    }

    engine->setMousePosition({x, y});
}

void MP_mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    //get handle
    auto engine = static_cast<MPEngine*>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        engine->setLeftMouseButtonState(action);
    }
}

void MP_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    auto engine = static_cast<MPEngine*>(glfwGetWindowUserPointer(window));
    if (!engine) return;

    if (yoffset > 0) engine->getArcballcam()->moveForward(0.5f);
    else if (yoffset < 0) engine->getArcballcam()->moveBackward(0.5f);
}
