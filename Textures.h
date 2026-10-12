#pragma once
#include <string>
#include <vector>

// loads a 2D texture (1, 2, 3, or 4 channel image) with mipmaps
// returns the GL texture id, or 0 on failure 
// flipVertically: true for stb's flip-on-load 
unsigned int loadTexture(const char* path, bool flipVertically = true);

// loads a cubemap from exactly 6 files in +X, -X, +Y, -Y, +Z, -Z order
// faces are always decoded as 4-channel RGBA and stored as RGB, but can swap this to rgb/rgb for jpg if u want
// returns the textureid or 0 if somethings wrong
unsigned int loadCubemap(const std::vector<std::string>& faces, bool flipVertically = true);

