#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec4 pColor;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;

out vec3 partCol;
out float fade;


void main()
{
	// Billboarding: set the upper 3x3 to be the identity matrix
	mat4 M0 = M;

	M0[0] = vec4(1.0, 0.0, 0.0, 0.0);
	M0[1] = vec4(0.0, 1.0, 0.0, 0.0);
	M0[2] = vec4(0.0, 0.0, 1.0, 0.0);

	gl_Position = P * V * M0 * vec4(vertPos.xyz, 1.0);

	partCol = pColor.xyz;
    fade = pColor.w;
}
