#version 330 core

out vec4 FragColor;

in vec2 texCoords;
uniform sampler2D img;
uniform sampler2D msk;

uniform float mode;

void main()
{
	FragColor = texture(img, texCoords);

	vec4 m = texture(msk, texCoords);
	if(m.x > 0.5) FragColor = vec4(0, 1, 0, 1);
	else if(m.x > 0) FragColor = vec4(1, 0, 0, 1);

	if(mode == 255) FragColor = vec4(0, 1, 0, 1);
	else if(mode == 1) FragColor = vec4(1, 0, 0, 1);
}