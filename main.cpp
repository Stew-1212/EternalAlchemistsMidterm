/*
 *  CSCI 441, Computer Graphics, Fall 2025
 *
 *  Project: A3
 *  File: main.cpp
 *
 *  Description:
 *      This file contains the basic setup to work with GLSL shaders.
 *
 *  Author: Dr. Paone, Colorado School of Mines, 2025
 *
 */

#include "MPEngine.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

///*****************************************************************************
//
// Our main function
int main() {
    const auto Engine = new MPEngine();
    Engine->initialize();
    if (Engine->getError() == CSCI441::OpenGLEngine::OPENGL_ENGINE_ERROR_NO_ERROR) {
        Engine->run();
    }
    Engine->shutdown();
    delete Engine;
	return EXIT_SUCCESS;
}
