#pragma once
#include <learnopengl/shader.h>

// Results of the image-based-lighting precompute. All zero if the HDR failed to load.
struct IBLMaps {
    unsigned int envCubemap = 0; // 512^2 cubemap of the environment (also used as the PBR background)
    unsigned int irradianceMap = 0; // 32^2 diffuse irradiance cubemap
    unsigned int prefilterMap = 0; // 128^2 specular prefilter cubemap, 5 mips
    unsigned int brdfLUT = 0; // 512^2 RG16F BRDF lookup table

    bool valid() const { return envCubemap != 0; }
};

struct IBLShaders {
    const Shader& equirectToCubemap; // generate_cubemap.vs + hdr_to_cubemap.fs
    const Shader& irradiance;        // generate_cubemap.vs + irradiance_convolution.fs
    const Shader& prefilter;         // generate_cubemap.vs + prefilter.fs
    const Shader& brdf;              // brdf.vs + brdf.fs
};

// Loads the equirectangular .hdr at hdrPath and bakes all four maps
// cubeVAO: unit cube (36 verts), quadVAO: fullscreen quad (6 verts)
// Requires a current GL context. Restores the caller's framebuffer binding and viewport
// On failure returns an IBLMaps with all handles 0 (binding texture 0 just samples black)
IBLMaps BuildIBL(const char* hdrPath, const IBLShaders& shaders,
    unsigned int cubeVAO, unsigned int quadVAO);

// Frees the four textures. Safe to call on a default-constructed/invalid IBLMaps.
void DestroyIBL(IBLMaps& maps);
