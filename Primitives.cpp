#include "Primitives.h"

#include <glad/glad.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

Primitives BuildPrimitives()
{
    Primitives prims;

    //----skybox: unit cube-----------------------------------------------
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    glGenVertexArrays(1, &prims.skyboxVAO);
    glGenBuffers(1, &prims.skyboxVBO);
    glBindVertexArray(prims.skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, prims.skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    //----fullscreen quad-----------------------------------------------------
    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    glGenVertexArrays(1, &prims.quadVAO);
    glGenBuffers(1, &prims.quadVBO);
    glBindVertexArray(prims.quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, prims.quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    //floor plane for shadows / glasses mode
    float planeVertices[] = {
        // positions            // normals         // texcoords
         25.0f, -1.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -1.0f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -25.0f, -1.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

         25.0f, -1.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -1.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         25.0f, -1.0f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
    };
    glGenVertexArrays(1, &prims.planeVAO);
    glGenBuffers(1, &prims.planeVBO);
    glBindVertexArray(prims.planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, prims.planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); //pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1); //norm
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); //uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // cubeVAO/cubeVBO stay at 0 here; renderCubes() below builds them on its first call.
    return prims;
}

void DestroyPrimitives(Primitives& prims)
{
    // glDeleteVertexArrays/Buffers should ignore 0 so should be fine for uninitialized vaos/vbos
    unsigned int vaos[] = { prims.skyboxVAO, prims.quadVAO, prims.planeVAO, prims.cubeVAO };
    unsigned int vbos[] = { prims.skyboxVBO, prims.quadVBO, prims.planeVBO, prims.cubeVBO };
    glDeleteVertexArrays(4, vaos);
    glDeleteBuffers(4, vbos);
    prims = Primitives{};
}

//void renderFloor(const Shader& shader, const Primitives& prims)
//{
//    glm::mat4 model = glm::mat4(1.0f);
//    shader.setMat4("model", model);
//    glBindVertexArray(prims.planeVAO);
//    glDrawArrays(GL_TRIANGLES, 0, 6);
//}

// draws two fixed cubes, mostly used for debugging some stuff
void renderCubes(const Shader& shader, Primitives& prims)
{
    if (prims.cubeVAO == 0)
    {
        float cubeVertices[] = {
            // Back face
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
             0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right         
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
            // Front face
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
            // Left face
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
            // Right face
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right         
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
             0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left     
             // Bottom face
             -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
              0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
              0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
              0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
             -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
             -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
             // Top face
             -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
              0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
              0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right     
              0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
             -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
             -0.5f,  0.5f,  0.5f,  0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &prims.cubeVAO);
        glGenBuffers(1, &prims.cubeVBO);
        glBindVertexArray(prims.cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, prims.cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0); //pos
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1); //uv
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glBindVertexArray(0);
    }

    glm::mat4 model = glm::mat4(1.0f);
    glBindVertexArray(prims.cubeVAO);
    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
    shader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
    shader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}
