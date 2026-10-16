#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec3 aTan;
layout (location = 3) in vec3 aBitan;
layout (location = 4) in vec2 aTexCoords;
layout (location = 5) in int aIndex; //debug index
layout (location = 6) in int aSplitHalf; //check for which half opens
layout (location = 7) in float aDeformed; //check for which half opens

out ioblock {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    float Deformed; //convert to float for interpolation
    mat3 TBNinv;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vs_out.Deformed = aDeformed;


    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));  //in world space
    vs_out.TexCoords = aTexCoords;
    
    //normal matrix to move TBN vecs from model to world space
    mat3 normalMatrix = transpose(inverse(mat3(model))); //can be moved outside shader,
    vec3 T = normalize(normalMatrix * aTan);
    vec3 N = normalize(normalMatrix * aNorm);
    vec3 B = normalize(normalMatrix * aBitan);
//    T = normalize(T - dot(T, N) * N); //gram schmidt for renormalizing when tangents are smudged
//    vec3 B = cross(N, T);
    
    mat3 TBNinv = transpose(mat3(T, B, N));    //inverse TBN to transform from world space to tangent space
    vs_out.TangentLightPos = TBNinv * lightPos;
    vs_out.TangentViewPos  = TBNinv * viewPos;
    vs_out.TangentFragPos  = TBNinv * vs_out.FragPos;
    vs_out.TBNinv=TBNinv;
        
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}  