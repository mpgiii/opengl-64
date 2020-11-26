#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform vec3 lightP;

out vec3 fragPos;
out vec3 fragNor;
out vec3 LV;
out vec2 vTexCoord;

void main()
{
	gl_Position = P * V * M * vertPos;
	fragNor = (M * vec4(vertNor, 0.0)).xyz;
    fragPos = (M * vertPos).xyz;

    // light vector
    LV = normalize(lightP - fragPos);
    
    // texture
    vTexCoord = vertTex;
}
