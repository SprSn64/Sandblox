#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec4 aColour;

uniform vec3 lightNorm;
uniform vec4 lightColour;
uniform vec4 ambColour;
uniform vec4 multColour;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

out vec3 norm;
out vec2 uv;
out vec4 colour;

void main(){
	gl_Position = vec4(aPos, 1.0) * world * view * proj;
	norm = aNormal; uv = aUV; 
	
	colour = vec4(aNormal, 1.0); 
}