#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aVertexNormal;
layout (location = 3) in vec2 aTexCoord;

out vec3 vertexNormal;
out vec3 outputColor;
out vec2 texCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main(){
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	outputColor = aColor;
	vertexNormal = aVertexNormal;
	texCoord = vec2(aTexCoord.x, aTexCoord.y);
}
