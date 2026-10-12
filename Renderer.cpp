#include "Renderer.h"

#include <glad/glad.h>
#include <glm/glm/gtc/matrix_transform.hpp>

Renderer::Renderer(Primitives& prims, RenderTargets& renderTargets, IBLMaps& ibl, dualnum::Sphere& sphere,
    const SceneTextures& textures,
    int screenWidth, int screenHeight, int shadowWidth, int shadowHeight)
    : prims_(&prims), renderTargets_(&renderTargets), ibl_(&ibl), sphere_(&sphere)
    , sphereShader_("shaders/DualSphere.vs", "shaders/DualSphere.fs") //build out the shaders here, default sphere shader
    , sphereShaderGG_("shaders/DualSphere.vs", "shaders/DualSphere.fs", "shaders/DualSphere.gs") //gameover shader
    , sphereShaderNormals_("shaders/DualSphereNormals.vs", "shaders/DualSphereNormals.fs", "shaders/DualSphereNormals.gs") //normals for debugging
    , cubeShader_("shaders/1.1.depth_testing.vs", "shaders/1.1.depth_testing.fs") //debug boxes
    , skyboxShader_("shaders/skybox.vs", "shaders/skybox.fs") //skybox
    , depthMapShader_("shaders/depthMap.vs", "shaders/depthMap.fs") //for shadow mapping
    , floorShader_("shaders/floorShader.vs", "shaders/floorShader.fs") //floor
    , sobelShader_("shaders/Sobel.vs", "shaders/Sobel.fs") //sobel post processing filter
    , gaussianBlurShader_("shaders/gaussianBlur.vs", "shaders/gaussianBlur.fs") //pp
    , bloomFinalShader_("shaders/bloomFinal.vs", "shaders/bloomFinal.fs") //pp
    , pbrShader_("shaders/PBR.vs", "shaders/PBR.fs") //pbr
    , backgroundShader_("shaders/PBR_sample_cubemap.vs", "shaders/PBR_sample_cubemap.fs") //pbr cubemap
    , textures_(textures)
    , screenWidth_(screenWidth), screenHeight_(screenHeight)
    , shadowWidth_(shadowWidth), shadowHeight_(shadowHeight)
{
    // one-time sampler bindings
    cubeShader_.use();
    cubeShader_.setInt("texture1", 0);
    sphereShader_.use();
    sphereShader_.setInt("diffuseTexture", 0);
    sphereShader_.setInt("normalTexture", 1);
    sphereShader_.setInt("depthTexture", 2);
    sphereShaderGG_.use();
    sphereShaderGG_.setInt("diffuseTexture", 0);
    sphereShaderGG_.setInt("normalTexture", 1);
    sphereShaderGG_.setInt("depthTexture", 2);
    skyboxShader_.use();
    skyboxShader_.setInt("skybox", 0);
    floorShader_.use();
    floorShader_.setInt("diffuseTexture", 0);
    floorShader_.setInt("shadowMap", 1);
    gaussianBlurShader_.use();
    gaussianBlurShader_.setInt("image", 0);
    bloomFinalShader_.use();
    bloomFinalShader_.setInt("scene", 0);
    bloomFinalShader_.setInt("bloomBlur", 1);
    pbrShader_.use();
    pbrShader_.setInt("albedoMap", 0);
    pbrShader_.setInt("normalMap", 1);
    pbrShader_.setInt("metallicMap", 2);
    pbrShader_.setInt("roughnessMap", 3);
    pbrShader_.setInt("irradianceMap", 4);
    pbrShader_.setInt("prefilterMap", 5);
    pbrShader_.setInt("brdfLUT", 6);
    backgroundShader_.use();
    backgroundShader_.setInt("environmentMap", 0);
    sobelShader_.use();
    sobelShader_.setInt("screenTexture", 0);
    sobelShader_.setFloat("scrWidth", screenWidth_);
    sobelShader_.setFloat("scrHeight", screenHeight_);
}

void Renderer::UpdateDimensions(const unsigned int Width, const unsigned int Height)
{
    screenWidth_ = Width;
    screenHeight_ = Height;
    sobelShader_.use();
    sobelShader_.setFloat("scrWidth", screenWidth_);
    sobelShader_.setFloat("scrHeight", screenHeight_);
};

