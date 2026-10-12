#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in vec3 normal[];
in vec2 TexCoords[];
out vec2 gTexCoords;

const float MAGNITUDE = 0.2;
  
//uniform mat4 projection;

void GenerateLine(int index)
{
    gl_Position = gl_in[index].gl_Position;
    gTexCoords=TexCoords[index];
    EmitVertex();
    gl_Position =  (gl_in[index].gl_Position + 
                                vec4(normal[index], 0.0) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(0); // first vertex normal
    GenerateLine(1); // second vertex normal
    GenerateLine(2); // third vertex normal
}  