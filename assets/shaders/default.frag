#version 330 core
out vec4 FragColor;

in vec3 norm;
in vec2 uv;
in vec4 colour;

void main(){
    FragColor = colour * dot(norm, vec3(0.25, 0.42, 0.33)) + 1;
} 