void Renderer::BeginFrame(const Camera& camera, const glm::mat4& view, const glm::mat4& projection,
    const glm::mat4& sphereModel, const glm::mat4& sphereView, const glm::mat4& sphereProjection,
    const glm::vec3& lightPos, float parallaxScaleFactor)
{//depending on final shape of game, some of this can likely be moved outside the render loop, e.g. lightpos, view matrices
    camera_ = &camera;
    view_ = view;
    projection_ = projection;
    sphereModel_ = sphereModel;
    sphereView_ = sphereView;
    sphereProjection_ = sphereProjection;
    lightPos_ = lightPos;
    parallaxScaleFactor_ = parallaxScaleFactor;

    cubeShader_.use();
    cubeShader_.setMat4("view", view_);
    cubeShader_.setMat4("projection", projection_);

    floorShader_.use();
    floorShader_.setMat4("model", glm::mat4(1.0f));
    floorShader_.setMat4("view", view_);
    floorShader_.setMat4("projection", projection_);

    sphereShader_.use();
    sphereShader_.setMat4("view", sphereView_);
    sphereShader_.setMat4("projection", sphereProjection_);
    sphereShader_.setMat4("model", sphereModel_);
    sphereShader_.setVec3("viewPos", camera.Position);
    sphereShader_.setVec3("lightPos", lightPos_);
    sphereShader_.setFloat("parallaxScaleFactor", parallaxScaleFactor_);

    pbrShader_.use();
    pbrShader_.setVec3("lightPos", lightPos_);
    pbrShader_.setVec3("lightColor", glm::vec3(10.0f, 10.0f, 10.0f));
    pbrShader_.setVec3("camPos", camera.Position);
    pbrShader_.setMat4("model", sphereModel_);
    pbrShader_.setMat4("view", sphereView_);
    pbrShader_.setMat4("projection", sphereProjection_);

    backgroundShader_.use();
    backgroundShader_.setMat4("projection", projection_);
}



void Renderer::DrawSphere(int useNormals)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures_.sphereTexture);//maybe move this inside sphere class
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures_.normalTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures_.depthTexture);
    glEnable(GL_CULL_FACE);
    sphere_->Draw();
    if (useNormals == 1)
    {
        sphereShaderNormals_.use();
        sphereShaderNormals_.setMat4("view", sphereView_);
        sphereShaderNormals_.setMat4("projection", sphereProjection_);
        sphereShaderNormals_.setMat4("model", sphereModel_);
        sphere_->Draw();
    }
}

void Renderer::DrawCubes(Shader& shader)
{
    shader.use();
    glEnable(GL_CULL_FACE);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures_.cubeTexture);
    renderCubes(shader, *prims_);
    glDisable(GL_CULL_FACE);
}

