#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aVertexNormal;
layout (location = 3) in vec2 aTexCoord;

out vec3 vertexNormal;
out vec3 outputColor;
out vec2 texCoord;
out vec3 fragPos;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 translation;
uniform vec4 rotation;
uniform vec3 scale;

void main(){
  vec3 model = aPos;
  //model = model.xyz + 2.0*cross(cross(model.xyz, rotation.xyz) + rotation.w*model.xyz, rotation.xyz);
  model = model.xyz + 2.0*cross(cross(model.xyz, rotation.xyz) + rotation.w*model.xyz, rotation.xyz);
  model = translation + model;
  // rotate by quaternion
  //model = scale * model;
	gl_Position = projection * view * vec4(model, 1);
	outputColor = aColor;
  fragPos = model.xyz;

  vertexNormal = aVertexNormal.xyz + 2.0*cross(cross(aVertexNormal.xyz, rotation.xyz) + rotation.w*aVertexNormal.xyz, rotation.xyz);
  vertexNormal = normalize(vertexNormal);
	texCoord = vec2(aTexCoord.x, aTexCoord.y);
}
