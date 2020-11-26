#version 330 core 
in vec3 fragNor;
in vec3 fragPos;
in vec3 LV;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float shine;

out vec4 color;

void main()
{
	vec3 normal = normalize(fragNor);
    
    vec3 position = normalize(fragPos);

    vec3 halfway = normalize(LV - position);

    // diffuse color
    float dc = max(dot(LV, normal), 0);

    // specular color
    float sc = pow(max(dot(normal, halfway), 0), shine);

    // light color
    vec3 il = vec3(1, 1, 1);

    // calculate the total color
    vec3 rc = MatDif * dc + MatSpec * sc + MatAmb;

	color = vec4(rc, 1.0);
}