void Renderer::DrawQuad()
{
    glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
    glBindVertexArray(prims_->quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

void Renderer::DrawSkybox()
{
    glDepthFunc(GL_LEQUAL);
    skyboxShader_.use();
    glm::mat4 viewSkyboxFlipped = glm::mat4(glm::mat3(view_)); //remove translation part of view matrix
    skyboxShader_.setMat4("view", viewSkyboxFlipped);
    skyboxShader_.setMat4("projection", projection_);
    glBindVertexArray(prims_->skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textures_.cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

void Renderer::DrawHDRAndBloom(bool bloomEnabled, float exposure)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // two-pass Gaussian blur of the bright-pass buffer
    bool horizontal = true, first_iteration = true;
    const unsigned int amount = 10;
    gaussianBlurShader_.use();
    for (unsigned int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, renderTargets_->pingpong.FBO[horizontal]);
        gaussianBlurShader_.setInt("horizontal", horizontal);
        glBindTexture(GL_TEXTURE_2D, first_iteration ? renderTargets_->hdr.colorBuffers[1]
            : renderTargets_->pingpong.colorBuffer[!horizontal]);
        DrawQuad();
        horizontal = !horizontal;
        if (first_iteration) first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    bloomFinalShader_.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTargets_->hdr.colorBuffers[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, renderTargets_->pingpong.colorBuffer[!horizontal]);
    bloomFinalShader_.setInt("bloom", bloomEnabled);
    bloomFinalShader_.setFloat("exposure", exposure);
    DrawQuad();
}

void Renderer::DrawShadowMap(bool useHDR)
{
    //render depth of scene from the lights point of view
    //float near_plane = 1.0f, far_plane = 7.5f;
    float lightDistance = glm::length(lightPos_);
    float sceneRadius = 3.0f;              //picked to cover egg and cubes
    float near_plane = std::max(0.1f, lightDistance - sceneRadius);
    float far_plane = lightDistance + sceneRadius;
    float lrbt = 5.0f; //left right bottom top, i.e. ortho dimensions
    //lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
    glm::mat4 lightProjection = glm::ortho(-lrbt, lrbt, -lrbt, lrbt, near_plane, far_plane); // ortho: directional light
    glm::mat4 lightView = glm::lookAt(lightPos_, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    //set floor shaders for later use
    floorShader_.use();
    floorShader_.setVec3("viewPos", camera_->Position);
    floorShader_.setVec3("lightPos", lightPos_);
    floorShader_.setMat4("lightSpaceMatrix", lightSpaceMatrix);

    //render scene from light's point of view
    depthMapShader_.use();
    depthMapShader_.setMat4("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, shadowWidth_, shadowHeight_);
    glBindFramebuffer(GL_FRAMEBUFFER, renderTargets_->shadow.FBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    depthMapShader_.use();
    depthMapShader_.setMat4("model", sphereModel_);//note sphereModel is static
    DrawSphere(0);
    //DrawCubes(depthMapShader_);
    glBindVertexArray(prims_->planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, useHDR ? renderTargets_->hdr.FBO : 0);
    glViewport(0, 0, screenWidth_, screenHeight_);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::DrawFloor()
{
    glDisable(GL_CULL_FACE);
    floorShader_.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures_.floorTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, renderTargets_->shadow.depthTexture);
    glBindVertexArray(prims_->planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_CULL_FACE);
}

void Renderer::DrawSpherePBR()
{
    pbrShader_.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures_.sphereTexture);//maybe move this inside sphere class
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures_.normalTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures_.metalnessMap);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, textures_.roughnessMap);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ibl_->irradianceMap);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ibl_->prefilterMap);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, ibl_->brdfLUT);
    glEnable(GL_CULL_FACE);
    sphere_->Draw();

    //skybox last to avoid overdraw
    glDepthFunc(GL_LEQUAL);
    backgroundShader_.use();
    backgroundShader_.setMat4("view", view_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ibl_->envCubemap);
    //glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
    //glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); // display prefilter map
    glBindVertexArray(prims_->skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    // render BRDF map to screen
    //brdfShader.use();
    //drawQuad();
}

void Renderer::RenderFrame(VisualMode vmode, GameState gmode, float explodeTime,
    bool useHDR, int useNormals, bool bloomEnabled, float exposure)
{
    if (gmode == ACTIVE)
    {
        switch (vmode)
        {
        case NORMAL:
            //render normally
            //DrawCubes(cubeShader_);
            DrawSkybox();
            sphereShader_.use();
            DrawSphere(useNormals);
            if (useHDR) DrawHDRAndBloom(bloomEnabled, exposure);
            break;

        case XRAY:
            break;

        case SOBEL:
            //edge detection algo
            //bind to framebuffer and draw to texture for post processing
            glBindFramebuffer(GL_FRAMEBUFFER, renderTargets_->post.FBO);
            glEnable(GL_DEPTH_TEST);
            //clear framebuffer
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //render
            sphereShader_.use();
            DrawSphere(useNormals);
            //DrawCubes(cubeShader_);
            DrawSkybox();
            //bind back to default framebuffer and draw quad with framebuffer texture
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // clear all relevant buffers
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);// set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
            glClear(GL_COLOR_BUFFER_BIT);

            sobelShader_.use();
            glBindTexture(GL_TEXTURE_2D, renderTargets_->post.colorTexture);// use the color attachment texture as the texture of the quad plane
            DrawQuad();
            break;

        case GLASSES:
            //calc shadowmap first
            DrawShadowMap(useHDR);
            // render normally
            sphereShader_.use();
            DrawSphere(useNormals);
            //DrawCubes(cubeShader_);
            DrawFloor();
            if (useHDR) DrawHDRAndBloom(bloomEnabled, exposure);
            break;

        case PBR:
            DrawSpherePBR();
            break;
        }
    }
    else if (gmode == INACTIVE)
    {
       // DrawCubes(cubeShader_);
        DrawSkybox();
        glDisable(GL_CULL_FACE);
        sphereShaderGG_.use();
        sphereShaderGG_.setFloat("time", explodeTime);
        sphereShaderGG_.setMat4("projection", sphereProjection_);
        sphereShaderGG_.setMat4("view", sphereView_);
        sphereShaderGG_.setMat4("model", sphereModel_);
        sphereShaderGG_.setVec3("viewPos", camera_->Position);
        sphereShaderGG_.setVec3("lightPos", lightPos_);
        sphereShaderGG_.setFloat("parallaxScaleFactor", parallaxScaleFactor_);
        DrawSphere(0);
    }
}
