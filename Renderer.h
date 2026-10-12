#pragma once
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <glm/glm/glm.hpp>
#include <dualnum.h>

#include "Primitives.h"
#include "RenderTargets.h"
#include "IBL.h"
#include "GameEnums.h"

//textures the Renderer binds while drawing. loaded in main() via loadTexture/loadCubemap and handed in
struct SceneTextures {
    unsigned int sphereTexture;
    unsigned int normalTexture;
    unsigned int depthTexture;
    unsigned int cubeTexture;
    unsigned int floorTexture;
    unsigned int cubemapTexture; // skybox
    unsigned int roughnessMap;
    unsigned int metalnessMap;
};

//Primitives/RenderTargets/IBLMaps/Sphere not owned (built in main()). shaders and texture handles ARE owned/copied in here
class Renderer {
public:
    //screenWidth/Height, shadowWidth/Height same values used to build RenderTargets
    Renderer(Primitives& prims, RenderTargets& renderTargets, IBLMaps& ibl, dualnum::Sphere& sphere,
        const SceneTextures& textures,
        int screenWidth, int screenHeight, int shadowWidth, int shadowHeight);

    //sets uniforms that are the same for every draw call this frame 
    //call once per frame before RenderFrame()
    void BeginFrame(const Camera& camera, const glm::mat4& view, const glm::mat4& projection,
        const glm::mat4& sphereModel, const glm::mat4& sphereView, const glm::mat4& sphereProjection,
        const glm::vec3& lightPos, float parallaxScaleFactor);

    //render the frame
    void RenderFrame(VisualMode vmode, GameState gmode, float explodeTime,
        bool useHDR, int useNormals, bool bloomEnabled, float exposure);

    //update dimensions when resizing screen
    void UpdateDimensions(unsigned int Width, unsigned int Height);

private:
    // ---- external resources (not owned) ----
    Primitives* prims_;
    RenderTargets* renderTargets_;
    IBLMaps* ibl_;
    dualnum::Sphere* sphere_;
    const Camera* camera_ = nullptr; // set by BeginFrame; only valid until the next BeginFrame

    // ---- owned shaders ----
    Shader sphereShader_;
    Shader sphereShaderGG_;       
    Shader sphereShaderNormals_;  // debug normals overlay
    Shader cubeShader_;           
    Shader skyboxShader_;
    Shader depthMapShader_;
    Shader floorShader_;
    Shader sobelShader_;
    Shader gaussianBlurShader_;
    Shader bloomFinalShader_;
    Shader pbrShader_;
    Shader backgroundShader_;

    // ---- owned texture handles (loaded by the caller, copied in) ----
    SceneTextures textures_;

    // ---- fixed sizes, same as whatever built renderTargets_ ----
    int screenWidth_, screenHeight_, shadowWidth_, shadowHeight_;

    // ---- per-frame state cached by BeginFrame() ----
    glm::mat4 view_{}, projection_{};
    glm::mat4 sphereModel_{}, sphereView_{}, sphereProjection_{};
    glm::vec3 lightPos_{};
    float parallaxScaleFactor_ = 0.0f;

    // ---- draw steps ----
    void DrawSphere(int useNormals);
    void DrawCubes(Shader& shader);
    void DrawQuad();
    void DrawSkybox();
    void DrawHDRAndBloom(bool bloomEnabled, float exposure);
    void DrawShadowMap(bool useHDR); 
    void DrawFloor();
    void DrawSpherePBR();
};
