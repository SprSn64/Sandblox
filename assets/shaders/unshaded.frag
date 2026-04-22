#version 330 core
out vec4 FragColor;

uniform sampler2D tex0;

in vec2 uv;
in vec4 colour;

void main(){
	vec4 baseColour = colour;// * texture(tex0, uv);
	if(baseColour.a <= 0.1)
		discard;

	FragColor = baseColour;
} 
