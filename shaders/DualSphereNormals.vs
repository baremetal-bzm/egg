#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec3 aTan;
layout (location = 3) in vec3 aBitan;
layout (location = 4) in vec2 aTexCoords;
layout (location = 5) in int aIndex; //debug index

out vec3 normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


//out vec3 VColor;
void main()
{

    TexCoords = aTexCoords;
        gl_Position = projection * view * model * vec4(aPos, 1.0); 

            
    mat3 normalMatrix = mat3(transpose(inverse(view * model))); //can move outside shader for performance
    normal = normalize(vec3(projection*vec4(normalMatrix * aNorm, 0.0)));
}  