#version 330 core 

in vec3 fragNor;
in vec3 fragPos;
in vec3 LV;
in vec2 vTexCoord;

uniform sampler2D Texture0;
uniform float shine;

out vec4 color;

void main()
{
    vec4 texColor = texture(Texture0, vTexCoord);

	vec3 normal = normalize(fragNor);
    
    vec3 position = normalize(fragPos);

    vec3 halfway = normalize(LV - position);

    // diffuse color
    float dc = max(dot(LV, normal), 0);

    // specular color
    float sc = pow(max(dot(normal, halfway), 0), shine);
    
    float ac = 0.4;

    // calculate the total color
    //color = texColor * dc + texColor * sc + texColor * ac;
    color = texColor;
}
