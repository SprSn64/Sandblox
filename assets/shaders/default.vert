#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 camera;

in float timer;
void main(){
	gl_Position = proj * model * view * camera * vec4(aPos, 1.0);
	color = aColor;
}