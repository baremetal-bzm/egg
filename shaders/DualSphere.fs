#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightnessColor;

in ioblock {
    vec3 FragPos; //worldspace
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    float Deformed;
    mat3 TBNinv;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;

uniform float parallaxScaleFactor;

vec2 ParallaxOcclusionMapping(vec2 texCoords, vec3 viewDir)
{ 
    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir))); //less precision when looking straight on 
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P along -view direction)
    //vec2 P = viewDir.xy / viewDir.z * parallaxScaleFactor;  //can opt to leave .z division out
    vec2 P = viewDir.xy * parallaxScaleFactor;  //see above
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(depthTexture, currentTexCoords).r;
    vec2 prevTexCoords = currentTexCoords;
    float prevDepthMapValue = currentDepthMapValue;

    while(currentLayerDepth < currentDepthMapValue) //find first value along P where the value in the depthmap is < the sample layer depth value
    {
        // shift texture coordinates along direction of P
        prevTexCoords = currentTexCoords;
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        prevDepthMapValue = currentDepthMapValue;
        currentDepthMapValue = texture(depthTexture, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
//    float beforeDepth = texture(depthTexture, prevTexCoords).r - currentLayerDepth + layerDepth;
 float beforeDepth = prevDepthMapValue - currentLayerDepth + layerDepth;

    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

void main()
{    

        
        // offset texture coordinates with Parallax Mapping
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords = fs_in.TexCoords;
    
    texCoords = ParallaxOcclusionMapping(fs_in.TexCoords,  viewDir);       
//    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0) //hack to prevent weird wrapping effects, seems to not work for sphere
//        discard;

    
    // normal map is rgb texture in range [0,1]
    vec3 normalTS = texture(normalTexture, texCoords).rgb;
    // transform to  [-1,1]
    normalTS = normalize(normalTS * 2.0 - 1.0);  
   
   //calc world space normal from screenspace derivatives to handle triangles on edge of displaced vertices
   vec3 ssNormalWS = normalize(cross(dFdx(fs_in.FragPos), dFdy(fs_in.FragPos)));
   ssNormalWS = normalize(fs_in.TBNinv * ssNormalWS); //convert into tangent space or stored vertex normal

   vec3 normal = normalize(mix(normalTS, ssNormalWS, step(0.001, fs_in.Deformed)));
   
    // get diffuse color
    vec3 color = texture(diffuseTexture, texCoords).rgb;
    // ambient
    vec3 ambient = 0.5 * color;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;

    vec3 composite = ambient + diffuse + specular;
    float brightness = dot(composite, vec3(0.2126, 0.7152, 0.0722)); //weights from wikipedia somethin to do with eye sensitivity
    if(brightness > 1.0)
        BrightnessColor = vec4(composite, 1.0);
    else
        BrightnessColor = vec4(0.0, 0.0, 0.0, 1.0);
    FragColor = vec4(composite, 1.0);
}