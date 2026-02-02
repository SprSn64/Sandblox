#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aColour;

uniform mat4 view;
uniform mat4 proj;

out vec3 norm;
out vec2 uv;
out vec3 colour;

void main(){
	gl_Position = vec4(aPos, 1.0) * view * proj;
	norm = aNormal; uv = aUV; colour = aColour; 
}