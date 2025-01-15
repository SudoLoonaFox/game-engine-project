#version 330 core

in vec3 outputColor;
in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D texture1;

void main(){
	FragColor = texture(texture1, texCoord);
	FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
