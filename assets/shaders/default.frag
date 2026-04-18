#version 330 core
out vec4 FragColor;

uniform vec3 lightNorm;
uniform vec4 lightColour;
uniform vec4 ambColour;
uniform vec3 cameraNorm;

uniform sampler2D tex0;

in vec4 pos;
in vec3 norm;
in vec2 uv;
in vec4 colour;

void main(){
	vec4 baseColour = colour * texture(tex0, uv);
	if(baseColour.a <= 0.1)
		discard;

	float fogStrength = min(max(0, (pos.z - 64)/32), 1);

	vec3 reflectSource = normalize(reflect(-lightNorm, norm));
	float specular = pow(max(dot(cameraNorm, reflectSource), 0), 16);

	FragColor = mix(baseColour * max(dot(norm, lightNorm), 0) + baseColour * ambColour + specular * lightColour, vec4(1, 1, 1, 1), fogStrength);
} 