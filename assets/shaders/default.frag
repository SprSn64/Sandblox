#version 330 core
out vec4 FragColor;

uniform vec3 lightNorm;
uniform vec4 lightColour;
uniform vec4 ambColour;
uniform vec3 cameraNorm;

in vec3 norm;
in vec2 uv;
in vec4 colour;

void main(){
	vec3 reflectSource = normalize(reflect(-lightNorm, norm));
	float specular = pow(max(dot(cameraNorm, reflectSource), 0), 16);

	FragColor = colour * max(dot(norm, lightNorm), 0) + colour * ambColour + specular * lightColour;
} 