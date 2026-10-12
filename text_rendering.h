#pragma once
#include <iostream>
#include <map>
#include <string>

#include <learnopengl/shader.h>
#include <ft2build.h>
#include FT_FREETYPE_H

struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

int InitializeFreetype(unsigned int& VAO, unsigned int& VBO, std::map<GLchar, Character>& Characters);
void RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color, unsigned int& VAO, unsigned int& VBO, std::map<GLchar, Character>& Characters);
