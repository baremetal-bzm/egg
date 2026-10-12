#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    mat4 rotation_only_view = mat4(mat3(view));
    vec4 pos = projection * rotation_only_view * vec4(aPos, 1.0); //clipspace
    gl_Position = pos.xyww; //set z value after perspective division to maximum of 1 for skybox 
}  