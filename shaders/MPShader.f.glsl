/*
 *   Fragment Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 * Stew Nowak
 */

#version 410 core

// all uniform inputs

// all varying inputs
// TODO #F3: create the varying input
//use the 'in' keyword because we are receiving "theColor" from the vertexShader
in vec3 theColor;


// all fragment outputs
// TODO #D: create the color output
//creat output variable with "out" keyword of type vec4 called fragColorOut
out vec4 fragColorOut;


void main() {
    //*****************************************
    //******* Final Color Calculations ********
    //*****************************************
    
    //specify the fragment color
    //set our output variable equal to white
    //fragColorOut = vec4(1, 1, 1, 1);

    //write out the varying color
    fragColorOut = vec4(theColor, 1); //like this more for now, but we can play with this later
}
