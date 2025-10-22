/*
 *   Vertex Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 * Stew Nowak
 */

#version 410 core

// all uniform inputs
//create the MVP uniform
//create our uniform of type mat4 named mvpMatrix to pass in the ModelViewPorjection Matrix
uniform mat4 mvpMtx;

//create the time uniform
//uniform float time;
//HAVE THIS COMMENTED OUT FOR NOW, BUT COULD BE USEFUL LATER!!

// all attribute inputs
//create the position attribute
//create our attribute of type vec3 named vPosition that represents a vertex's position (ignoring the w=1 that we will hard code in later to save memory)
in vec3 vPosition;

// all varying outputs
//create the varying output
//use the 'out' keyword to create a varying variable of type vec3
//we use 'out' because output from vertex shader -> input to fragment shader
out vec3 theColor;


void main() {
    //*****************************************
    //********* Vertex Calculations  **********
    //*****************************************
    
    
    //set the transformed vertex position
    //set gl_position equal to the MVP matrix times the vertex
    gl_Position = mvpMtx * vec4(vPosition, 1.0);


    //set our varying
    theColor = vPosition;
}
