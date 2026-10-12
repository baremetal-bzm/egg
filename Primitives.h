#pragma once
#include <learnopengl/shader.h>

struct Primitives {
    unsigned int skyboxVAO = 0, skyboxVBO = 0; //unit cube, pos
    unsigned int quadVAO = 0, quadVBO = 0;   //fullscreen NDC quad, pos+uv 
    unsigned int planeVAO = 0, planeVBO = 0;  //floor plane, pos+normal+uv 
    unsigned int cubeVAO = 0, cubeVBO = 0;   //test cubes, pos+uv 
};

Primitives BuildPrimitives();
void DestroyPrimitives(Primitives& prims);


//void renderFloor(const Shader& shader, const Primitives& prims);
void renderCubes(const Shader& shader, Primitives& prims); //not const because its scuffed and builds the vao/vbo ints inside it
