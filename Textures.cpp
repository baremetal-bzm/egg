#include "Textures.h"

#include <glad/glad.h>
#include <stb_image.h> 

#include <iostream>

unsigned int loadTexture(const char* path, bool flipVertically)
{
    stbi_set_flip_vertically_on_load(flipVertically);

    // Peek at the file first so we know its channel count before decoding.
    int width, height, nrComponents;
    if (!stbi_info(path, &width, &height, &nrComponents))
    {
        std::cout << "Texture failed to load at path: " << path
            << " (" << stbi_failure_reason() << ")" << std::endl;
        return 0;
    }

    //recast 2-channel as 4, apparently 2 channel images are deprecated in modern gl. alpha component of grey/alpha image will be casted to .r and .a, instead of rgba (r,a,0,0), apparently?
    //passing 0 to stbi_load for the req_comp param will use the images original channel count
    const int requested = (nrComponents == 2) ? 4 : 0;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, requested);
    if (!data)
    {
        std::cout << "Texture failed to load at path: " << path
            << " (" << stbi_failure_reason() << ")" << std::endl;
        return 0;
    }
    const int channels = requested ? requested : nrComponents; // always 1, 3 or 4 here

    const GLenum format = (channels == 1) ? GL_RED
        : (channels == 3) ? GL_RGB
        : GL_RGBA;

    // Rows of 1- and 3-channel images are only 4-byte aligned when width*channels % 4 == 0
    // Set the alignment explicitly instead of relying on some earlier code having done it
    GLint prevAlignment = 4;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &prevAlignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // RGBA images (usually with transparency) clamp to avoid edge bleeding; the rest tile.
    const GLint wrap = (format == GL_RGBA) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, prevAlignment);
    stbi_image_free(data);
    return textureID;
}

unsigned int loadCubemap(const std::vector<std::string>& faces, bool flipVertically)
{
    // POSITIVE_X + i is only a valid face target for i in [0, 5].
    if (faces.size() != 6)
    {
        std::cout << "Cubemap needs exactly 6 faces, got " << faces.size() << std::endl;
        return 0;
    }

    stbi_set_flip_vertically_on_load(flipVertically);

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    bool ok = true;
    for (unsigned int i = 0; i < 6; i++)
    {
        int width, height, nrComponents;
        // Force 4 channels so the RGBA upload below always matches the buffer, can swap to 0(i.e. 3)/rgb for jpg tho
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 4);
        if (!data)
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i]
                << " (" << stbi_failure_reason() << ")" << std::endl;
            ok = false;
            continue; // keep going so every bad path gets reported
        }
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); //swap to rgb for jpg
        stbi_image_free(data);
    }

    if (!ok)
    {
        glDeleteTextures(1, &textureID); // an incomplete cubemap is useless
        return 0;
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
