#pragma once

//depth-only FBO used for shadow mapping
struct ShadowMap {
    unsigned int FBO = 0;           
    unsigned int depthTexture = 0;
};

//color+depth/stencil FBO the scene renders into for postprocessing
struct PostProcessTarget {
    unsigned int FBO = 0;
    unsigned int colorTexture = 0;
    unsigned int RBO = 0; // depth24_stencil8
};

//floating point FBO with two color attachments: [0] lit scene [1] bright-pass for bloom
struct HDRTarget {
    unsigned int FBO = 0;
    unsigned int colorBuffers[2] = { 0, 0 };
    unsigned int depthRBO = 0;
};

//two FBO/texture pairs the Gaussian blur bounces between (no depth buffer needed)
struct PingPongTarget {
    unsigned int FBO[2] = { 0, 0 };
    unsigned int colorBuffer[2] = { 0, 0 };
};

struct RenderTargets {
    ShadowMap shadow;
    PostProcessTarget post;
    HDRTarget hdr;
    PingPongTarget pingpong;
};

//width/height: size for post, hdr and pingpong (SCR_WIDTH/SCR_HEIGHT)
RenderTargets BuildRenderTargets(int width, int height, int shadowWidth, int shadowHeight);
void ResizeRenderTargets(RenderTargets& rt, int width, int height);
void DestroyRenderTargets(RenderTargets& rt);
