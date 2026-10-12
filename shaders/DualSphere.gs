#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in ioblock {
    vec3 FragPos; //world space position
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    float Deformed; //convert to float for interpolation
    mat3 TBNinv;
} gs_in[];  

out ioblock {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    float Deformed; //convert to float for interpolation
    mat3 TBNinv;
} gs_out; 

uniform float time;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 lightPos;
uniform vec3 viewPos;

float hash11(float n) { return fract(sin(n) * 43758.5453123); } //return value is (-1, 1)
vec3  hash31(float n) { return vec3(hash11(n), hash11(n + 17.13), hash11(n + 41.7)); }
mat3 rotationMatrix(vec3 axis, float angle)   //axis must be normalized,  Rodrigues' rotation formula, from wiki
{
    float s = sin(angle), c = cos(angle), oc = 1.0 - c;
    return mat3(
        oc*axis.x*axis.x + c,        oc*axis.x*axis.y + axis.z*s, oc*axis.z*axis.x - axis.y*s,
        oc*axis.x*axis.y - axis.z*s, oc*axis.y*axis.y + c,        oc*axis.y*axis.z + axis.x*s,
        oc*axis.z*axis.x + axis.y*s, oc*axis.y*axis.z - axis.x*s, oc*axis.z*axis.z + c);
}
vec3 GetNormal()
{
   vec3 a = gs_in[0].FragPos - gs_in[1].FragPos;
   vec3 b = gs_in[2].FragPos - gs_in[1].FragPos;
   return normalize(cross(a, b));
}  

void SetVaryings(int index)
{
//gs_out.FragPos = gs_in[index].FragPos;
gs_out.TexCoords = gs_in[index].TexCoords;
gs_out.TangentLightPos = gs_in[index].TangentLightPos;
gs_out.TangentViewPos = gs_in[index].TangentViewPos;
gs_out.TangentFragPos = gs_in[index].TangentFragPos;
gs_out.Deformed = gs_in[index].Deformed;
gs_out.TBNinv = gs_in[index].TBNinv;

////add this commented out code below to the loop if you want correct lighting on fragments, left out since its a quick effect you likely wont notice
//mat3 TBNinvRot = gs_in[i].TBNinv * transpose(R);
//gs_out.TBNinv           = TBNinvRot;
//gs_out.TangentLightPos  = TBNinvRot * lightPos;
//gs_out.TangentViewPos   = TBNinvRot * viewPos;
//gs_out.TangentFragPos   = TBNinvRot * p;
}

void main() {    

 vec3 normal = GetNormal();

    float magnitude = 2.0;
    float t = clamp(time, 0.0, 4.5); //shards hit origin at t=2+sqrt6 ~ 4.45
    float s = 0.5 * t * t - magnitude * t;      // gravity(inward) - initial velocity (outward): normals are flipped -1 due to the reconstruction above

    //per-triangle randomness
    vec3 r = hash31(float(gl_PrimitiveIDIn));
    vec3 axis = normalize(r * 2.0 - 1.0 + vec3(1e-4)); //r is [0,1) so remap to [-1, 1), small offset in case r is vec3(0) (is this even possible?)
    float spin = mix(2, 5.0, hash11(r.x * 91.7));//arbitrary spin rate constants in rads, hashed input * arbitrary constant
    mat3 R = rotationMatrix(axis, spin * s);    

    vec3 center = (gs_in[0].FragPos + gs_in[1].FragPos + gs_in[2].FragPos) / 3.0;

    for (int i = 0; i < 3; i++) {
        vec3 p = R * (gs_in[i].FragPos - center) + center + normal * s; // (rotation around center) + (center, default position) + (inward/outward motion)
        SetVaryings(i);
        gs_out.FragPos = p;
        gl_Position = projection * view * vec4(p, 1.0);
        EmitVertex();
    }
    EndPrimitive();
}  