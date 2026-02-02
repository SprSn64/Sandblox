#version 330 core
out vec4 FragColor;

in vec3 norm;
in vec2 uv;
in vec3 colour;

void main(){
    FragColor = vec4(colour, 1.0f);
} 