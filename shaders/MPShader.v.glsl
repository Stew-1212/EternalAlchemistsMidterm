/*
 *   Vertex Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 * Stew Nowak
 */

#version 410 core

//all vertex Attributes
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 vColor;

//all Uniforms
uniform mat4 mvpMtx;
uniform mat3 normMtx;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 matColor;
uniform bool useTexture;
uniform bool useVertexColor;
uniform vec3 emissiveColor;
uniform bool useEmissive;

//outputs to fragment shader
out vec3 vertexColor;
out vec2 vTexCoord;
out float texEnabled;
out vec3 vEmissiveColor;
out float emissiveEnabled;

void main() {
    //*****************************************
    //********* Vertex Calculations  **********
    //*****************************************

    //emissive color stuff
    vEmissiveColor = emissiveColor;
    emissiveEnabled = useEmissive ? 1.0 : 0.0;

    //texture coord stuff
    vTexCoord = texCoord;
    texEnabled = useTexture ? 1.0: 0.0;
    
    //transform vertex position
    gl_Position = mvpMtx * vec4(vPosition, 1.0);
    
    //combine vColor and matColor for base material color and if we don't use vertex color just use matColor
    vec3 baseColor = useVertexColor ? vColor * matColor : matColor;
    
    //LIGHTING
    //normalize normal after transformation
    vec3 N = normalize(normMtx * vNormal);
    vec3 L = normalize(-lightDir); //ensure pointing toward light
    //view direction (viewer at origin)
    vec3 V = normalize(vec3(0.0, 0.0, 1.0));
    //Reflection vector
    vec3 R = reflect(-L, N);

    //PHONG ILLUMINATION COMPONENTS
    //ambient
    vec3 ambient = 0.25 * baseColor; //tweak this as seen fit
    //diffuse
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor * baseColor;
    //specular
    float shininess = 20.0; //tweak this as seen fit
    float spec = pow(max(dot(R, V), 0.0), shininess);
    vec3 specular = spec * lightColor;
    
    //final vertex color
    vertexColor = ambient + diffuse + specular;
}
