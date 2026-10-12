#include "RenderTargets.h"

#include <glad/glad.h>
#include <iostream>
#include <algorithm>

namespace {

    ShadowMap BuildShadowMap(int width, int height)
    {
        ShadowMap sm;

        glGenFramebuffers(1, &sm.FBO);
        // create depth texture
        glGenTextures(1, &sm.depthTexture);
        glBindTexture(GL_TEXTURE_2D, sm.depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);//texture img for shadow map
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // clamp to border to prevent arbitrary shadow regions
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        //attach depth texture as FBOs depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, sm.FBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sm.depthTexture, 0);
        glDrawBuffer(GL_NONE); // depth only: no color attachment to draw/read
        glReadBuffer(GL_NONE);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: shadow map framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return sm;
    }

    PostProcessTarget BuildPostProcessTarget(int width, int height)
    {
        PostProcessTarget pp;
        glGenFramebuffers(1, &pp.FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, pp.FBO);

        glGenTextures(1, &pp.colorTexture);
        glBindTexture(GL_TEXTURE_2D, pp.colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        //attach it to currently bound framebuffer object
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pp.colorTexture, 0);
        //depth/stencil attachment
        glGenRenderbuffers(1, &pp.RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, pp.RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, pp.RBO);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: post-process framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return pp;
    }

    HDRTarget BuildHDRTarget(int width, int height)
    {
        HDRTarget hdr;
        glGenFramebuffers(1, &hdr.FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, hdr.FBO);

        // create 2 floating point color buffers (0 for normal rendering, 1 for pulling brightness>threshold value)
        glGenTextures(2, hdr.colorBuffers);
        for (unsigned int i = 0; i < 2; i++)
        {
            glBindTexture(GL_TEXTURE_2D, hdr.colorBuffers[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // clamp to edge as the blur filter would otherwise sample repeated texture values
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            // attach texture to framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, hdr.colorBuffers[i], 0);
        }
        //create and attach depth buffer (renderbuffer)
        glGenRenderbuffers(1, &hdr.depthRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, hdr.depthRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height); //no stencil needed here
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdr.depthRBO);
        //tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
        GLenum attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, attachments);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return hdr;
    }

    PingPongTarget BuildPingPongTarget(int width, int height)
    {
        PingPongTarget pp;
        glGenFramebuffers(2, pp.FBO);
        glGenTextures(2, pp.colorBuffer);
        for (unsigned int i = 0; i < 2; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pp.FBO[i]);
            glBindTexture(GL_TEXTURE_2D, pp.colorBuffer[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//clamp to edge as blur filter would otherwise sample repeated texture values
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pp.colorBuffer[i], 0);
            // no depth buffer needed for a blur pass
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "Framebuffer not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return pp;
    }

    void ResizePostProcessTarget(PostProcessTarget& pp, int width, int height)
    {
        glBindTexture(GL_TEXTURE_2D, pp.colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glBindRenderbuffer(GL_RENDERBUFFER, pp.RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    }

    void ResizeHDRTarget(HDRTarget& hdr, int width, int height)
    {
        for (unsigned int i = 0; i < 2; i++)
        {
            glBindTexture(GL_TEXTURE_2D, hdr.colorBuffers[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        }
        glBindRenderbuffer(GL_RENDERBUFFER, hdr.depthRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    }

    void ResizePingPongTarget(PingPongTarget& pp, int width, int height)
    {
        for (unsigned int i = 0; i < 2; i++)
        {
            glBindTexture(GL_TEXTURE_2D, pp.colorBuffer[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        }
    }
} // namespace

RenderTargets BuildRenderTargets(int width, int height, int shadowWidth, int shadowHeight)
{
    RenderTargets rt;
    rt.shadow = BuildShadowMap(shadowWidth, shadowHeight);
    rt.post = BuildPostProcessTarget(width, height);
    rt.hdr = BuildHDRTarget(width, height);
    rt.pingpong = BuildPingPongTarget(width, height);
    return rt;
}

void ResizeRenderTargets(RenderTargets& rt, int width, int height)
{
    // shadow map intentionally excluded its resolution is independent of window size
    ResizePostProcessTarget(rt.post, width, height);
    ResizeHDRTarget(rt.hdr, width, height);
    ResizePingPongTarget(rt.pingpong, width, height);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}


void DestroyRenderTargets(RenderTargets& rt)
{
    glDeleteFramebuffers(1, &rt.shadow.FBO);
    glDeleteTextures(1, &rt.shadow.depthTexture);

    glDeleteFramebuffers(1, &rt.post.FBO);
    glDeleteTextures(1, &rt.post.colorTexture);
    glDeleteRenderbuffers(1, &rt.post.RBO);

    glDeleteFramebuffers(1, &rt.hdr.FBO);
    glDeleteTextures(2, rt.hdr.colorBuffers);
    glDeleteRenderbuffers(1, &rt.hdr.depthRBO);

    glDeleteFramebuffers(2, rt.pingpong.FBO);
    glDeleteTextures(2, rt.pingpong.colorBuffer);

    rt = RenderTargets{};
}
