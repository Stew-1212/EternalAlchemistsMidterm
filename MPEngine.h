#ifndef MP_ENGINE_H
#define MP_ENGINE_H
//includew the header files from our class library that we will need
#include <CSCI441/ModelLoader.hpp>
#include <CSCI441/OpenGLEngine.hpp>
#include <CSCI441/ShaderProgram.hpp>
#include "ArcballCam.h"

//begin defining the MP Engine class
class MPEngine final : public CSCI441::OpenGLEngine {
    public:
        //constructor, destructor, and run function
        MPEngine();
        ~MPEngine();

        void run() override;

        //function that returns a pointer to our arcball camera
        ArcballCam* getArcballcam() const {return _pArcballCam;}
        /*
        *NEED THESE FOR THE ARCBALL IMPLEMENTATION
        */
        //function that sets mouse position for camera
        void setMousePosition(const glm::vec2 mousePos) {_mousePosition = mousePos;}
        //function that gets mouse position for camera
        glm::vec2 getMousePosition() {return {_mousePosition.x, _mousePosition.y};}
        //function that returns the state of the left mosue button
        GLint getLeftMouseButtoneState() const {return _leftMouseButtonState;}
        //function that sets the left mouse button state
        void setLeftMouseButtonState(const GLint state) {_leftMouseButtonState = state;}
        //default mouse value to represent mouse has not begun interacting with the window yet
        const GLfloat MOUSE_UNINITIALIZED = -9999.0f;

        //function to update the chao's directional heading
        void _updateChaoHeading(float angle);
        //function to move around the chao
        void _updateChaoPos(float moveAmount);
        //function to reset the body angles of the chao
        void _resetBodyState();
        //function to randomly change the chao's mat color
        void _changeChaoCol();


    private:
        /***************************************************************************
         * FUNCTIONS FOR SCENE SETUP, CLEANUP, GENERATION, RENDERING, AND UPDATING *
         ***************************************************************************
         */
        //engine setup
        void mSetupGLFW() override;
        void mSetupOpenGL() override;
        void mSetupShaders() override;
        void mSetupBuffers() override;
        void mSetupScene() override;
        
        //engine cleanup
        void mCleanupScene() override;
        void mCleanupBuffers() override;
        void mCleanupShaders() override;

        //generate, render, and update scene functions
        void _generateEnvironment();
        void _renderScene(const glm::mat4& viewMtx, const glm::mat4& projMtx) const;
        void _updateScene();

        /*
        ******************************************************
        * Environment variables: camera, modelPtr, grid, etc.*
        * ****************************************************
        */
        //CAMERA STUFF
        //arcball cam we need to implement
        ArcballCam* _pArcballCam;
        //variables for our arcball camera
        glm::vec2 _cameraAngle;
        glm::vec2 _mousePosition;
        GLint _leftMouseButtonState;

        //OBJECT/MODEL STUFF
        //.obj models where we will load in each part of our Chao
        CSCI441::ModelLoader* _chaoHead;
        CSCI441::ModelLoader* _chaoHeadBall;
        CSCI441::ModelLoader* _chaoRArm;
        CSCI441::ModelLoader* _chaoLArm;
        CSCI441::ModelLoader* _chaoBody;
        CSCI441::ModelLoader* _chaoRFoot;
        CSCI441::ModelLoader* _chaoLFoot;
        CSCI441::ModelLoader* _chaoTail;
        CSCI441::ModelLoader* _chaoWings;//might need to split this into two seperate...we'll see
        //function to build the chao from all the parts
        void _buildChao();
        //function to draw the chao from all loaded in parts
        void _drawChao(const glm::mat4& viewMtx, const glm::mat4& projMtx) const;
        //variable for changing chao color randomly
        glm::vec3 _chaoMatCol;
        //variables and function for passive animation of head ball
        glm::vec3 _ballPos;
        float _theta;
        float _thetaSpeed;
        bool _spiralOut;
        float _thetaMax;
        glm::vec3 _ballCenter;
        void _animateBall();
        //variables for moving the Chao and animating the feet, arms, and head
        glm::vec3 _chaoHeading;
        glm::vec3 _chaoPosOffset;
        bool _isMoving;
        glm::vec3 _chaoPos; //need this variable for the camera
        void _animateBody();
        bool _origAngle;
        float _armAngle;
        float _armAngle2;
        float _footAngle;
        float _headAngle;


        //GRID STUFF
        //size of the world/ground plane
        GLfloat WORLD_SIZE = 180.0f;
        //VAO for the ground plane
        GLuint _groundVAO;
        //number of points that make up our ground 
        GLsizei _numGroundPoints;
        //function that creates the ground VAO
        void _createGroundBuffers();
        //function that draws the ground grid
        void _drawGroundGrid(const glm::mat4& viewMtx, const glm::mat4& projMtx) const;
        
        //function to utilize class objects.hpp file to create the environment
        void _drawEnvironment(const glm::mat4& viewMtx, const glm::mat4& projMtx) const; //this will actually be a drawing function so must be const type
        //add two vectors to store the positional and color data (randomly generated) for the environment
        std::vector<glm::vec3> _starPositions;
        std::vector<glm::vec3> _starColors;
        float _starAngle;
        

        /**********************************************
         * SHADER STUFF SUCH AS UNIFORMS, ATTRIBUTES, *
         **********************************************
         */
        //shader program that performs full phong illumination model and texturing (for Chao)
        CSCI441::ShaderProgram* _MPShaderProgram;

        //struct that will store the locations of all our shader uniforms
        struct MPShaderUniformLocations {
            //precomputed MVP matrix location
            GLint mvpMtx;
            //material diffuse color location
            GLint materialColor;
            //light direction Uniform
            GLint lightDir;
            //light color Uniform
            GLint lightColor;
            //WILL LIKELY NEED TO ADD MORE THINGS HERE FOR THE REST OF THE PHONG ILLUMINATION MODEL: SPECULAR AND AMBIENT  
            //normMtx uniform
            GLint normMtx;
            //texture map Uniform
            GLint texMap;
            //useTexture bool
            GLint useTexture;
            //useVertexColor bool
            GLint useVertexColor;
            //emissive color
            GLint emissiveColor;
            //use emissive bool
            GLint useEmissive;
        } _MPShaderUniformLocations;

        //struc that will store the locations of all our shader attributes
        struct MPShaderAttributeLocations {
            //vertex position location
            GLint vPos;
            //vertex normal location
            GLint vNormal;
            //texture coordinate
            GLint texCoord;
            //vertex color
            GLint vColor;
        } _MPShaderAttributeLocations;


};

//CALLBACK FUNCTIONS
void MP_keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void MP_cursor_callback(GLFWwindow *window, double x, double y);
void MP_mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void MP_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

#endif