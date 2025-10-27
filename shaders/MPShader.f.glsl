/*
 *   Fragment Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 * Stew Nowak
 */

#version 410 core

// all uniform inputs from vertex shader
in vec3 vertexColor;
in vec2 vTexCoord;
in float texEnabled;
in vec3 vEmissiveColor;
in float emissiveEnabled;

//texture stuff
uniform sampler2D texMap;

// all fragment outputs
out vec4 fragColor;

void main() {
    vec3 color = vertexColor;
    //check for texturing
    if (texEnabled > 0.5) {
        vec4 texColor = texture(texMap, vTexCoord);
        color *= texColor.rgb;//modulate lighting with texture color
    }

    //check for emissive
    if (emissiveEnabled > 0.5) {
        color += vEmissiveColor;
    }

    fragColor = vec4(color, 1.0);
}
