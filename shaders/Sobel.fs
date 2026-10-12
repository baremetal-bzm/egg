#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

//const float width = 800;
//const float height = 600;
uniform float scrWidth;
uniform float scrHeight;

void generate_kernel(inout vec4 k[9], sampler2D tex, vec2 coords)
{
    float w = 1 / scrWidth;
    float h = 1 / scrHeight;
        vec2 offsets[9] = vec2[](
            vec2(-w, h), // top left
            vec2(0.0, h), //top center
            vec2(w, h), //top right
            vec2(-w, 0.0), //center left
            vec2(0.0, 0.0), //center
            vec2(w, 0.0), //center right
            vec2(-w, -h), //bottom left
            vec2(0.0, -h), // bottom center
            vec2(w, -h) // bottom right
        );
    for(int i = 0; i < 9; i++)
        k[i] = texture(tex, coords + offsets[i]);
};


void main()
{ 
    vec4 k[9];
    generate_kernel(k, screenTexture, TexCoords.st);
    vec4 sobel_x = -(k[0] + 2*k[3] + k[6]) + (k[2] + 2*k[5] + k[8]);
    vec4 sobel_y = -(k[0] + 2*k[1] + k[2]) + (k[6] + 2*k[7] + k[8]);
    vec4 sobel = sqrt((sobel_x * sobel_x) + (sobel_y * sobel_y));

    FragColor = vec4(sobel.rgb, 1.0);

}
