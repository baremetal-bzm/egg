//#version 330 core
//out vec4 FragColor;
//
//in vec2 TexCoords;
//
//float near = 0.1;
//float far = 100.0;
//
//uniform sampler2D texture1;
//
//float LinearizeDepth(float depth)
//{
//    float z = depth * 2.0 - 1.0; //Back to NDC
//    return (2.0 * near * far) / (far + near - z * (far - near));
//}
//
//void main()
//{    
//    float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstraton
//    FragColor = vec4(vec3(depth), 1.0);
//}
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{    
	FragColor = texture(texture1, TexCoords);
//    vec4 texColor = texture (texture1, TexCoords);
//    if(texColor.a < 0.1)
//        discard;
//    FragColor = texColor;
}
