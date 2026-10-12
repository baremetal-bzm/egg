#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0); //either remove translation portion of view matrix before passing uniform, or do it before this line
    gl_Position = pos.xyww; //set z value after perspective division to maximum of 1 for skybox 
}  