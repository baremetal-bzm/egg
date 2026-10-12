#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec3 aTan;
layout (location = 3) in vec3 aBitan;
layout (location = 4) in vec2 aTexCoords;
layout (location = 5) in int aIndex; //debug index
layout (location = 6) in int aSplitHalf; //check for which half opens
layout (location = 7) in float aDeformed; //check for which half opens

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
out vec3 T; 
out vec3 B; 
out vec3 N; 
out float Deformed;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

//uniform mat3 normalMatrix;

void main()
{
    TexCoords = aTexCoords;
    Deformed = aDeformed;
    WorldPos = vec3(model * vec4(aPos, 1.0));
    mat3 normalMatrix = transpose(inverse(mat3(model))); //can be moved outside shader,
    Normal = normalMatrix * aNorm;   
    //TBN matrix to transform from tangent space to world space
    T = normalize(vec3(model * vec4(aTan, 0.0)));
    N = normalize(vec3(model * vec4(aNorm, 0.0)));
    T = normalize(T - dot(T, N) * N); //gram schmidt for renormalizing when tangents are smudged
    B = -cross(N, T);
//    T = normalize(T - dot(T, N) * N); //gram schmidt for renormalizing when tangents are smudged
//    B = cross(N, T);
//    mat3 TBN = mat3(T, B, N);

    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}