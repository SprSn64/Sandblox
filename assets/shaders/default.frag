#version 330 core
out vec4 FragColor;

in vec3 norm;
in vec2 uv;
in vec4 colour;

void main(){
    FragColor = vec4(norm, 1.0);
} 