#version 330 core
out vec4 FragColor;

uniform vec3 lightNorm;
uniform vec4 lightColour;
uniform vec4 ambColour;
uniform vec3 cameraNorm;

uniform sampler2D tex0;

in vec3 norm;
in vec2 uv;
in vec4 colour;

void main(){
	vec3 reflectSource = normalize(reflect(-lightNorm, norm));
	float specular = pow(max(dot(cameraNorm, reflectSource), 0), 16);

	vec4 baseColour = colour * texture(tex0, uv);

	FragColor = baseColour * max(dot(norm, lightNorm), 0) + baseColour * ambColour + specular * lightColour;
} 