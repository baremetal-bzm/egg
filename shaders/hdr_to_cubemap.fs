#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183); //Precalculated constants representing 1/2pi and 1/pi. These are used to scale spherical coordinates (-pi to pi, -pi/2 to pi/2) into standard texture coordinates ranging from 0.0 to 1.0
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y)); //calculates the longitude angle (azimuth) around the Y-axis | calculates the latitude angle (elevation) from the XZ plane
    uv *= invAtan; //map to [-0.5, 0.5]
    uv += 0.5; //map to [0, 1]
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(WorldPos));
    vec3 color = texture(equirectangularMap, uv).rgb;
    
    FragColor = vec4(color, 1.0);
}