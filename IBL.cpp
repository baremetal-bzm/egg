#include "IBL.h"

#include <glad/glad.h>
#include <stb_image.h> 
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <iostream>

namespace {

    // Allocates an RGB16F cubemap (level 0 only) with clamp-to-edge wrapping.
    unsigned int CreateCubemap(int size, GLenum minFilter)
    {
        unsigned int tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        return tex;
    }

    void ResizeDepth(unsigned int rbo, int size)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
    }

    void DrawCube(unsigned int cubeVAO)
    {
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    void CheckFBO(const char* what)
    {
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::IBL:: framebuffer incomplete during " << what << std::endl;
    }

} // namespace

IBLMaps BuildIBL(const char* hdrPath, const IBLShaders& shaders,
    unsigned int cubeVAO, unsigned int quadVAO)
{
    IBLMaps maps;

    //PBR: load the HDR environment map
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float* data = stbi_loadf(hdrPath, &width, &height, &nrComponents, 0);
    if (!data)
    {
        std::cout << "Failed to load HDR image: " << hdrPath << std::endl;
        return maps; // all zeros; caller can check maps.valid()
    }
    // HDR files decode to 3 floats/pixel here; bail out rather than upload a mismatched format.
    if (nrComponents != 3)
    {
        std::cout << "HDR image has " << nrComponents << " channels, expected 3: " << hdrPath << std::endl;
        stbi_image_free(data);
        return maps;
    }

    //save previous gl state
    GLint prevFBO = 0, prevViewport[4] = { 0, 0, 0, 0 };
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
    glGetIntegerv(GL_VIEWPORT, prevViewport);

    //set up capture framebuffer (depth RBO is reused and resized for each pass)
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    ResizeDepth(captureRBO, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO); //depth attachment

    //load HDR texture 
    unsigned int hdrTexture;
    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);// textures data value set to float
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);

    //capture projection/views for the 6 cube faces
    const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    const glm::mat4 captureViews[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // STEP 1 HDR->env cubemap: setup cubemap to render to and attach to framebuffer=============
    maps.envCubemap = CreateCubemap(512, GL_LINEAR_MIPMAP_LINEAR);
    // NOTE (possible bug, behavior kept as-is): the LearnOpenGL tutorial uses
    // GL_LINEAR_MIPMAP_LINEAR here. With plain GL_LINEAR the mips generated below are never
    // sampled, so the prefilter pass can't use them to suppress the "visible dots" artifact.
    // To try the tutorial behavior, pass GL_LINEAR_MIPMAP_LINEAR above.

    shaders.equirectToCubemap.use();
    shaders.equirectToCubemap.setInt("equirectangularMap", 0);
    shaders.equirectToCubemap.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, 512, 512);//don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        shaders.equirectToCubemap.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, maps.envCubemap, 0); //color attachment
        if (i == 0) CheckFBO("env cubemap"); //can only check after both depth and color attachments
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        DrawCube(cubeVAO);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //let opengl generate mipmaps from the first mip face (combats visible dots artifact)
    glBindTexture(GL_TEXTURE_CUBE_MAP, maps.envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    //STEP 2 create an irradiance cubemap; re-scale capture FBO to irradiance scale =============
    maps.irradianceMap = CreateCubemap(32, GL_LINEAR); //smaller size(32) okay for this one

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    ResizeDepth(captureRBO, 32);

    //solve diffuse integral by convolution to create an irradiance (cube)map.
    shaders.irradiance.use();
    shaders.irradiance.setInt("environmentMap", 0);
    shaders.irradiance.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, maps.envCubemap);

    glViewport(0, 0, 32, 32);//configure viewport to the capture dimensions
    for (unsigned int i = 0; i < 6; ++i)
    {
        shaders.irradiance.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, maps.irradianceMap, 0);
        if (i == 0) CheckFBO("irradiance map");
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        DrawCube(cubeVAO);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //STEP 3 prefilter map (specular, roughness per mip), rescale FBO=========
    maps.prefilterMap = CreateCubemap(128, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP); // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory

    //run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map
    shaders.prefilter.use();
    shaders.prefilter.setInt("environmentMap", 0);
    shaders.prefilter.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, maps.envCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        const int mipSize = static_cast<int>(128 * std::pow(0.5, mip));
        ResizeDepth(captureRBO, mipSize);
        glViewport(0, 0, mipSize, mipSize);

        const float roughness = static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1);
        shaders.prefilter.setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            shaders.prefilter.setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, maps.prefilterMap, mip);
            if (i == 0 && mip == 0) CheckFBO("prefilter map");
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            DrawCube(cubeVAO);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //STEP 4 generate 2D LUT from BRDF eqns =======================
    glGenTextures(1, &maps.brdfLUT);
    //pre-allocate enough memory for the LUT texture.
    glBindTexture(GL_TEXTURE_2D, maps.brdfLUT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//be sure to set wrapping mode to GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //re-configure capture framebuffer object and render screen-space quad with BRDF shader
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    ResizeDepth(captureRBO, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, maps.brdfLUT, 0);
    CheckFBO("BRDF LUT");

    glViewport(0, 0, 512, 512);
    shaders.brdf.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //render quad
    //double check we dont need to disable depth test here
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    //do cleanup and restore caller state
    glDeleteTextures(1, &hdrTexture);
    glDeleteRenderbuffers(1, &captureRBO);
    glDeleteFramebuffers(1, &captureFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<unsigned int>(prevFBO));
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);

    //alternatively do this but needs glfw context
    //int scrWidth, scrHeight;
    //glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    //glViewport(0, 0, scrWidth, scrHeight);

    return maps;
}

void DestroyIBL(IBLMaps& maps)
{
    unsigned int textures[4] = { maps.envCubemap, maps.irradianceMap, maps.prefilterMap, maps.brdfLUT };
    glDeleteTextures(4, textures); // zeros are silently ignored by GL
    maps = IBLMaps{};
}
