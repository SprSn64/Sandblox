#version 330 core
out vec4 FragColor;

uniform vec3 lightNorm;
uniform vec4 lightColour;
uniform vec4 ambColour;
uniform vec3 cameraNorm;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

uniform sampler2D tex0;

uniform vec4 fogColour = vec4(1, 1, 1, 1);
uniform vec2 fogDist;

uniform vec2 resolution;

in vec4 pos;
in vec3 norm;
in vec2 uv;
in vec4 colour;

void main(){
	vec4 baseColour = colour * texture(tex0, uv);
	if(baseColour.a == 0) discard;
	float alphaDither = baseColour.a == 1 ? 0 : (sin(pos.x/pos.w * resolution.x + pos.w) * sin(pos.y/pos.w * resolution.y + pos.w) + 1) / 2;
	if(baseColour.a <= alphaDither)
		discard;

	vec3 reflectSource = normalize(reflect(-normalize(lightNorm), norm));
	float specular = pow(max(dot(cameraNorm, reflectSource), 0), 16);

	FragColor = (baseColour + baseColour * ambColour + specular - specular) * lightColour;
} 