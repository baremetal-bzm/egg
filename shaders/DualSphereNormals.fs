#version 330 core
out vec4 FragColor;

in vec2 gTexCoords;
//in vec3 VColor;
uniform sampler2D texture1;

void main()
{    
//	FragColor = texture(texture1, TexCoords);
//FragColor = vec4(VColor, 1.0);
	FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
}